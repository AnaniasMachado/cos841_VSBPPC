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
    Solution compact(src.inst, src.kw, src.kc, true);

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

    int bestItem = -1;
    int bestFrom = -1;
    int bestTo = -1;
    int bestDelta = 0;

    for (int from : bins) {
        const std::vector<int> items = sol.itemsInBin[from];

        for (int item : items) {
            for (int to : bins) {
                if (!feasibleTransfer(sol, item, from, to)) continue;

                int delta = objectiveDeltaTransfer(sol, item, from, to);

                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestItem = item;
                    bestFrom = from;
                    bestTo = to;
                }
            }
        }
    }

    if (bestItem == -1) {
        return false;
    }

    sol.moveItem(bestItem, bestFrom, bestTo);

    if (sol.itemsInBin[bestFrom].empty()) {
        sol = compactSolution(sol);
    }

    return true;
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

    int bestItem1 = -1;
    int bestItem2 = -1;
    int bestDelta = 0;

    for (std::size_t a = 0; a < bins.size(); ++a) {
        for (std::size_t b = a + 1; b < bins.size(); ++b) {
            int bin1 = bins[a];
            int bin2 = bins[b];

            const std::vector<int> items1 = sol.itemsInBin[bin1];
            const std::vector<int> items2 = sol.itemsInBin[bin2];

            for (int item1 : items1) {
                for (int item2 : items2) {
                    if (!feasibleSwap(sol, item1, item2, bin1, bin2)) {
                        continue;
                    }

                    int delta = objectiveDeltaSwap(sol, item1, item2, bin1, bin2);

                    if (delta < bestDelta) {
                        bestDelta = delta;
                        bestItem1 = item1;
                        bestItem2 = item2;
                    }
                }
            }
        }
    }

    if (bestItem1 == -1) {
        return false;
    }

    sol.swapItems(bestItem1, bestItem2);
    return true;
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

Solution LNSAAlgorithm::destroyBest(const Solution& best,
                                    double destroyPercent,
                                    std::mt19937& rng) {
    const std::vector<int> bins = usedBins(best);

    if (bins.empty()) {
        return best;
    }

    int removeCount =
        static_cast<int>(std::ceil((destroyPercent / 100.0) * bins.size()));

    removeCount = std::max(1, removeCount);
    removeCount = std::min(removeCount, static_cast<int>(bins.size()));

    std::vector<int> shuffled = bins;
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    std::vector<char> remove(best.B, 0);

    for (int r = 0; r < removeCount; ++r) {
        remove[shuffled[r]] = 1;
    }

    Solution partial(best.inst, best.kw, best.kc, true);

    int nextBin = 0;

    for (int bin : bins) {
        if (remove[bin]) continue;

        for (int item : best.itemsInBin[bin]) {
            partial.addItem(item, nextBin);
        }

        ++nextBin;
    }

    return partial;
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

    Solution best = Builder::aliEkiciConstructive(inst, kw, kc, nullptr);
    localSearchToLocalOptimum(best);

    for (int it = 0; it < params.iterationLimit; ++it) {
        if (params.lowerBound >= 0 &&
            best.computeObjective() <= params.lowerBound) {
            break;
        }

        Solution partial = destroyBest(best, params.destroyPercent, rng);
        Solution repaired = Builder::aliEkiciConstructive(inst, kw, kc, &partial);
        localSearchToLocalOptimum(repaired);

        if (repaired.isFeasible() &&
            repaired.computeObjective() < best.computeObjective()) {
            best = repaired;
        }
    }

    return best;
}