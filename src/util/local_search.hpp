#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "solution.hpp"
#include <random>

enum class ImprovementType {
    FI,
    BI
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

    void setSolution(Solution& sol);

private:
    void applySwapSubsets(const std::vector<int>& A,
                          const std::vector<int>& B,
                          int b1, int b2);
};

#endif