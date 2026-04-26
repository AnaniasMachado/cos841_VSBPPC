#include <iostream>
#include <chrono>
#include <random>
#include <string>

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/perturbation.hpp"

void printResult(const std::string& name,
                 const Solution& sol,
                 std::chrono::duration<double> elapsed) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "Time: " << elapsed.count() << " s\n";
    std::cout << "Objective: " << sol.computeObjective() << "\n";
    std::cout << "Feasible: " << (sol.isFeasible() ? "YES" : "NO") << "\n\n";
}

void testRelocateK(const Solution& baseSol, int k, std::mt19937& rng) {
    Solution sol = baseSol;

    auto start = std::chrono::high_resolution_clock::now();
    Perturbation::relocateK(sol, k, rng);
    auto end = std::chrono::high_resolution_clock::now();

    printResult("relocateK(k = " + std::to_string(k) + ")", sol, end - start);
}

void testExchangeK(const Solution& baseSol, int k, std::mt19937& rng) {
    Solution sol = baseSol;

    auto start = std::chrono::high_resolution_clock::now();
    Perturbation::exchangeK(sol, k, rng);
    auto end = std::chrono::high_resolution_clock::now();

    printResult("exchangeK(k = " + std::to_string(k) + ")", sol, end - start);
}

void testMerge(const Solution& baseSol, std::mt19937& rng) {
    Solution sol = baseSol;

    auto start = std::chrono::high_resolution_clock::now();
    Perturbation::merge(sol, rng);
    auto end = std::chrono::high_resolution_clock::now();

    printResult("merge", sol, end - start);
}

void testMergeK(const Solution& baseSol, int k, std::mt19937& rng) {
    Solution sol = baseSol;

    auto start = std::chrono::high_resolution_clock::now();
    Perturbation::mergeK(sol, k, rng);
    auto end = std::chrono::high_resolution_clock::now();

    printResult("mergeK(k = " + std::to_string(k) + ")", sol, end - start);
}

void testSplit(const Solution& baseSol, std::mt19937& rng) {
    Solution sol = baseSol;

    auto start = std::chrono::high_resolution_clock::now();
    Perturbation::split(sol, rng);
    auto end = std::chrono::high_resolution_clock::now();

    printResult("split", sol, end - start);
}

void testSplitK(const Solution& baseSol, int k, std::mt19937& rng) {
    Solution sol = baseSol;

    auto start = std::chrono::high_resolution_clock::now();
    Perturbation::splitK(sol, k, rng);
    auto end = std::chrono::high_resolution_clock::now();

    printResult("splitK(k = " + std::to_string(k) + ")", sol, end - start);
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

    Solution baseSol = Builder::bestFitDecreasing(inst, kw, kc);

    std::cout << "=== Initial solution (BFD) ===\n";
    std::cout << "Objective: " << baseSol.computeObjective() << "\n";
    std::cout << "Feasible: " << (baseSol.isFeasible() ? "YES" : "NO") << "\n\n";

    int k = 5;

    testRelocateK(baseSol, k, rng);
    testExchangeK(baseSol, k, rng);
    testMerge(baseSol, rng);
    testMergeK(baseSol, k, rng);
    testSplit(baseSol, rng);
    testSplitK(baseSol, k, rng);

    return 0;
}