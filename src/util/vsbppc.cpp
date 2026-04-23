#include "vsbppc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>

// ---- Constructor ----
VSBPPCInstance::VSBPPCInstance(
    int N_,
    std::vector<int> weights_,
    std::vector<std::vector<uint64_t>> conflicts_,
    std::vector<std::vector<int>> neighbors_,
    std::vector<int> degree_,
    std::vector<BinType> binTypes_)
    : N(N_),
      weights(std::move(weights_)),
      conflicts(std::move(conflicts_)),
      neighbors(std::move(neighbors_)),
      degree(std::move(degree_)),
      binTypes(std::move(binTypes_)) {}

// ---- Bin types ----
static std::vector<BinType> buildBinTypes(InstanceSet setType, CostType costType) {
    std::vector<int> capacities;

    if (setType == InstanceSet::SET1) {
        capacities = {60, 80, 100, 120, 150};
    } else {
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

    return types;
}

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
    CostType costType) {
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

            int word = a >> 6;
            int bit  = a & 63;

            if (!(conflicts[idx][word] & (1ULL << bit))) {
                conflicts[idx][word] |= (1ULL << bit);
                conflicts[a][idx >> 6] |= (1ULL << (idx & 63));

                neighbors[idx].push_back(a);
                neighbors[a].push_back(idx);

                degree[idx]++;
                degree[a]++;
            }
        }
    }

    auto binTypes = buildBinTypes(setType, costType);

    return VSBPPCInstance(N, weights, conflicts, neighbors, degree, binTypes);
}