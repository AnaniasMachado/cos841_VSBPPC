#ifndef BUILDER_H
#define BUILDER_H

#include "solution.hpp"

class Builder {
public:
    // Paper (cost-based greedy insertion)
    static Solution greedyCost(const VSBPPCInstance& inst,
                               int kw, int kc);

    // First-Fit Decreasing (FFD)
    static Solution firstFitDecreasing(const VSBPPCInstance& inst,
                                       int kw, int kc);

    // Degree-based greedy (BPPC classic)
    static Solution degreeGreedy(const VSBPPCInstance& inst,
                                 int kw, int kc);

    // Best-Fit Decreasing (optional but useful)
    static Solution bestFitDecreasing(const VSBPPCInstance& inst,
                                      int kw, int kc);
};

#endif