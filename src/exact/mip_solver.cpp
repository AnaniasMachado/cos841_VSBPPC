#include "mip_solver.hpp"

namespace fs = std::filesystem;

VSBPPCMIPSolver::VSBPPCMIPSolver(double timeLimitSeconds,
                                 const std::string& outputDirectory)
    : timeLimit(timeLimitSeconds),
      outputDir(outputDirectory) {
    fs::create_directories(outputDir);
}

// =====================================================
// Helpers
// =====================================================

static bool isSet1InstancePath(const std::string& path) {
    return path.find("set_1") != std::string::npos ||
           path.find("Correia_Random") != std::string::npos;
}

static bool isSet2InstancePath(const std::string& path) {
    return path.find("set_2") != std::string::npos ||
           path.find("HS_Random") != std::string::npos;
}

static bool compatibleInstanceConfig(const std::string& path,
                                     const MIPConfig& config) {
    if (isSet1InstancePath(path) && config.setType != InstanceSet::SET1) {
        return false;
    }

    if (isSet2InstancePath(path) && config.setType != InstanceSet::SET2) {
        return false;
    }

    if (config.setType == InstanceSet::SET1 &&
        config.binSizeSetting == BinSizeSetting::SEVEN_TYPES) {
        return false;
    }

    if (config.setType == InstanceSet::SET2 &&
        config.binSizeSetting != BinSizeSetting::SEVEN_TYPES) {
        return false;
    }

    return true;
}

static bool resultFileLooksComplete(const std::string& path) {
    if (!fs::exists(path)) return false;
    if (fs::file_size(path) == 0) return false;

    std::ifstream in(path);
    if (!in) return false;

    std::string line;
    bool hasStatus = false;
    bool hasObjective = false;
    bool hasGap = false;
    bool hasOptimal = false;

    while (std::getline(in, line)) {
        if (line.rfind("status ", 0) == 0) hasStatus = true;
        if (line.rfind("objective ", 0) == 0) hasObjective = true;
        if (line.rfind("gap ", 0) == 0) hasGap = true;
        if (line.rfind("optimal ", 0) == 0) hasOptimal = true;
    }

    return hasStatus && hasObjective && hasGap && hasOptimal;
}

// =====================================================
// Static methods
// =====================================================

std::string VSBPPCMIPSolver::sanitizeFilename(const std::string& name) {
    std::string s = name;

    for (char& c : s) {
        if (c == '/' || c == '\\' || c == ':' || c == ' ' || c == '.') {
            c = '_';
        }
    }

    return s;
}

bool VSBPPCMIPSolver::hasConflict(const VSBPPCInstance& inst, int i, int j) {
    int word = j >> 6;
    int bit = j & 63;

    return (inst.conflicts[i][word] & (1ULL << bit)) != 0;
}

std::string VSBPPCMIPSolver::statusToString(int status) {
    switch (status) {
        case GRB_OPTIMAL: return "OPTIMAL";
        case GRB_TIME_LIMIT: return "TIME_LIMIT";
        case GRB_INFEASIBLE: return "INFEASIBLE";
        case GRB_INF_OR_UNBD: return "INF_OR_UNBD";
        case GRB_UNBOUNDED: return "UNBOUNDED";
        case GRB_INTERRUPTED: return "INTERRUPTED";
        default: return "OTHER";
    }
}

std::string VSBPPCMIPSolver::makeOutputPath(
    const std::string& instancePath,
    const MIPConfig& config) const {
    fs::path p(instancePath);

    std::string instanceName = p.stem().string();

    std::string file =
        "solution_" +
        sanitizeFilename(instanceName) +
        "_" +
        sanitizeFilename(config.name) +
        ".txt";

    return (fs::path(outputDir) / file).string();
}

// =====================================================
// Solve one instance/config
// =====================================================

MIPSolveResult VSBPPCMIPSolver::solveOne(const std::string& instancePath,
                                         const MIPConfig& config) {
    MIPSolveResult result;

    std::string outputPath = makeOutputPath(instancePath, config);

    if (resultFileLooksComplete(outputPath)) {
        std::cout << "Skipping existing result: " << outputPath << "\n";
        result.skipped = true;
        result.status = "SKIPPED";
        return result;
    }

    if (fs::exists(outputPath)) {
        std::cout << "Removing incomplete result file: " << outputPath << "\n";
        fs::remove(outputPath);
    }

    std::cout << "Solving: " << instancePath
              << " | config: " << config.name << "\n";

    auto wallStart = std::chrono::high_resolution_clock::now();

    std::ofstream out(outputPath);
    if (!out) {
        throw std::runtime_error("Could not open output file: " + outputPath);
    }

    try {
        VSBPPCInstance inst = readInstance(
            instancePath,
            config.setType,
            config.costType,
            config.binSizeSetting
        );

        const int N = inst.N;
        const int K = static_cast<int>(inst.binTypes.size());
        const int P = N;

        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 0);
        env.start();

        GRBModel model(env);
        model.set(GRB_DoubleParam_TimeLimit, timeLimit);
        model.set(GRB_DoubleParam_MIPGap, 1e-2);

        std::vector<std::vector<GRBVar>> x(
            N,
            std::vector<GRBVar>(P)
        );

        std::vector<std::vector<GRBVar>> y(
            P,
            std::vector<GRBVar>(K)
        );

        for (int t = 0; t < N; ++t) {
            for (int p = 0; p < P; ++p) {
                x[t][p] = model.addVar(
                    0.0,
                    1.0,
                    0.0,
                    GRB_BINARY,
                    "x_" + std::to_string(t) + "_" + std::to_string(p)
                );
            }
        }

        for (int p = 0; p < P; ++p) {
            for (int k = 0; k < K; ++k) {
                y[p][k] = model.addVar(
                    0.0,
                    1.0,
                    inst.binTypes[k].cost,
                    GRB_BINARY,
                    "y_" + std::to_string(p) + "_" + std::to_string(k)
                );
            }
        }

        model.update();

        // Each bin has at most one type.
        for (int p = 0; p < P; ++p) {
            GRBLinExpr expr = 0;

            for (int k = 0; k < K; ++k) {
                expr += y[p][k];
            }

            model.addConstr(expr <= 1, "one_type_" + std::to_string(p));
        }

        // Each item is assigned exactly once.
        for (int t = 0; t < N; ++t) {
            GRBLinExpr expr = 0;

            for (int p = 0; p < P; ++p) {
                expr += x[t][p];
            }

            model.addConstr(expr == 1, "assign_" + std::to_string(t));
        }

        // Capacity.
        for (int p = 0; p < P; ++p) {
            GRBLinExpr load = 0;
            GRBLinExpr cap = 0;

            for (int t = 0; t < N; ++t) {
                load += inst.weights[t] * x[t][p];
            }

            for (int k = 0; k < K; ++k) {
                cap += inst.binTypes[k].capacity * y[p][k];
            }

            model.addConstr(load <= cap, "capacity_" + std::to_string(p));
        }

        // Conflicts.
        for (int p = 0; p < P; ++p) {
            GRBLinExpr binUsed = 0;

            for (int k = 0; k < K; ++k) {
                binUsed += y[p][k];
            }

            for (int i = 0; i < N; ++i) {
                for (int j = i + 1; j < N; ++j) {
                    if (!hasConflict(inst, i, j)) continue;

                    model.addConstr(
                        x[i][p] + x[j][p] <= binUsed,
                        "conf_" + std::to_string(i) + "_" +
                        std::to_string(j) + "_" +
                        std::to_string(p)
                    );
                }
            }
        }

        // Constraint (6): nonincreasing bin loads
        for (int p = 1; p < P; ++p) {
            GRBLinExpr prevLoad = 0;
            GRBLinExpr currLoad = 0;

            for (int i = 0; i < N; ++i) {
                prevLoad += inst.weights[i] * x[i][p - 1];
                currLoad += inst.weights[i] * x[i][p];
            }

            model.addConstr(
                prevLoad >= currLoad,
                "load_order_" + std::to_string(p)
            );
        }

        GRBLinExpr obj = 0;
        for (int p = 0; p < P; ++p) {
            for (int k = 0; k < K; ++k) {
                obj += inst.binTypes[k].cost * y[p][k];
            }
        }
        model.setObjective(obj, GRB_MINIMIZE);

        auto solveStart = std::chrono::high_resolution_clock::now();
        model.optimize();
        auto solveEnd = std::chrono::high_resolution_clock::now();

        double solveTime =
            std::chrono::duration<double>(solveEnd - solveStart).count();

        auto wallEnd = std::chrono::high_resolution_clock::now();
        double totalTime =
            std::chrono::duration<double>(wallEnd - wallStart).count();

        int status = model.get(GRB_IntAttr_Status);

        result.status = statusToString(status);
        result.optimal = (status == GRB_OPTIMAL);
        result.solveTime = solveTime;

        out << std::fixed << std::setprecision(6);

        out << "instance_file " << instancePath << "\n";
        out << "config " << config.name << "\n";
        out << "status " << statusToString(status) << "\n";
        out << "gurobi_status_code " << status << "\n";
        out << "time_limit " << timeLimit << "\n";
        out << "solve_time " << solveTime << "\n";
        out << "total_time " << totalTime << "\n";

        bool hasIncumbent = false;

        try {
            hasIncumbent = model.get(GRB_IntAttr_SolCount) > 0;
        } catch (...) {
            hasIncumbent = false;
        }

        if (hasIncumbent) {
            result.hasIncumbent = true;
            result.objective = model.get(GRB_DoubleAttr_ObjVal);
            result.gap = model.get(GRB_DoubleAttr_MIPGap);

            double objVal = model.get(GRB_DoubleAttr_ObjVal);
            double objBound = model.get(GRB_DoubleAttr_ObjBound);
            double mipGap = model.get(GRB_DoubleAttr_MIPGap);

            out << "objective " << objVal << "\n";
            out << "best_bound " << objBound << "\n";
            out << "gap " << mipGap << "\n";
            out << "optimal " << (status == GRB_OPTIMAL ? "yes" : "no") << "\n";

            out << "\nsolution\n";

            for (int p = 0; p < P; ++p) {
                int selectedType = -1;

                for (int k = 0; k < K; ++k) {
                    if (y[p][k].get(GRB_DoubleAttr_X) > 0.5) {
                        selectedType = k;
                        break;
                    }
                }

                if (selectedType == -1) continue;

                int load = 0;

                out << "bin " << p
                    << " type " << selectedType
                    << " capacity " << inst.binTypes[selectedType].capacity
                    << " cost " << inst.binTypes[selectedType].cost
                    << " items";

                for (int t = 0; t < N; ++t) {
                    if (x[t][p].get(GRB_DoubleAttr_X) > 0.5) {
                        out << " " << t;
                        load += inst.weights[t];
                    }
                }

                out << " load " << load << "\n";
            }
        } else {
            result.hasIncumbent = false;
            result.objective = 0.0;
            result.gap = -1.0;

            out << "objective NA\n";
            out << "best_bound NA\n";
            out << "gap NA\n";
            out << "optimal no\n";
            out << "\nsolution NA\n";
        }

        out.close();

        std::cout << "Saved: " << outputPath << "\n";
        return result;

    } catch (const GRBException& e) {
        out << "instance_file " << instancePath << "\n";
        out << "config " << config.name << "\n";
        out << "status GUROBI_EXCEPTION\n";
        out << "objective NA\n";
        out << "best_bound NA\n";
        out << "gap NA\n";
        out << "optimal no\n";
        out << "error_code " << e.getErrorCode() << "\n";
        out << "message " << e.getMessage() << "\n";
        out.close();

        std::cerr << "Gurobi error on " << instancePath
                  << ": " << e.getMessage() << "\n";

        result.status = "ERROR";
        return result;

    } catch (const std::exception& e) {
        out << "instance_file " << instancePath << "\n";
        out << "config " << config.name << "\n";
        out << "status EXCEPTION\n";
        out << "objective NA\n";
        out << "best_bound NA\n";
        out << "gap NA\n";
        out << "optimal no\n";
        out << "message " << e.what() << "\n";
        out.close();

        std::cerr << "Error on " << instancePath
                  << ": " << e.what() << "\n";

        result.status = "ERROR";
        return result;
    }
}

// =====================================================
// Solve folder recursively
// =====================================================

void VSBPPCMIPSolver::solveFolderRecursive(
    const std::string& rootFolder,
    const std::vector<MIPConfig>& configs) {

    std::vector<fs::path> files;

    for (const auto& entry : fs::recursive_directory_iterator(rootFolder)) {
        if (!entry.is_regular_file()) continue;
        files.push_back(entry.path());
    }

    std::sort(files.begin(), files.end());

    int totalJobs = 0;

    for (const fs::path& file : files) {
        std::string path = file.string();

        for (const MIPConfig& config : configs) {
            if (!compatibleInstanceConfig(path, config)) continue;
            ++totalJobs;
        }
    }

    int done = 0;

    for (const fs::path& file : files) {
        std::string path = file.string();

        for (const MIPConfig& config : configs) {
            if (!compatibleInstanceConfig(path, config)) continue;

            auto start = std::chrono::high_resolution_clock::now();

            MIPSolveResult result = solveOne(path, config);

            auto end = std::chrono::high_resolution_clock::now();
            double elapsed =
                std::chrono::duration<double>(end - start).count();

            ++done;

            std::cout << std::fixed << std::setprecision(6);
            std::cout << "[Progress] " << done << "/" << totalJobs
                      << " | Last time: " << elapsed << " s"
                      << " | Status: " << result.status
                      << " | Optimal: " << (result.optimal ? "yes" : "no");

            if (result.hasIncumbent) {
                std::cout << " | Objective: " << result.objective
                          << " | Gap: " << result.gap;
            } else {
                std::cout << " | Objective: NA"
                          << " | Gap: NA";
            }

            if (result.skipped) {
                std::cout << " | Skipped: yes";
            }

            std::cout << "\n\n";
        }
    }
}