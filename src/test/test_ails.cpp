#include <iostream>
#include <chrono>
#include <string>

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../metaheuristic/ails.hpp"

int main() {
    std::string filename = "../instances/set_1/N_1000_idx_2/Correia_Random_4_1_8_1.txt";

    InstanceSet setType = InstanceSet::SET1;
    CostType costType = CostType::LINEAR;
    BinSizeSetting binSizeSetting = BinSizeSetting::THREE_TYPES;

    int kw = 400;
    int kc = 275;

    VSBPPCInstance inst =
        readInstance(filename, setType, costType, binSizeSetting);

    std::cout << "Instance loaded: N = " << inst.N << "\n\n";

    // ---- AILS PARAMETERS ----
    int maxIterations = 1000;
    int maxNoImprove = 25;

    AcceptanceType acceptance = AcceptanceType::ITERATIVE;
    ImprovementType improvement = ImprovementType::BI;

    BuilderType builder = BuilderType::IBFD;
    double builderAlpha = 0.5;

    bool useUCB = true;
    double c = 0.5718;

    bool useQRVND = true;
    double alpha = 0.1495;
    double gamma = 0.7455;
    double epsilon = 0.0377;

    bool verbose = true;
    double timeLimit = 600.0;

    std::mt19937 rng(42);

    // ---- RUN AILS ----
    auto start = std::chrono::high_resolution_clock::now();

    AILS ails(inst,
              kw, kc, rng,
              maxIterations,
              maxNoImprove,
              acceptance,
              improvement,
              useUCB,
              c,
              builder,
              builderAlpha,
              useQRVND,
              alpha,
              gamma,
              epsilon,
              verbose,
              timeLimit);

    Solution best = ails.run();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // ---- RESULTS ----
    std::cout << "\n===== FINAL RESULT =====\n";
    std::cout << "Time: " << elapsed.count() << " s\n";
    std::cout << "Objective: " << best.computeObjective() << "\n";
    std::cout << "Feasible: " << (best.isFeasible() ? "YES" : "NO") << "\n";

    return 0;
}