#include <iostream>
#include <chrono>
#include <random>
#include <string>

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/perturbation.hpp"
#include "../util/local_search.hpp"

void testNeighborhood(const std::string& name,
                      Solution baseSol,
                      ImprovementType improvementType,
                      std::mt19937& rng) {
    LocalSearch ls(baseSol, improvementType, rng);

    auto start = std::chrono::high_resolution_clock::now();

    bool improved = false;

    if (name == "relocate") {
        improved = ls.relocate();
    } else if (name == "exchange") {
        improved = ls.exchange();
    } else if (name == "exchange21") {
        improved = ls.exchange21();
    } else if (name == "classic") {
        improved = ls.classic();
    } else if (name == "ejectionGlobal") {
        improved = ls.ejectionGlobal();
    } else if (name == "assignment") {
        improved = ls.assignment((int)baseSol.N / 20);
    } else if (name == "ejectionChain") {
        improved = ls.ejectionChain();
    } else if (name == "grenade") {
        improved = ls.grenade();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "=== " << name << " ===\n";
    std::cout << "Improved: " << (improved ? "YES" : "NO") << "\n";
    std::cout << "Time: " << elapsed.count() << " s\n";
    std::cout << "Objective: " << baseSol.computeObjective() << "\n";
    std::cout << "Feasible: " << (baseSol.isFeasible() ? "YES" : "NO") << "\n\n";
}

int main() {
    std::string filename = "../instances/set_1/N_100/Correia_Random_1_1_1_1.txt";

    InstanceSet setType = InstanceSet::SET1;
    CostType costType = CostType::LINEAR;
    BinSizeSetting binSizeSetting = BinSizeSetting::THREE_TYPES;

    int kw = 1000;
    int kc = 1000;

    unsigned int seed = 42;
    std::mt19937 rng(seed);

    VSBPPCInstance inst = readInstance(filename, setType, costType, binSizeSetting);

    std::cout << "Instance loaded: N = " << inst.N << "\n\n";

    Solution initialSol = Builder::bestFitDecreasing(inst, kw, kc);

    std::cout << "=== Initial solution (BFD) ===\n";
    std::cout << "Objective: " << initialSol.computeObjective() << "\n";
    std::cout << "Feasible: " << (initialSol.isFeasible() ? "YES" : "NO") << "\n\n";

    Solution pertSol = initialSol;
    Perturbation::relocateK(pertSol, 5, rng);
    Perturbation::split(pertSol, rng);

    std::cout << "=== After perturbation ===\n";
    std::cout << "Objective: " << pertSol.computeObjective() << "\n";
    std::cout << "Feasible: " << (pertSol.isFeasible() ? "YES" : "NO") << "\n\n";

    testNeighborhood("relocate", pertSol, ImprovementType::BI, rng);
    testNeighborhood("exchange", pertSol, ImprovementType::BI, rng);
    testNeighborhood("exchange21", pertSol, ImprovementType::BI, rng);
    testNeighborhood("classic", pertSol, ImprovementType::BI, rng);
    testNeighborhood("ejectionGlobal", pertSol, ImprovementType::BI, rng);
    testNeighborhood("assignment", pertSol, ImprovementType::BI, rng);
    testNeighborhood("ejectionChain", pertSol, ImprovementType::BI, rng);
    testNeighborhood("grenade", pertSol, ImprovementType::BI, rng);

    return 0;
}