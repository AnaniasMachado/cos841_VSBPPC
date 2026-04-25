#ifndef LNSA_H
#define LNSA_H

#include "../util/solution.hpp"
#include "../util/builder.hpp"

#include <random>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

struct LNSAParams {
    int iterationLimit = 1000;
    double destroyPercent = 10.0;
    int lowerBound = -1;
};

class LNSAAlgorithm {
public:
    static Solution solve(const VSBPPCInstance& inst,
                      int kw,
                      int kc,
                      const LNSAParams& params,
                      std::mt19937& rng);

    static bool localSearchPass(Solution& sol);
    static void localSearchToLocalOptimum(Solution& sol);

private:
    static int smallestFeasibleType(const VSBPPCInstance& inst, int load);
    static int binCost(const VSBPPCInstance& inst, int type);

    static bool canInsert(const Solution& sol, int item, int bin);
    static double insertionUnitCost(const Solution& sol, int item, int bin);

    static std::vector<int> usedBins(const Solution& sol);
    static Solution compactSolution(const Solution& src);

    static int objectiveDeltaTransfer(const Solution& sol,
                                      int item,
                                      int from,
                                      int to);

    static bool feasibleTransfer(const Solution& sol,
                                 int item,
                                 int from,
                                 int to);

    static int objectiveDeltaSwap(const Solution& sol,
                                  int item1,
                                  int item2,
                                  int bin1,
                                  int bin2);

    static bool feasibleSwap(const Solution& sol,
                             int item1,
                             int item2,
                             int bin1,
                             int bin2);

    static bool tryTransfer(Solution& sol);
    static bool trySwap(Solution& sol);

    static Solution destroyBest(const Solution& best,
                                double destroyPercent,
                                std::mt19937& rng);
};

#endif