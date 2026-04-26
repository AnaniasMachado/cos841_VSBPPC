#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../metaheuristic/lnsa.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <random>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <ctime>
#include <sstream>

namespace fs = std::filesystem;

static std::string modeName(LNSAMode mode) {
    switch (mode) {
        case LNSAMode::VANILLA: return "vanilla";
        case LNSAMode::SFC:     return "sfc";
    }

    return "unknown";
}

static std::string currentDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static bool isTxtFile(const fs::path& path) {
    return fs::is_regular_file(path) && path.extension() == ".txt";
}

static std::vector<std::string> collectInstanceFiles(const std::string& rootDir) {
    std::vector<std::string> files;

    for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (isTxtFile(entry.path())) {
            files.push_back(entry.path().string());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

static bool isSet1Instance(const std::string& file) {
    return file.find("set_1") != std::string::npos ||
           file.find("Correia_Random") != std::string::npos;
}

static bool isSet2Instance(const std::string& file) {
    return file.find("set_2") != std::string::npos ||
           file.find("HS_Random") != std::string::npos;
}

static std::string solutionFilename(const std::string& solutionsDir,
                                    const std::string& instanceFile,
                                    const std::string& configName,
                                    int run) {
    fs::create_directories(solutionsDir);

    return solutionsDir + "/sol_" +
           fs::path(instanceFile).stem().string() +
           "_" + configName +
           "_run" + std::to_string(run) +
           ".txt";
}

static void writeSolution(std::ofstream& out, const Solution& sol) {
    out << "solution\n";

    int outBin = 0;

    for (int b = 0; b < sol.B; ++b) {
        if (sol.itemsInBin[b].empty()) continue;

        int type = sol.binType[b];
        int capacity = sol.inst.binTypes[type].capacity;
        int cost = sol.inst.binTypes[type].cost;

        out << "bin " << outBin
            << " type " << type
            << " capacity " << capacity
            << " cost " << cost
            << " items";

        for (int item : sol.itemsInBin[b]) {
            out << " " << item;
        }

        out << " load " << sol.binLoad[b] << "\n";

        ++outBin;
    }
}

static void writeRunResult(std::ofstream& out,
                           const std::string& instanceFile,
                           const std::string& configName,
                           const LNSAParams& params,
                           int run,
                           unsigned int seed,
                           const std::string& startTime,
                           const Solution& sol,
                           double runtime) {
    out << "instance_file " << instanceFile << "\n";
    out << "config " << configName << "\n";
    out << "mode " << modeName(params.mode) << "\n";
    out << "run " << run << "\n";
    out << "seed " << seed << "\n";
    out << "start_time " << startTime << "\n";
    out << "iteration_limit " << params.iterationLimit << "\n";
    out << "destroy_percent " << params.destroyPercent << "\n";
    out << "status " << (sol.isFeasible() ? "FEASIBLE" : "INFEASIBLE") << "\n";

    out << std::fixed << std::setprecision(6);
    out << "runtime " << runtime << "\n";
    out << "objective " << static_cast<double>(sol.computeObjective()) << "\n";
    out << "total_bin_cost " << sol.totalBinCost << "\n";
    out << "total_excess " << sol.totalExcess << "\n";
    out << "total_conflicts " << sol.totalConflicts << "\n";
    out << "\n";

    writeSolution(out, sol);
}

static void runOneConfiguration(const std::string& instanceFile,
                                const std::string& configName,
                                InstanceSet setType,
                                CostType costType,
                                BinSizeSetting binSizeSetting,
                                const LNSAParams& params,
                                int kw,
                                int kc,
                                int runsPerInstance,
                                unsigned int firstSeed,
                                const std::string& solutionsDir) {
    VSBPPCInstance inst = readInstance(
        instanceFile,
        setType,
        costType,
        binSizeSetting
    );

    for (int run = 1; run <= runsPerInstance; ++run) {
        std::string solFile = solutionFilename(
            solutionsDir,
            instanceFile,
            configName,
            run
        );

        if (fs::exists(solFile)) {
            std::cout << "Skipping completed run: " << solFile << "\n";
            continue;
        }

        unsigned int seed = firstSeed + static_cast<unsigned int>(run - 1);
        std::mt19937 rng(seed);

        std::string startTime = currentDateTime();

        std::cout << "[" << startTime << "] "
                  << "Solving " << instanceFile
                  << " | config " << configName
                  << " | run " << run
                  << " | seed " << seed << "\n";

        auto start = std::chrono::high_resolution_clock::now();

        Solution best = LNSAAlgorithm::solve(inst, kw, kc, params, rng);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::ofstream out(solFile);

        if (!out) {
            throw std::runtime_error("Could not open solution file: " + solFile);
        }

        writeRunResult(
            out,
            instanceFile,
            configName,
            params,
            run,
            seed,
            startTime,
            best,
            elapsed.count()
        );

        out.close();

        std::cout << "Saved " << solFile
                  << " | objective " << best.computeObjective()
                  << " | feasible " << (best.isFeasible() ? "yes" : "no")
                  << " | runtime " << elapsed.count() << " s\n";
    }
}

int main() {
    // =========================
    // Parameters to edit
    // =========================
    const std::string instancesRoot = "../instances";
    const std::string solutionsDir = "../lnsa_solutions/vanilla";

    const int iterationLimit = 200;
    const int destroyPercent = 20;
    const int runsPerInstance = 10;
    const unsigned int firstSeed = 42;

    const int kw = 1000;
    const int kc = 1000;

    LNSAParams params;
    params.iterationLimit = iterationLimit;
    params.destroyPercent = destroyPercent;
    params.mode = LNSAMode::VANILLA;

    try {
        std::vector<std::string> instanceFiles =
            collectInstanceFiles(instancesRoot);

        std::cout << "Found " << instanceFiles.size()
                  << " instance files under " << instancesRoot << "\n";

        std::cout << "Saving solutions to: " << solutionsDir << "\n";

        for (const std::string& instanceFile : instanceFiles) {
            if (isSet1Instance(instanceFile)) {
                runOneConfiguration(
                    instanceFile,
                    "set1_3types_linear",
                    InstanceSet::SET1,
                    CostType::LINEAR,
                    BinSizeSetting::THREE_TYPES,
                    params,
                    kw,
                    kc,
                    runsPerInstance,
                    firstSeed,
                    solutionsDir
                );

                runOneConfiguration(
                    instanceFile,
                    "set1_5types_linear",
                    InstanceSet::SET1,
                    CostType::LINEAR,
                    BinSizeSetting::FIVE_TYPES,
                    params,
                    kw,
                    kc,
                    runsPerInstance,
                    firstSeed,
                    solutionsDir
                );

            } else if (isSet2Instance(instanceFile)) {
                runOneConfiguration(
                    instanceFile,
                    "set2_7types_convex",
                    InstanceSet::SET2,
                    CostType::CONVEX,
                    BinSizeSetting::SEVEN_TYPES,
                    params,
                    kw,
                    kc,
                    runsPerInstance,
                    firstSeed,
                    solutionsDir
                );

                runOneConfiguration(
                    instanceFile,
                    "set2_7types_linear",
                    InstanceSet::SET2,
                    CostType::LINEAR,
                    BinSizeSetting::SEVEN_TYPES,
                    params,
                    kw,
                    kc,
                    runsPerInstance,
                    firstSeed,
                    solutionsDir
                );

                runOneConfiguration(
                    instanceFile,
                    "set2_7types_concave",
                    InstanceSet::SET2,
                    CostType::CONCAVE,
                    BinSizeSetting::SEVEN_TYPES,
                    params,
                    kw,
                    kc,
                    runsPerInstance,
                    firstSeed,
                    solutionsDir
                );

            } else {
                std::cerr << "Warning: could not infer set for file: "
                          << instanceFile << "\n";
            }
        }

        std::cout << "\nAll results saved in: " << solutionsDir << "\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}