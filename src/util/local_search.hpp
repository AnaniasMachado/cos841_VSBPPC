#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "solution.hpp"
#include <random>
#include <algorithm>
#include <limits>
#include <climits>
#include <unordered_set>
#include <numeric>
#include <vector>
#include <cstddef>
#include <iostream>
#include <functional>
#include "gurobi_c++.h"

enum class ImprovementType {
    FI,
    BI
};

struct SCColumn {
    std::vector<int> items;
    std::vector<char> mask;
    int cost = 0;
    int load = 0;
    int type = -1;

    void buildMask(int n_items) {
        mask.assign(n_items, 0);
        for (int item : items) {
            mask[item] = 1;
        }
    }
};

struct SCPoolEntry {
    SCColumn col;
    std::size_t hash = 0;
};

class LocalSearch {
public:
    Solution* solution;
    int kw, kc;
    ImprovementType improvementType;
    std::mt19937& rng;

    LocalSearch(Solution& sol,
                ImprovementType improvementType_,
                std::mt19937& rng_);

    bool relocate();
    bool exchange();
    bool exchange21();
    bool classic();
    bool ejectionGlobal();
    int hungarian(const std::vector<std::vector<int>>& cost,
                std::vector<int>& assignment);
    bool assignment(int N_ASSIGN);
    bool ejectionChain();
    bool grenade();

    void initializeSetCoveringFromElite(const Solution& elite);
    void addToSetCoveringPool(const Solution& sol);
    bool setCoveringNeighborhood(double timeLimitSeconds = 30.0,
                             bool exactCover = false);

    void setSolution(Solution& sol);

private:
    std::vector<SCPoolEntry> scPool;
    std::unordered_set<std::size_t> scSeen;

    std::vector<std::vector<int>> eliteBins;
    std::unordered_set<std::size_t> eliteHashes;
    std::size_t eliteSolutionHash = 0;
    int eliteObjective = std::numeric_limits<int>::max();

    int scPoolLimit;

    std::size_t hashBin(const std::vector<int>& bin) const;
    std::size_t hashSolution(const std::vector<std::vector<int>>& bins) const;

    int smallestFeasibleTypeSC(const VSBPPCInstance& inst, int load) const;
    int binCostSC(const VSBPPCInstance& inst, int type) const;

    bool isFeasibleColumn(const std::vector<int>& items,
                          int& load,
                          int& type,
                          int& cost) const;

    void addSetCoveringColumn(SCColumn&& col);
    void trimSetCoveringPool();

    std::vector<std::vector<int>> extractBins(const Solution& sol) const;
    Solution buildSolutionFromBins(const std::vector<std::vector<int>>& bins) const;

    std::vector<std::vector<int>> repairSetCoveringSolution(
                const std::vector<std::vector<int>>& inputBins);

    void applySwapSubsets(const std::vector<int>& A,
                          const std::vector<int>& B,
                          int b1, int b2);
};

#endif