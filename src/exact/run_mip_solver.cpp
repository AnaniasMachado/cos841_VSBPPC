#include "mip_solver.hpp"

#include <iostream>
#include <vector>

int main() {
    std::string rootFolder = "../instances";
    std::string outputFolder = "../mip_solutions";

    double timeLimitSeconds = 3600.0;

    std::vector<MIPConfig> configs = {
        {
            InstanceSet::SET1,
            CostType::LINEAR,
            BinSizeSetting::THREE_TYPES,
            "set1_3types_linear"
        },
        {
            InstanceSet::SET1,
            CostType::LINEAR,
            BinSizeSetting::FIVE_TYPES,
            "set1_5types_linear"
        },
        {
            InstanceSet::SET2,
            CostType::LINEAR,
            BinSizeSetting::SEVEN_TYPES,
            "set2_7types_linear"
        },
        {
            InstanceSet::SET2,
            CostType::CONVEX,
            BinSizeSetting::SEVEN_TYPES,
            "set2_7types_convex"
        },
        {
            InstanceSet::SET2,
            CostType::CONCAVE,
            BinSizeSetting::SEVEN_TYPES,
            "set2_7types_concave"
        }
    };

    try {
        VSBPPCMIPSolver solver(timeLimitSeconds, outputFolder);
        solver.solveFolderRecursive(rootFolder, configs);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}