#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../metaheuristic/lnsa.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

int main() {
    // =========================
    // Parameters to edit
    // =========================
    // std::string instanceFile = "../instances/set_1/N_1000_idx_2/Correia_Random_4_1_8_1.txt";
    std::string instanceFile = "../instances/set_1/N_100/Correia_Random_1_1_1_1.txt";

    const InstanceSet setType = InstanceSet::SET1;
    const CostType costType = CostType::LINEAR;
    const BinSizeSetting binSizeSetting = BinSizeSetting::THREE_TYPES;

    const int iterationLimit = 120;
    const int destroyPercent = 25;
    std::mt19937 rng(42);

    // Penalties used by your Solution objective for infeasible states.
    const int kw = 1000;
    const int kc = 1000;

    try {
        VSBPPCInstance inst = readInstance(
            instanceFile,
            setType,
            costType,
            binSizeSetting
        );

        std::cout << "Loaded instance: " << instanceFile << "\n";
        inst.printStatistics();

        LNSAParams params;
        params.iterationLimit = iterationLimit;
        params.destroyPercent = destroyPercent;
        params.mode = LNSAMode::SFC;

        // =========================
        // Start timing
        // =========================
        auto start = std::chrono::high_resolution_clock::now();

        Solution best = LNSAAlgorithm::solve(inst, kw, kc, params, rng);

        // =========================
        // End timing
        // =========================
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "\n===== LNSA result =====\n";
        std::cout << "Objective: " << best.computeObjective() << "\n";
        std::cout << "Feasible: " << (best.isFeasible() ? "yes" : "no") << "\n";
        std::cout << "Total excess: " << best.totalExcess << "\n";
        std::cout << "Total conflicts: " << best.totalConflicts << "\n";

        std::cout << "Runtime (s): " << elapsed.count() << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}