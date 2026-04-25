#include "vsbppc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>

// ---- Bin types ----
static std::vector<BinType> buildBinTypes(
    InstanceSet setType,
    CostType costType,
    BinSizeSetting binSizeSetting) {

    std::vector<int> capacities;

    if (setType == InstanceSet::SET1) {
        if (binSizeSetting == BinSizeSetting::THREE_TYPES) {
            capacities = {100, 120, 150};
        } else if (binSizeSetting == BinSizeSetting::FIVE_TYPES) {
            capacities = {60, 80, 100, 120, 150};
        } else {
            throw std::runtime_error(
                "Invalid bin size setting for Set-1. Use THREE_TYPES or FIVE_TYPES."
            );
        }
    } else {
        if (binSizeSetting != BinSizeSetting::SEVEN_TYPES) {
            throw std::runtime_error(
                "Invalid bin size setting for Set-2. Use SEVEN_TYPES."
            );
        }

        capacities = {70, 100, 130, 160, 190, 220, 250};
    }

    std::vector<BinType> types;
    types.reserve(capacities.size());

    for (int c : capacities) {
        int cost;

        if (setType == InstanceSet::SET1 || costType == CostType::LINEAR) {
            cost = c;
        } else if (costType == CostType::CONVEX) {
            cost = static_cast<int>(std::ceil(0.1 * std::pow(c, 1.5)));
        } else {
            cost = static_cast<int>(std::ceil(10.0 * std::sqrt(c)));
        }

        types.push_back({c, cost});
    }

    std::sort(types.begin(), types.end(),
        [](const BinType& a, const BinType& b) {
            return a.capacity < b.capacity;
        });

    return types;
}

// ---- Constructor ----
VSBPPCInstance::VSBPPCInstance(
    int N_,
    std::vector<int> weights_,
    std::vector<std::vector<uint64_t>> conflicts_,
    std::vector<std::vector<int>> neighbors_,
    std::vector<int> degree_,
    InstanceSet setType_,
    CostType costType_,
    BinSizeSetting binSizeSetting_)
    : N(N_),
      weights(std::move(weights_)),
      conflicts(std::move(conflicts_)),
      neighbors(std::move(neighbors_)),
      degree(std::move(degree_)),
      setType(setType_),
      costType(costType_),
      binSizeSetting(binSizeSetting_),
      binTypes(buildBinTypes(setType_, costType_, binSizeSetting_)) {}

// ---- Print ----
void VSBPPCInstance::print() const {
    std::cout << "N = " << N << "\n";

    std::cout << "Bin types:\n";
    for (std::size_t k = 0; k < binTypes.size(); k++) {
        std::cout << "  Type " << k
                  << ": capacity=" << binTypes[k].capacity
                  << ", cost=" << binTypes[k].cost << "\n";
    }

    for (int i = 0; i < N; i++) {
        std::cout << "Item " << (i + 1)
                  << ": w=" << weights[i]
                  << ", conflicts={ ";

        for (int j = 0; j < N; j++) {
            int word = j >> 6;
            int bit = j & 63;

            if (conflicts[i][word] & (1ULL << bit)) {
                std::cout << (j + 1) << " ";
            }
        }

        std::cout << "}\n";
    }
}

// ---- Statistics ----
void VSBPPCInstance::printStatistics() const {
    long long edge_count = 0;

    for (int i = 0; i < N; i++) {
        for (uint64_t w : conflicts[i]) {
            edge_count += __builtin_popcountll(w);
        }
    }

    edge_count /= 2;

    std::cout << "Instance statistics:\n";
    std::cout << "Number of items (N): " << N << "\n";
    std::cout << "Number of conflicts (|E|): " << edge_count << "\n";
    std::cout << "Number of bin types: " << binTypes.size() << "\n";

    for (std::size_t k = 0; k < binTypes.size(); k++) {
        std::cout << "  Bin type " << k
                  << ": capacity=" << binTypes[k].capacity
                  << ", cost=" << binTypes[k].cost << "\n";
    }
}

// ---- Read instance ----
VSBPPCInstance readInstance(
    const std::string& filename,
    InstanceSet setType,
    CostType costType,
    BinSizeSetting binSizeSetting) {

    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    int N;
    file >> N;

    std::vector<int> weights(N, 0);

    int W = (N + 63) / 64;

    std::vector<std::vector<uint64_t>> conflicts(
        N, std::vector<uint64_t>(W, 0ULL)
    );

    std::vector<std::vector<int>> neighbors(N);
    std::vector<int> degree(N, 0);

    auto addConflictEdge = [&](int i, int j) {
        if (i < 0 || i >= N || j < 0 || j >= N || i == j) return;

        int wordJ = j >> 6;
        int bitJ  = j & 63;

        if (conflicts[i][wordJ] & (1ULL << bitJ)) return;

        conflicts[i][wordJ] |= (1ULL << bitJ);
        conflicts[j][i >> 6] |= (1ULL << (i & 63));

        neighbors[i].push_back(j);
        neighbors[j].push_back(i);

        degree[i]++;
        degree[j]++;
    };

    int idx, w;
    while (file >> idx >> w) {
        idx--;

        if (idx < 0 || idx >= N) {
            throw std::runtime_error("Invalid item index in file.");
        }

        weights[idx] = w;

        std::string line;
        std::getline(file, line);
        std::stringstream ss(line);

        int a;
        while (ss >> a) {
            a--;

            if (a < 0 || a >= N || a == idx) continue;

            addConflictEdge(idx, a);
        }
    }

    // =====================================================
    // EXTENDED CONFLICT GRAPH
    // Comment/uncomment this block to compare behavior.
    //
    // If two items cannot fit together in the largest bin,
    // they are treated as conflicting, as in Ekici (2023).
    // =====================================================
    std::vector<BinType> binTypes =
        buildBinTypes(setType, costType, binSizeSetting);

    int maxCapacity = binTypes.back().capacity;

    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            if (weights[i] + weights[j] > maxCapacity) {
                addConflictEdge(i, j);
            }
        }
    }

    return VSBPPCInstance(
        N,
        weights,
        conflicts,
        neighbors,
        degree,
        setType,
        costType,
        binSizeSetting
    );
}