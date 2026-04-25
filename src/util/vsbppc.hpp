#ifndef VSBPPC_HPP
#define VSBPPC_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

enum class InstanceSet {
    SET1,
    SET2
};

enum class CostType {
    LINEAR,
    CONVEX,
    CONCAVE
};

enum class BinSizeSetting {
    THREE_TYPES,
    FIVE_TYPES,
    SEVEN_TYPES
};

struct BinType {
    int capacity;
    int cost;
};

class VSBPPCInstance {
public:
    int N;
    std::vector<int> weights;
    std::vector<std::vector<uint64_t>> conflicts;
    std::vector<std::vector<int>> neighbors;
    std::vector<int> degree;
    std::vector<BinType> binTypes;

    InstanceSet setType;
    CostType costType;
    BinSizeSetting binSizeSetting;

    VSBPPCInstance(
        int N_,
        std::vector<int> weights_,
        std::vector<std::vector<uint64_t>> conflicts_,
        std::vector<std::vector<int>> neighbors_,
        std::vector<int> degree_,
        InstanceSet setType_,
        CostType costType_,
        BinSizeSetting binSizeSetting_
    );

    void print() const;
    void printStatistics() const;
};

VSBPPCInstance readInstance(
    const std::string& filename,
    InstanceSet setType,
    CostType costType,
    BinSizeSetting binSizeSetting
);

#endif