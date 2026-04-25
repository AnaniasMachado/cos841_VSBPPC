#ifndef SOLUTION_H
#define SOLUTION_H

#include "vsbppc.hpp"
#include <vector>
#include <stdexcept>

class Solution {
public:
    const VSBPPCInstance& inst;

    int N;
    int B;

    int kw, kc;

    // assignment
    std::vector<int> itemBin;
    std::vector<std::vector<int>> itemsInBin;

    // load
    std::vector<int> binLoad;
    std::vector<int> binType;
    bool autoBinSizing;
    // size level of each bin (same as binType, but explicit for clarity)
    std::vector<int> binLevel;
    // maximum level (same for all bins in an instance)
    int maxBinLevel;

    // conflicts
    std::vector<std::vector<int>> confCount; // confCount[i][b]
    std::vector<int> binConflicts;           // conflicts per bin

    // stats
    long long totalConflicts;
    long long totalExcess;

    // bins with violations
    std::vector<int> badBins;

    Solution(const VSBPPCInstance& instance,
         int kw_,
         int kc_,
         bool autoBinSizing_ = true);

    void addItem(int i, int b);
    void removeItem(int i);
    void moveItem(int i, int from, int to);
    void swapItems(int i, int j);

    bool increaseBinLevel(int b);
    void updateBadBin(int b);
    bool isFeasible() const;

    int computeObjective() const;

    Solution(const Solution& other) = default;
    Solution& operator=(const Solution& other);
};

int deltaSwapSubsets(
    const Solution& sol,
    const std::vector<int>& A, // from b1 -> b2
    const std::vector<int>& B, // from b2 -> b1
    int b1, int b2);

int deltaIncreaseBinLevel(const Solution& sol, int b);

#endif