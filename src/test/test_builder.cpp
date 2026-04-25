#include <iostream>
#include <chrono>
#include <string>
#include <vector>

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"

void printResult(const std::string& name,
                 const Solution& sol,
                 double elapsed) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "Time: " << elapsed << " s\n";
    std::cout << "Objective: " << sol.computeObjective() << "\n";
    std::cout << "Feasible: " << (sol.isFeasible() ? "YES" : "NO") << "\n\n";
}

void testBuilder(
    const std::string& name,
    Solution (*builder)(const VSBPPCInstance&, int, int, const Solution*),
    const VSBPPCInstance& inst,
    int kw,
    int kc
) {
    auto start = std::chrono::high_resolution_clock::now();

    Solution sol = builder(inst, kw, kc, nullptr);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    printResult(name, sol, elapsed.count());
}

void testBuilderAlpha(
    const std::string& name,
    Solution (*builder)(const VSBPPCInstance&, int, int, double, const Solution*),
    const VSBPPCInstance& inst,
    int kw,
    int kc,
    double alpha
) {
    auto start = std::chrono::high_resolution_clock::now();

    Solution sol = builder(inst, kw, kc, alpha, nullptr);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    printResult(name + " alpha=" + std::to_string(alpha), sol, elapsed.count());
}

int main() {
    std::string filename = "../instances/set_1/N_100/Correia_Random_1_1_1_1.txt";

    InstanceSet setType = InstanceSet::SET1;
    CostType costType = CostType::LINEAR;
    BinSizeSetting binSizeSetting = BinSizeSetting::THREE_TYPES;

    int kw = 1000;
    int kc = 1000;

    VSBPPCInstance inst = readInstance(filename, setType, costType, binSizeSetting);

    std::cout << "Instance loaded: N = " << inst.N << "\n\n";

    testBuilder("Greedy Cost", Builder::greedyCost, inst, kw, kc);
    testBuilder("Degree Greedy", Builder::degreeGreedy, inst, kw, kc);
    testBuilder("First Fit Decreasing", Builder::firstFitDecreasing, inst, kw, kc);
    testBuilder("Best Fit Decreasing", Builder::bestFitDecreasing, inst, kw, kc);
    testBuilder("HC1", Builder::HC1, inst, kw, kc);
    testBuilder("HC2", Builder::HC2, inst, kw, kc);
    testBuilder("Ali Ekici Constructive", Builder::aliEkiciConstructive, inst, kw, kc);

    std::vector<double> alphas = {0.0, 0.25, 0.5, 0.75, 1.0};

    for (double alpha : alphas) {
        testBuilderAlpha("IFFD", Builder::IFFD, inst, kw, kc, alpha);
        testBuilderAlpha("IBFD", Builder::IBFD, inst, kw, kc, alpha);
    }

    return 0;
}