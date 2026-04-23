#ifndef BPPC_H
#define BPPC_H

#include <vector>
#include <string>
#include <cstdint>

enum class InstanceSet {
    SET1,
    SET2
};

enum class CostType {
    LINEAR,
    CONVEX,
    CONCAVE
};

struct BinType {
    int capacity;
    int cost;
};

class VSBPPCInstance {
public:
    int N;
    std::vector<int> weights;

    // Hybrid representation
    std::vector<std::vector<uint64_t>> conflicts; // bitset
    std::vector<std::vector<int>> neighbors;      // adjacency list
    std::vector<int> degree;

    std::vector<BinType> binTypes;

    VSBPPCInstance(
        int N_,
        std::vector<int> weights_,
        std::vector<std::vector<uint64_t>> conflicts_,
        std::vector<std::vector<int>> neighbors_,
        std::vector<int> degree_,
        std::vector<BinType> binTypes_
    );

    void print() const;
    void printStatistics() const;
};

VSBPPCInstance readInstance(
    const std::string& filename,
    InstanceSet setType,
    CostType costType = CostType::LINEAR
);

#endif