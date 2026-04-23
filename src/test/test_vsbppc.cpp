#include <iostream>
#include <string>
#include "../util/vsbppc.hpp"

int main() {
    // ---- Default values ----
    std::string filename = "../instances/set_1/N_100/Correia_Random_1_1_1_1.txt";

    InstanceSet setType = InstanceSet::SET1;
    CostType costType   = CostType::LINEAR;

    // ---- Read instance ----
    VSBPPCInstance instance = readInstance(filename, setType, costType);

    std::cout << "Instance successfully read!\n\n";

    instance.print();

    // ---- Quick checks ----
    std::cout << "\n--- Quick checks ---\n";
    std::cout << "Number of items: " << instance.N << "\n";
    std::cout << "Weight of item 1: " << instance.weights[0] << "\n";

    std::cout << "\nBin types:\n";
    for (size_t k = 0; k < instance.binTypes.size(); k++) {
        std::cout << "  Type " << k
                  << ": capacity=" << instance.binTypes[k].capacity
                  << ", cost=" << instance.binTypes[k].cost << "\n";
    }

    // ---- Print conflicts of item 1 ----
    std::cout << "\nConflicts of item 1: { ";
    for (int v : instance.neighbors[0]) {
        std::cout << (v + 1) << " ";
    }
    std::cout << "}\n";

    return 0;
}