#ifndef MIP_SOLVER_HPP
#define MIP_SOLVER_HPP

#include "../util/vsbppc.hpp"


#include <gurobi_c++.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class MIPStatus {
    OPTIMAL,
    TIME_LIMIT,
    INFEASIBLE,
    NO_SOLUTION,
    OTHER
};

struct MIPConfig {
    InstanceSet setType;
    CostType costType;
    BinSizeSetting binSizeSetting;

    std::string name;
};

struct MIPSolveResult {
    bool skipped = false;
    bool optimal = false;
    bool hasIncumbent = false;

    std::string status = "UNKNOWN";
    double objective = 0.0;
    double gap = -1.0;
    double solveTime = 0.0;
};

class VSBPPCMIPSolver {
private:
    double timeLimit;
    std::string outputDir;

    static std::string sanitizeFilename(const std::string& name);
    static std::string statusToString(int gurobiStatus);

    static bool hasConflict(const VSBPPCInstance& inst, int i, int j);

    std::string makeOutputPath(const std::string& instancePath,
                    const MIPConfig& config) const;

public:
    VSBPPCMIPSolver(double timeLimitSeconds,
                    const std::string& outputDirectory);

    MIPSolveResult solveOne(const std::string& instancePath,
                    const MIPConfig& config);

    void solveFolderRecursive(const std::string& rootFolder,
                    const std::vector<MIPConfig>& configs);
};

#endif