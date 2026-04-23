#ifndef SOLUTION_H
#define SOLUTION_H

#include "vsbppc.hpp"
#include <vector>

class Solution {
public:
    const VSBPPCInstance& inst;

    int N;
    int B;

    double kw, kc;

    // assignment
    std::vector<int> itemBin;
    std::vector<std::vector<int>> itemsInBin;

    // load
    std::vector<int> binLoad;
    std::vector<int> binType;

    // conflicts
    std::vector<std::vector<int>> confCount; // confCount[i][b]
    std::vector<int> binConflicts;           // conflicts per bin

    // stats
    long long totalConflicts;
    long long totalExcess;

    // bins with violations
    std::vector<int> badBins;

    Solution(const VSBPPCInstance& instance,
             double kw_, double kc_);

    void addItem(int i, int b);
    void removeItem(int i);
    void moveItem(int i, int from, int to);
    void swapItems(int i, int j);

    void updateBadBin(int b);
    bool isFeasible() const;

    double computeObjective() const;
};

double deltaSwapSubsets(
    const Solution& sol,
    const std::vector<int>& A, // from b1 -> b2
    const std::vector<int>& B, // from b2 -> b1
    int b1, int b2);

#endif