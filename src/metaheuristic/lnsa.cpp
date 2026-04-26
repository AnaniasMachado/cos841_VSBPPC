#include "lnsa.hpp"

int LNSAAlgorithm::smallestFeasibleType(const VSBPPCInstance& inst, int load) {
    if (load <= 0) return -1;

    for (int k = 0; k < static_cast<int>(inst.binTypes.size()); ++k) {
        if (load <= inst.binTypes[k].capacity) return k;
    }

    return -1;
}

int LNSAAlgorithm::binCost(const VSBPPCInstance& inst, int type) {
    return type < 0 ? 0 : inst.binTypes[type].cost;
}

std::vector<int> LNSAAlgorithm::usedBins(const Solution& sol) {
    std::vector<int> bins;

    for (int b = 0; b < sol.B; ++b) {
        if (!sol.itemsInBin[b].empty()) {
            bins.push_back(b);
        }
    }

    return bins;
}

Solution LNSAAlgorithm::compactSolution(const Solution& src) {
    Solution compact(src.inst, src.kw, src.kc);

    int nextBin = 0;

    for (int b = 0; b < src.B; ++b) {
        if (src.itemsInBin[b].empty()) continue;

        for (int item : src.itemsInBin[b]) {
            compact.addItem(item, nextBin);
        }

        ++nextBin;
    }

    return compact;
}

bool LNSAAlgorithm::canInsert(const Solution& sol, int item, int bin) {
    if (bin < 0 || bin >= sol.B) return false;
    if (sol.confCount[item][bin] != 0) return false;

    int newLoad = sol.binLoad[bin] + sol.inst.weights[item];
    return smallestFeasibleType(sol.inst, newLoad) != -1;
}

double LNSAAlgorithm::insertionUnitCost(const Solution& sol, int item, int bin) {
    if (!canInsert(sol, item, bin)) {
        return std::numeric_limits<double>::infinity();
    }

    int oldType = sol.binType[bin];
    int oldCost = binCost(sol.inst, oldType);

    int newLoad = sol.binLoad[bin] + sol.inst.weights[item];
    int newType = smallestFeasibleType(sol.inst, newLoad);
    int newCost = binCost(sol.inst, newType);

    return static_cast<double>(newCost - oldCost) /
           static_cast<double>(sol.inst.weights[item]);
}

bool LNSAAlgorithm::feasibleTransfer(const Solution& sol,
                                     int item,
                                     int from,
                                     int to) {
    if (from == to) return false;
    if (to < 0 || to >= sol.B) return false;

    if (sol.confCount[item][to] != 0) return false;

    int newToLoad = sol.binLoad[to] + sol.inst.weights[item];
    return smallestFeasibleType(sol.inst, newToLoad) != -1;
}

int LNSAAlgorithm::objectiveDeltaTransfer(const Solution& sol,
                                          int item,
                                          int from,
                                          int to) {
    const auto& inst = sol.inst;
    int w = inst.weights[item];

    int oldFromType = sol.binType[from];
    int oldToType = sol.binType[to];

    int newFromLoad = sol.binLoad[from] - w;
    int newToLoad = sol.binLoad[to] + w;

    int newFromType = smallestFeasibleType(inst, newFromLoad);
    int newToType = smallestFeasibleType(inst, newToLoad);

    if (newToType == -1) {
        return std::numeric_limits<int>::max();
    }

    int oldCost = binCost(inst, oldFromType) + binCost(inst, oldToType);
    int newCost = binCost(inst, newFromType) + binCost(inst, newToType);

    return newCost - oldCost;
}

bool LNSAAlgorithm::tryTransfer(Solution& sol) {
    const std::vector<int> bins = usedBins(sol);

    if (bins.empty()) {
        return false;
    }

    int firstEmpty = -1;
    for (int b = 0; b < sol.B; ++b) {
        if (sol.itemsInBin[b].empty()) {
            firstEmpty = b;
            break;
        }
    }

    std::vector<int> targetBins = bins;
    if (firstEmpty != -1) {
        targetBins.push_back(firstEmpty); // Algorithm 2 also considers T + 1.
    }

    auto contentCostRatio = [&](int load, int type) -> double {
        int cost = binCost(sol.inst, type);
        if (load <= 0 || cost <= 0) return 0.0;
        return static_cast<double>(load) / static_cast<double>(cost);
    };

    auto improvesLikeAlgorithm2 = [&](int item, int from, int to) -> bool {
        if (!feasibleTransfer(sol, item, from, to)) return false;

        const auto& inst = sol.inst;
        const int w = inst.weights[item];

        const int oldFromType = sol.binType[from];
        const int oldToType = sol.binType[to];

        const int newFromLoad = sol.binLoad[from] - w;
        const int newToLoad = sol.binLoad[to] + w;

        int newFromType = smallestFeasibleType(inst, newFromLoad);
        int newToType = smallestFeasibleType(inst, newToLoad);

        if (newFromLoad <= 0) newFromType = -1;
        if (newToType == -1) return false;

        const int oldCost = binCost(inst, oldFromType) +
                            binCost(inst, oldToType);
        const int newCost = binCost(inst, newFromType) +
                            binCost(inst, newToType);

        if (newCost < oldCost) return true;
        if (newCost > oldCost) return false;

        // Algorithm 2 only applies the equal-cost utilization tie-break when
        // moving to an existing bin, not to the artificial empty bin T + 1.
        if (sol.itemsInBin[to].empty()) return false;

        const double oldUtil = std::max(
            contentCostRatio(sol.binLoad[from], oldFromType),
            contentCostRatio(sol.binLoad[to], oldToType)
        );

        const double newUtil = std::max(
            contentCostRatio(newFromLoad, newFromType),
            contentCostRatio(newToLoad, newToType)
        );

        return newUtil > oldUtil;
    };

    // First-improvement version of Algorithm 2.
    for (int from : bins) {
        const std::vector<int> items = sol.itemsInBin[from];

        for (int item : items) {
            for (int to : targetBins) {
                if (from == to) continue;
                if (!improvesLikeAlgorithm2(item, from, to)) continue;

                sol.moveItem(item, from, to);

                if (sol.itemsInBin[from].empty()) {
                    sol = compactSolution(sol);
                }

                return true;
            }
        }
    }

    return false;
}

bool LNSAAlgorithm::feasibleSwap(const Solution& sol,
                                 int item1,
                                 int item2,
                                 int bin1,
                                 int bin2) {
    if (bin1 == bin2) return false;

    const auto& inst = sol.inst;

    int w1 = inst.weights[item1];
    int w2 = inst.weights[item2];

    int newLoad1 = sol.binLoad[bin1] - w1 + w2;
    int newLoad2 = sol.binLoad[bin2] - w2 + w1;

    if (smallestFeasibleType(inst, newLoad1) == -1) return false;
    if (smallestFeasibleType(inst, newLoad2) == -1) return false;

    // item2 entering bin1: conflicts with bin1 except item1, which leaves.
    for (int other : sol.itemsInBin[bin1]) {
        if (other == item1) continue;

        int word = other >> 6;
        int bit = other & 63;

        if (inst.conflicts[item2][word] & (1ULL << bit)) {
            return false;
        }
    }

    // item1 entering bin2: conflicts with bin2 except item2, which leaves.
    for (int other : sol.itemsInBin[bin2]) {
        if (other == item2) continue;

        int word = other >> 6;
        int bit = other & 63;

        if (inst.conflicts[item1][word] & (1ULL << bit)) {
            return false;
        }
    }

    return true;
}

int LNSAAlgorithm::objectiveDeltaSwap(const Solution& sol,
                                      int item1,
                                      int item2,
                                      int bin1,
                                      int bin2) {
    const auto& inst = sol.inst;

    int w1 = inst.weights[item1];
    int w2 = inst.weights[item2];

    int oldType1 = sol.binType[bin1];
    int oldType2 = sol.binType[bin2];

    int newLoad1 = sol.binLoad[bin1] - w1 + w2;
    int newLoad2 = sol.binLoad[bin2] - w2 + w1;

    int newType1 = smallestFeasibleType(inst, newLoad1);
    int newType2 = smallestFeasibleType(inst, newLoad2);

    if (newType1 == -1 || newType2 == -1) {
        return std::numeric_limits<int>::max();
    }

    int oldCost = binCost(inst, oldType1) + binCost(inst, oldType2);
    int newCost = binCost(inst, newType1) + binCost(inst, newType2);

    return newCost - oldCost;
}

bool LNSAAlgorithm::trySwap(Solution& sol) {
    const std::vector<int> bins = usedBins(sol);

    auto contentCostRatio = [&](int load, int type) -> double {
        int cost = binCost(sol.inst, type);
        if (load <= 0 || cost <= 0) return 0.0;
        return static_cast<double>(load) / static_cast<double>(cost);
    };

    auto improvesLikeAlgorithm3 = [&](int item1,
                                      int item2,
                                      int bin1,
                                      int bin2) -> bool {
        if (!feasibleSwap(sol, item1, item2, bin1, bin2)) return false;

        const auto& inst = sol.inst;

        const int w1 = inst.weights[item1];
        const int w2 = inst.weights[item2];

        const int oldType1 = sol.binType[bin1];
        const int oldType2 = sol.binType[bin2];

        const int newLoad1 = sol.binLoad[bin1] - w1 + w2;
        const int newLoad2 = sol.binLoad[bin2] - w2 + w1;

        const int newType1 = smallestFeasibleType(inst, newLoad1);
        const int newType2 = smallestFeasibleType(inst, newLoad2);

        if (newType1 == -1 || newType2 == -1) return false;

        const int oldCost = binCost(inst, oldType1) +
                            binCost(inst, oldType2);
        const int newCost = binCost(inst, newType1) +
                            binCost(inst, newType2);

        if (newCost < oldCost) return true;
        if (newCost > oldCost) return false;

        const double oldUtil = std::max(
            contentCostRatio(sol.binLoad[bin1], oldType1),
            contentCostRatio(sol.binLoad[bin2], oldType2)
        );

        const double newUtil = std::max(
            contentCostRatio(newLoad1, newType1),
            contentCostRatio(newLoad2, newType2)
        );

        return newUtil > oldUtil;
    };

    // First-improvement version of Algorithm 3.
    for (std::size_t a = 0; a < bins.size(); ++a) {
        for (std::size_t b = a + 1; b < bins.size(); ++b) {
            int bin1 = bins[a];
            int bin2 = bins[b];

            const std::vector<int> items1 = sol.itemsInBin[bin1];
            const std::vector<int> items2 = sol.itemsInBin[bin2];

            for (int item1 : items1) {
                for (int item2 : items2) {
                    if (!improvesLikeAlgorithm3(item1, item2, bin1, bin2)) {
                        continue;
                    }

                    sol.swapItems(item1, item2);
                    return true;
                }
            }
        }
    }

    return false;
}

bool LNSAAlgorithm::localSearchPass(Solution& sol) {
    if (tryTransfer(sol)) return true;
    if (trySwap(sol)) return true;
    return false;
}

void LNSAAlgorithm::localSearchToLocalOptimum(Solution& sol) {
    while (localSearchPass(sol)) {
    }
}


std::vector<int> LNSAAlgorithm::shuffledUsedBins(const Solution& sol,
                                                 std::mt19937& rng) {
    std::vector<int> bins = usedBins(sol);
    std::shuffle(bins.begin(), bins.end(), rng);
    return bins;
}

bool LNSAAlgorithm::sfcBetterReinsertion(const Solution& sol,
                                         int item,
                                         int from,
                                         int to) {
    if (!feasibleTransfer(sol, item, from, to)) return false;

    const auto& inst = sol.inst;
    const int size = inst.weights[item];

    const int oldFromType = sol.binType[from];
    const int oldToType = sol.binType[to];

    const int newFromLoad = sol.binLoad[from] - size;
    const int newToLoad = sol.binLoad[to] + size;

    const int newFromType = smallestFeasibleType(inst, newFromLoad);
    const int newToType = smallestFeasibleType(inst, newToLoad);

    if (newToType == -1) return false;

    const int oldCost = binCost(inst, oldFromType) + binCost(inst, oldToType);
    const int newCost = (newFromLoad <= 0 ? 0 : binCost(inst, newFromType)) +
                        binCost(inst, newToType);

    if (oldCost > newCost) return true;
    if (oldCost < newCost) return false;

    const double oldUtil = std::max(
        static_cast<double>(sol.binLoad[from]) /
            static_cast<double>(inst.binTypes[oldFromType].capacity),
        static_cast<double>(sol.binLoad[to]) /
            static_cast<double>(inst.binTypes[oldToType].capacity)
    );

    const double newFromUtil =
        newFromLoad <= 0
            ? 0.0
            : static_cast<double>(newFromLoad) /
              static_cast<double>(inst.binTypes[newFromType].capacity);

    const double newToUtil =
        static_cast<double>(newToLoad) /
        static_cast<double>(inst.binTypes[newToType].capacity);

    const double newUtil = std::max(newFromUtil, newToUtil);

    return oldUtil < newUtil;
}

bool LNSAAlgorithm::trySFCReinsertion(Solution& sol, std::mt19937& rng) {
    const std::vector<int> bins = shuffledUsedBins(sol, rng);

    for (int from : bins) {
        const std::vector<int> fromItems = sol.itemsInBin[from];

        std::vector<int> toBins;
        toBins.reserve(bins.size() > 0 ? bins.size() - 1 : 0);
        for (int b : bins) {
            if (b != from) toBins.push_back(b);
        }

        for (int item : fromItems) {
            for (int to : toBins) {
                if (!sfcBetterReinsertion(sol, item, from, to)) continue;

                sol.moveItem(item, from, to);

                if (sol.itemsInBin[from].empty()) {
                    sol = compactSolution(sol);
                }

                return true;
            }
        }
    }

    return false;
}

bool LNSAAlgorithm::sfcBetterSwap(const Solution& sol,
                                  int item1,
                                  int item2,
                                  int bin1,
                                  int bin2) {
    if (!feasibleSwap(sol, item1, item2, bin1, bin2)) return false;

    const auto& inst = sol.inst;

    const int size1 = inst.weights[item1];
    const int size2 = inst.weights[item2];

    const int oldType1 = sol.binType[bin1];
    const int oldType2 = sol.binType[bin2];

    const int newLoad1 = sol.binLoad[bin1] - size1 + size2;
    const int newLoad2 = sol.binLoad[bin2] - size2 + size1;

    const int newType1 = smallestFeasibleType(inst, newLoad1);
    const int newType2 = smallestFeasibleType(inst, newLoad2);

    if (newType1 == -1 || newType2 == -1) return false;

    const int oldCost = binCost(inst, oldType1) + binCost(inst, oldType2);
    const int newCost = binCost(inst, newType1) + binCost(inst, newType2);

    if (oldCost > newCost) return true;
    if (oldCost < newCost) return false;

    const double oldUtil = std::max(
        static_cast<double>(sol.binLoad[bin1]) /
            static_cast<double>(inst.binTypes[oldType1].capacity),
        static_cast<double>(sol.binLoad[bin2]) /
            static_cast<double>(inst.binTypes[oldType2].capacity)
    );

    const double newUtil = std::max(
        static_cast<double>(newLoad1) /
            static_cast<double>(inst.binTypes[newType1].capacity),
        static_cast<double>(newLoad2) /
            static_cast<double>(inst.binTypes[newType2].capacity)
    );

    return oldUtil < newUtil;
}

bool LNSAAlgorithm::trySFCSwap(Solution& sol, std::mt19937& rng) {
    const std::vector<int> bins = shuffledUsedBins(sol, rng);

    for (int bin1 : bins) {
        std::vector<int> otherBins;
        otherBins.reserve(bins.size() > 0 ? bins.size() - 1 : 0);
        for (int b : bins) {
            if (b != bin1) otherBins.push_back(b);
        }

        for (int bin2 : otherBins) {
            const std::vector<int> items1 = sol.itemsInBin[bin1];
            const std::vector<int> items2 = sol.itemsInBin[bin2];

            for (int item1 : items1) {
                for (int item2 : items2) {
                    if (!sfcBetterSwap(sol, item1, item2, bin1, bin2)) continue;

                    sol.swapItems(item1, item2);
                    return true;
                }
            }
        }
    }

    return false;
}

bool LNSAAlgorithm::sfcLocalSearchPass(Solution& sol, std::mt19937& rng) {
    return trySFCReinsertion(sol, rng) || trySFCSwap(sol, rng);
}

void LNSAAlgorithm::sfcLocalSearchToLocalOptimum(Solution& sol,
                                                 std::mt19937& rng) {
    while (sfcLocalSearchPass(sol, rng)) {
    }
}

Solution LNSAAlgorithm::destroyVanilla(const Solution& sol,
                                       double destroyPercent,
                                       std::mt19937& rng) {
    const std::vector<int> bins = usedBins(sol);

    if (bins.empty()) {
        return sol;
    }

    int removeCount =
        static_cast<int>(std::ceil((destroyPercent / 100.0) * bins.size()));

    removeCount = std::max(1, removeCount);
    removeCount = std::min(removeCount, static_cast<int>(bins.size()));

    std::vector<int> shuffled = bins;
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    std::vector<char> remove(sol.itemsInBin.size(), 0);

    for (int r = 0; r < removeCount; ++r) {
        remove[shuffled[r]] = 1;
    }

    Solution partial(sol.inst, sol.kw, sol.kc);

    int nextBin = 0;

    for (int bin : bins) {
        if (remove[bin]) continue;

        for (int item : sol.itemsInBin[bin]) {
            partial.addItem(item, nextBin);
        }

        ++nextBin;
    }

    return partial;
}

Solution LNSAAlgorithm::destroySFC(const Solution& sol,
                                   double destroyPercent,
                                   std::mt19937& rng) {
    static constexpr double WEIGHT = 8.74;
    static constexpr double OFFSET = 1.26;

    const std::vector<int> bins = usedBins(sol);

    if (bins.empty()) {
        return sol;
    }

    int maxGap = 0;

    for (int bin : bins) {
        int type = sol.binType[bin];
        if (type < 0) continue;

        int gap = sol.inst.binTypes[type].capacity - sol.binLoad[bin];
        maxGap = std::max(maxGap, gap);
    }

    double p = destroyPercent / 100.0;
    p = std::max(0.0, std::min(1.0, p));

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<char> remove(sol.itemsInBin.size(), 0);

    // Visit the current solution's active bins
    // and remove each one with probability p * prob_weight(bin).
    for (int bin : bins) {
        double w = 0.1;

        if (maxGap != 0) {
            int type = sol.binType[bin];
            int free = sol.inst.binTypes[type].capacity - sol.binLoad[bin];
            double coef = static_cast<double>(free) / static_cast<double>(maxGap);
            w = std::log10(WEIGHT * coef + OFFSET);
        }

        if (dist(rng) < p * w) {
            remove[bin] = 1;
        }
    }

    Solution partial(sol.inst, sol.kw, sol.kc);

    int nextBin = 0;

    // Rebuild the current solution without the destroyed bins. This is the
    // equivalent, for this Solution class, of the attached code mutating sol
    // with reloc_bin(idx, &V) and then repairing the remaining partial solution.
    for (int bin : bins) {
        if (remove[bin]) continue;

        for (int item : sol.itemsInBin[bin]) {
            partial.addItem(item, nextBin);
        }

        ++nextBin;
    }

    return partial;
}

Solution LNSAAlgorithm::solveVanilla(const VSBPPCInstance& inst,
                                     int kw,
                                     int kc,
                                     const LNSAParams& params,
                                     std::mt19937& rng) {
    Solution best = Builder::aliEkiciConstructive(inst, kw, kc, nullptr);
    localSearchToLocalOptimum(best);

    for (int it = 0; it < params.iterationLimit; ++it) {

        Solution partial = destroyVanilla(best, params.destroyPercent, rng);
        Solution repaired = Builder::aliEkiciConstructive(inst, kw, kc, &partial);
        localSearchToLocalOptimum(repaired);

        if (repaired.isFeasible() &&
            repaired.computeObjective() < best.computeObjective()) {
            best = repaired;
        }
    }

    return best;
}

Solution LNSAAlgorithm::solveSFC(const VSBPPCInstance& inst,
                                 int kw,
                                 int kc,
                                 const LNSAParams& params,
                                 std::mt19937& rng) {
    // Closer to the attached SFC LNSA:
    //   sol  = current solution, modified every iteration;
    //   best = best solution seen so far;
    //   iterationLimit acts as patience and resets on improvement.
    Solution sol = Builder::HC1(inst, kw, kc, nullptr);
    sfcLocalSearchToLocalOptimum(sol, rng);

    Solution best = sol;

    int iter = 0;
    while (iter <= params.iterationLimit) {

        Solution partial = destroySFC(sol, params.destroyPercent, rng);
        Solution repaired = Builder::HC1(inst, kw, kc, &partial);
        sfcLocalSearchToLocalOptimum(repaired, rng);

        // Important: continue from the repaired solution even when it is not
        // better than best. This matches the attached implementation, which
        // destroys and repairs sol, while best is only a memory of the best seen.
        sol = repaired;

        if (sol.isFeasible() && sol.computeObjective() < best.computeObjective()) {
            best = sol;
            iter = 0;
        } else {
            ++iter;
        }
    }

    return best;
}

Solution LNSAAlgorithm::solve(const VSBPPCInstance& inst,
                              int kw,
                              int kc,
                              const LNSAParams& params,
                              std::mt19937& rng) {
    if (params.iterationLimit < 0) {
        throw std::invalid_argument(
            "LNSA iterationLimit must be nonnegative."
        );
    }

    if (params.destroyPercent < 0.0 || params.destroyPercent > 100.0) {
        throw std::invalid_argument(
            "LNSA destroyPercent must be in [0, 100]."
        );
    }

    switch (params.mode) {
        case LNSAMode::VANILLA:
            return solveVanilla(inst, kw, kc, params, rng);

        case LNSAMode::SFC:
            return solveSFC(inst, kw, kc, params, rng);
    }

    throw std::invalid_argument("Unknown LNSA mode.");
}
