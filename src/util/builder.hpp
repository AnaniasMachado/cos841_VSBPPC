#ifndef BUILDER_H
#define BUILDER_H

#include "solution.hpp"

class Builder {
public:
    // Paper (cost-based greedy insertion)
    static Solution greedyCost(const VSBPPCInstance& inst,
                               double kw, double kc);

    // First-Fit Decreasing (FFD)
    static Solution firstFitDecreasing(const VSBPPCInstance& inst,
                                       double kw, double kc);

    // Degree-based greedy (BPPC classic)
    static Solution degreeGreedy(const VSBPPCInstance& inst,
                                 double kw, double kc);

    // Best-Fit Decreasing (optional but useful)
    static Solution bestFitDecreasing(const VSBPPCInstance& inst,
                                      double kw, double kc);
};

#endif