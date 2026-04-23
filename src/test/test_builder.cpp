#include <iostream>
#include <chrono>
#include <string>

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"

void testBuilder(const std::string& name,
                 Solution (*builder)(const VSBPPCInstance&, int, int),
                 const VSBPPCInstance& inst,
                 int kw, int kc) {

    auto start = std::chrono::high_resolution_clock::now();

    Solution sol = builder(inst, kw, kc);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "=== " << name << " ===\n";
    std::cout << "Time: " << elapsed.count() << " s\n";
    std::cout << "Objective: " << sol.computeObjective() << "\n";
    std::cout << "Feasible: " << (sol.isFeasible() ? "YES" : "NO") << "\n\n";
}

int main() {
    std::string filename = "../instances/set_1/N_100/Correia_Random_1_1_1_1.txt";

    InstanceSet setType = InstanceSet::SET1;
    CostType costType = CostType::LINEAR;

    int kw = 1000;
    int kc = 1000;

    VSBPPCInstance inst = readInstance(filename, setType, costType);

    std::cout << "Instance loaded: N = " << inst.N << "\n\n";

    testBuilder("Greedy Cost", Builder::greedyCost, inst, kw, kc);
    testBuilder("First Fit Decreasing", Builder::firstFitDecreasing, inst, kw, kc);
    testBuilder("Degree Greedy", Builder::degreeGreedy, inst, kw, kc);
    testBuilder("Best Fit Decreasing", Builder::bestFitDecreasing, inst, kw, kc);

    return 0;
}