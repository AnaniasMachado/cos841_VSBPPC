#ifndef BUILDER_H
#define BUILDER_H

#include "solution.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <memory>

class Builder {
public:
    // Greedy Cost (cost-based greedy insertion)
    static Solution greedyCost(const VSBPPCInstance& inst,
                               int kw, int kc,
                               const Solution* partial = nullptr);

    // Degree-based greedy (BPPC classic)
    static Solution degreeGreedy(const VSBPPCInstance& inst,
                                int kw, int kc,
                                const Solution* partial = nullptr);

    // First-Fit Decreasing (FFD)
    static Solution firstFitDecreasing(const VSBPPCInstance& inst,
                                int kw, int kc,
                                const Solution* partial = nullptr);

    // Best-Fit Decreasing (BFD)
    static Solution bestFitDecreasing(const VSBPPCInstance& inst,
                                int kw, int kc,
                                const Solution* partial = nullptr);

    // Ali Ekici constructive
    static Solution aliEkiciConstructive(const VSBPPCInstance& inst,
                                int kw,int kc,
                                const Solution* partial = nullptr);

    // Iterated FFD (IFFD)
    static Solution IFFD(const VSBPPCInstance& inst,
                                int kw, int kc,
                                double alpha,
                                const Solution* partial = nullptr);

    // Iterated BFD (IBFD)
    static Solution IBFD(const VSBPPCInstance& inst,
                                int kw, int kc,
                                double alpha,
                                const Solution* partial = nullptr);

    static Solution HC1(const VSBPPCInstance& inst,
                                int kw,
                                int kc,
                                const Solution* partial = nullptr);

    static Solution HC2(const VSBPPCInstance& inst,
                                int kw,
                                int kc,
                                const Solution* partial = nullptr);
};

#endif