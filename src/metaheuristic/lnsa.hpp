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

// LNSA execution mode:
// - VANILLA: original LNSA destroy + Ali-Ekici constructive heuristic.
// - SFC: SFC-style probabilistic destroy + HC1 constructive heuristic.
enum class LNSAMode {
    VANILLA,
    SFC
};

struct LNSAParams {
    // VANILLA: maximum number of iterations.
    // SFC: patience, i.e., maximum number of consecutive non-improving
    //      iterations before stopping, matching the attached SFC code style.
    int iterationLimit = 1000;

    // Percentage in [0, 100]. For SFC this is converted to probability p / 100.
    double destroyPercent = 10.0;
    
    LNSAMode mode = LNSAMode::VANILLA;
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
    static std::vector<int> shuffledUsedBins(const Solution& sol,
                                             std::mt19937& rng);
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

    static bool sfcLocalSearchPass(Solution& sol, std::mt19937& rng);
    static void sfcLocalSearchToLocalOptimum(Solution& sol,
                                             std::mt19937& rng);

    static bool trySFCReinsertion(Solution& sol, std::mt19937& rng);
    static bool trySFCSwap(Solution& sol, std::mt19937& rng);

    static bool sfcBetterReinsertion(const Solution& sol,
                                     int item,
                                     int from,
                                     int to);

    static bool sfcBetterSwap(const Solution& sol,
                              int item1,
                              int item2,
                              int bin1,
                              int bin2);

    static Solution solveVanilla(const VSBPPCInstance& inst,
                                 int kw,
                                 int kc,
                                 const LNSAParams& params,
                                 std::mt19937& rng);

    static Solution solveSFC(const VSBPPCInstance& inst,
                             int kw,
                             int kc,
                             const LNSAParams& params,
                             std::mt19937& rng);

    static Solution destroyVanilla(const Solution& sol,
                                   double destroyPercent,
                                   std::mt19937& rng);

    static Solution destroySFC(const Solution& sol,
                               double destroyPercent,
                               std::mt19937& rng);
};

#endif
