#include "builder.hpp"

// ---- FEASIBILITY ----
static int smallestFeasibleType(const VSBPPCInstance& inst, int load) {
    for (int k = 0; k < static_cast<int>(inst.binTypes.size()); k++) {
        if (load <= inst.binTypes[k].capacity) return k;
    }

    return -1;
}

static bool feasible(const Solution& sol, int i, int b) {
    if (sol.confCount[i][b] != 0) return false;

    int newLoad = sol.binLoad[b] + sol.inst.weights[i];
    return smallestFeasibleType(sol.inst, newLoad) != -1;
}

// ---- COST DELTA (simple version) ----
static double deltaCost(const Solution& sol, int i, int b) {
    int oldType = sol.binType[b];
    int oldCost = (oldType == -1 ? 0 : sol.inst.binTypes[oldType].cost);

    int newLoad = sol.binLoad[b] + sol.inst.weights[i];
    int newType = smallestFeasibleType(sol.inst, newLoad);

    if (newType == -1) return std::numeric_limits<double>::infinity();

    int newCost = sol.inst.binTypes[newType].cost;
    return static_cast<double>(newCost - oldCost);
}

// ---- START FROM PARTIAL ----
static Solution startFromPartial(const VSBPPCInstance& inst,
                                 int kw,
                                 int kc,
                                 const Solution* partial,
                                 std::vector<char>& packed,
                                 int& packedCount,
                                 std::vector<int>& openBins) {
    Solution sol = (partial == nullptr)
                 ? Solution(inst, kw, kc)
                 : Solution(*partial);

    packed.assign(inst.N, 0);
    packedCount = 0;
    openBins.clear();

    for (int b = 0; b < sol.B; ++b) {
        if (!sol.itemsInBin[b].empty()) {
            openBins.push_back(b);

            for (int i : sol.itemsInBin[b]) {
                if (!packed[i]) {
                    packed[i] = 1;
                    ++packedCount;
                }
            }
        }
    }

    return sol;
}

// ---- FIRST EMPTY BIN ----
static int firstEmptyBin(const Solution& sol) {
    for (int b = 0; b < sol.B; ++b) {
        if (sol.itemsInBin[b].empty()) return b;
    }
    return -1;
}

// ---- ADD TO NEW EXISTING BIN ----
static void addToNewOrExistingBin(Solution& sol,
                                  int item,
                                  int& bin,
                                  std::vector<int>& openBins) {
    if (bin == -1) {
        bin = firstEmptyBin(sol);

        if (bin == -1) {
            throw std::runtime_error("No empty bin available.");
        }

        openBins.push_back(bin);
    }

    sol.addItem(item, bin);
}

// =====================================================
// 1. GREEDY COST (cost-based)
// =====================================================
Solution Builder::greedyCost(const VSBPPCInstance& inst,
                             int kw,
                             int kc,
                             const Solution* partial) {
    std::vector<char> packed;
    std::vector<int> openBins;
    int packedCount = 0;

    Solution sol = startFromPartial(inst, kw, kc, partial,
                                    packed, packedCount, openBins);

    while (packedCount < inst.N) {
        int bestItem = -1;
        int bestBin = -1;
        double bestVal = std::numeric_limits<double>::infinity();

        for (int i = 0; i < inst.N; ++i) {
            if (packed[i]) continue;

            for (int b : openBins) {
                if (!feasible(sol, i, b)) continue;

                double val = deltaCost(sol, i, b);

                if (val < bestVal) {
                    bestVal = val;
                    bestItem = i;
                    bestBin = b;
                }
            }
        }

        if (bestItem == -1) {
            for (int i = 0; i < inst.N; ++i) {
                if (!packed[i]) {
                    bestItem = i;
                    bestBin = -1;
                    break;
                }
            }
        }

        addToNewOrExistingBin(sol, bestItem, bestBin, openBins);
        packed[bestItem] = 1;
        ++packedCount;
    }

    return sol;
}

// =====================================================
// 2. DEGREE GREEDY (BPPC standard)
// =====================================================
Solution Builder::degreeGreedy(const VSBPPCInstance& inst,
                               int kw,
                               int kc,
                               const Solution* partial) {
    std::vector<char> packed;
    std::vector<int> openBins;
    int packedCount = 0;

    Solution sol = startFromPartial(inst, kw, kc, partial,
                                    packed, packedCount, openBins);

    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; ++i) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            if (inst.degree[a] != inst.degree[b]) {
                return inst.degree[a] > inst.degree[b];
            }
            return inst.weights[a] > inst.weights[b];
        });

    for (int i : order) {
        if (packed[i]) continue;

        int best = -1;
        int bestSlack = std::numeric_limits<int>::max();

        for (int b : openBins) {
            if (!feasible(sol, i, b)) continue;

            int newLoad = sol.binLoad[b] + inst.weights[i];
            int type = smallestFeasibleType(inst, newLoad);
            int slack = inst.binTypes[type].capacity - newLoad;

            if (slack < bestSlack) {
                bestSlack = slack;
                best = b;
            }
        }

        addToNewOrExistingBin(sol, i, best, openBins);
        packed[i] = 1;
        ++packedCount;
    }

    return sol;
}

// =====================================================
// 3. FIRST-FIT DECREASING (FFD)
// =====================================================
Solution Builder::firstFitDecreasing(const VSBPPCInstance& inst,
                                     int kw,
                                     int kc,
                                     const Solution* partial) {
    std::vector<char> packed;
    std::vector<int> openBins;
    int packedCount = 0;

    Solution sol = startFromPartial(inst, kw, kc, partial,
                                    packed, packedCount, openBins);

    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; ++i) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            return inst.weights[a] > inst.weights[b];
        });

    for (int i : order) {
        if (packed[i]) continue;

        int chosen = -1;

        for (int b : openBins) {
            if (feasible(sol, i, b)) {
                chosen = b;
                break;
            }
        }

        addToNewOrExistingBin(sol, i, chosen, openBins);
        packed[i] = 1;
        ++packedCount;
    }

    return sol;
}

// =====================================================
// 4. BEST-FIT DECREASING (BFD)
// =====================================================
Solution Builder::bestFitDecreasing(const VSBPPCInstance& inst,
                                    int kw,
                                    int kc,
                                    const Solution* partial) {
    std::vector<char> packed;
    std::vector<int> openBins;
    int packedCount = 0;

    Solution sol = startFromPartial(inst, kw, kc, partial,
                                    packed, packedCount, openBins);

    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; ++i) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            return inst.weights[a] > inst.weights[b];
        });

    for (int i : order) {
        if (packed[i]) continue;

        int best = -1;
        int bestSpace = std::numeric_limits<int>::max();

        for (int b : openBins) {
            if (!feasible(sol, i, b)) continue;

            int newLoad = sol.binLoad[b] + inst.weights[i];
            int type = smallestFeasibleType(inst, newLoad);
            int space = inst.binTypes[type].capacity - newLoad;

            if (space < bestSpace) {
                bestSpace = space;
                best = b;
            }
        }

        addToNewOrExistingBin(sol, i, best, openBins);
        packed[i] = 1;
        ++packedCount;
    }

    return sol;
}

// ---- FEASIBILITY WITH FIXED MAX TYPE ----
static bool feasibleWithType(const Solution& sol, int i, int b, int type) {
    if (sol.confCount[i][b] != 0) return false;

    int newLoad = sol.binLoad[b] + sol.inst.weights[i];
    return newLoad <= sol.inst.binTypes[type].capacity;
}

// ---- NEW BIN UNIT COST ----
static double newBinUnitCost(const VSBPPCInstance& inst, int i) {
    double best = std::numeric_limits<double>::infinity();

    for (const auto& bt : inst.binTypes) {
        if (inst.weights[i] <= bt.capacity) {
            best = std::min(best,
                static_cast<double>(bt.cost) / inst.weights[i]);
        }
    }

    return best;
}

// ---- EXISTING BIN UNIT COST ----
static double existingBinUnitCost(const Solution& sol, int i, int b) {
    if (sol.confCount[i][b] != 0) {
        return std::numeric_limits<double>::infinity();
    }

    int oldType = sol.binType[b];
    int oldCost = (oldType == -1 ? 0 : sol.inst.binTypes[oldType].cost);

    int newLoad = sol.binLoad[b] + sol.inst.weights[i];
    int newType = smallestFeasibleType(sol.inst, newLoad);

    if (newType == -1) {
        return std::numeric_limits<double>::infinity();
    }

    int newCost = sol.inst.binTypes[newType].cost;

    return static_cast<double>(newCost - oldCost) / sol.inst.weights[i];
}

// =====================================================
// 5. ALI EKICI CONSTRUCTIVE ALGORITHM -- SECTION 5.1
// =====================================================
Solution Builder::aliEkiciConstructive(const VSBPPCInstance& inst,
                                               int kw,
                                               int kc,
                                               const Solution* partial) {
    Solution sol = (partial == nullptr)
                 ? Solution(inst, kw, kc)
                 : Solution(*partial);

    std::vector<char> packed(inst.N, 0);
    int packedCount = 0;

    // Open bins are exactly non-empty bins from the partial solution.
    // This avoids treating empty holes as available bins during repair.
    std::vector<int> openBins;
    for (int b = 0; b < sol.B; ++b) {
        if (!sol.itemsInBin[b].empty()) {
            openBins.push_back(b);

            for (int i : sol.itemsInBin[b]) {
                if (!packed[i]) {
                    packed[i] = 1;
                    ++packedCount;
                }
            }
        }
    }

    auto smallestType = [&](int load) -> int {
        for (int k = 0; k < static_cast<int>(inst.binTypes.size()); ++k) {
            if (load <= inst.binTypes[k].capacity) return k;
        }
        return -1;
    };

    auto canPackInExistingBin = [&](int item, int bin) -> bool {
        if (sol.confCount[item][bin] != 0) return false;

        int newLoad = sol.binLoad[bin] + inst.weights[item];
        return smallestType(newLoad) != -1;
    };

    auto existingBinUnitCost = [&](int item, int bin) -> double {
        if (!canPackInExistingBin(item, bin)) {
            return std::numeric_limits<double>::infinity();
        }

        int oldType = sol.binType[bin];
        int oldCost = oldType < 0 ? 0 : inst.binTypes[oldType].cost;

        int newLoad = sol.binLoad[bin] + inst.weights[item];
        int newType = smallestType(newLoad);

        int newCost = inst.binTypes[newType].cost;

        return static_cast<double>(newCost - oldCost) /
               static_cast<double>(inst.weights[item]);
    };

    auto newBinUnitCost = [&](int item) -> double {
        int type = smallestType(inst.weights[item]);

        if (type == -1) {
            return std::numeric_limits<double>::infinity();
        }

        return static_cast<double>(inst.binTypes[type].cost) /
               static_cast<double>(inst.weights[item]);
    };

    auto firstEmptyBin = [&]() -> int {
        for (int b = 0; b < sol.B; ++b) {
            if (sol.itemsInBin[b].empty()) return b;
        }
        return -1;
    };

    while (packedCount < inst.N) {
        int bestItem = -1;
        int bestBin = -1;
        bool bestUsesNewBin = false;
        double bestUnitCost = std::numeric_limits<double>::infinity();

        for (int i = 0; i < inst.N; ++i) {
            if (packed[i]) continue;

            for (int b : openBins) {
                double unitCost = existingBinUnitCost(i, b);

                if (unitCost < bestUnitCost) {
                    bestUnitCost = unitCost;
                    bestItem = i;
                    bestBin = b;
                    bestUsesNewBin = false;
                }
            }

            double unitCost = newBinUnitCost(i);

            if (unitCost < bestUnitCost) {
                bestUnitCost = unitCost;
                bestItem = i;
                bestBin = -1;
                bestUsesNewBin = true;
            }
        }

        if (bestItem == -1 || !std::isfinite(bestUnitCost)) {
            throw std::runtime_error(
                "aliEkiciConstructiveFaithful could not construct a feasible solution."
            );
        }

        if (bestUsesNewBin) {
            bestBin = firstEmptyBin();

            if (bestBin == -1) {
                throw std::runtime_error(
                    "aliEkiciConstructiveFaithful ran out of available bin slots."
                );
            }

            openBins.push_back(bestBin);
        }

        sol.addItem(bestItem, bestBin);
        packed[bestItem] = 1;
        ++packedCount;
    }

    return sol;
}

// ---- SURROGATE ORDER FOR FFD(alpha) / BFD(alpha) ----
static std::vector<int> surrogateOrder(const VSBPPCInstance& inst,
                                       double alpha) {
    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; i++) order[i] = i;

    double avgWeight = 0.0;
    double avgDegree = 0.0;

    for (int i = 0; i < inst.N; i++) {
        avgWeight += inst.weights[i];
        avgDegree += inst.degree[i];
    }

    avgWeight /= std::max(1, inst.N);
    avgDegree /= std::max(1, inst.N);

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            double sa =
                alpha * (inst.weights[a] / avgWeight) +
                (1.0 - alpha) *
                    (avgDegree > 0.0 ? inst.degree[a] / avgDegree : 0.0);

            double sb =
                alpha * (inst.weights[b] / avgWeight) +
                (1.0 - alpha) *
                    (avgDegree > 0.0 ? inst.degree[b] / avgDegree : 0.0);

            if (sa != sb) return sa > sb;
            return inst.weights[a] > inst.weights[b];
        });

    return order;
}

// ---- SHRINK BIN TYPES AFTER FIXED-TYPE PACKING ----
static void shrinkBins(Solution& sol) {
    for (int b = 0; b < sol.B; b++) {
        if (sol.itemsInBin[b].empty()) {
            sol.binType[b] = -1;
            continue;
        }

        int type = smallestFeasibleType(sol.inst, sol.binLoad[b]);

        sol.binType[b] = type;
    }
}

// ---- IFFD/IBFD CORE ----
static Solution iterativeFFD_BFD(const VSBPPCInstance& inst,
                                 int kw,
                                 int kc,
                                 double alpha,
                                 bool bestFit,
                                 const Solution* partial) {
    std::vector<int> order = surrogateOrder(inst, alpha);

    std::vector<char> basePacked;
    std::vector<int> baseOpenBins;
    int basePackedCount = 0;

    Solution base = startFromPartial(inst, kw, kc, partial,
                                     basePacked,
                                     basePackedCount,
                                     baseOpenBins);

    std::unique_ptr<Solution> best = nullptr;

    for (int maxType = static_cast<int>(inst.binTypes.size()) - 1;
         maxType >= 0;
         --maxType) {

        Solution sol = base;
        std::vector<char> packed = basePacked;
        std::vector<int> openBins = baseOpenBins;

        bool failed = false;

        for (int i : order) {
            if (packed[i]) continue;

            int chosenBin = -1;

            if (!bestFit) {
                for (int b : openBins) {
                    if (feasibleWithType(sol, i, b, maxType)) {
                        chosenBin = b;
                        break;
                    }
                }
            } else {
                int bestSpace = std::numeric_limits<int>::max();

                for (int b : openBins) {
                    if (!feasibleWithType(sol, i, b, maxType)) continue;

                    int newLoad = sol.binLoad[b] + inst.weights[i];
                    int space = inst.binTypes[maxType].capacity - newLoad;

                    if (space < bestSpace) {
                        bestSpace = space;
                        chosenBin = b;
                    }
                }
            }

            if (chosenBin == -1) {
                if (inst.weights[i] > inst.binTypes[maxType].capacity) {
                    failed = true;
                    break;
                }

                chosenBin = firstEmptyBin(sol);

                if (chosenBin == -1) {
                    failed = true;
                    break;
                }

                openBins.push_back(chosenBin);
            }

            sol.addItem(i, chosenBin);
            packed[i] = 1;
        }

        if (failed) continue;

        shrinkBins(sol);

        if (!best || sol.computeObjective() < best->computeObjective()) {
            best = std::make_unique<Solution>(sol);
        }
    }

    if (!best) {
        throw std::runtime_error("IFFD/IBFD failed to construct a solution.");
    }

    return *best;
}

// =====================================================
// 6. ITERATIVE FIRST-FIT DECREASING IFFD(alpha)
// =====================================================
Solution Builder::IFFD(const VSBPPCInstance& inst,
                       int kw,
                       int kc,
                       double alpha,
                       const Solution* partial) {
    return iterativeFFD_BFD(inst, kw, kc, alpha, false, partial);
}

// =====================================================
// 7. ITERATIVE BEST-FIT DECREASING IBFD(alpha)
// =====================================================
Solution Builder::IBFD(const VSBPPCInstance& inst,
                       int kw,
                       int kc,
                       double alpha,
                       const Solution* partial) {
    return iterativeFFD_BFD(inst, kw, kc, alpha, true, partial);
}

// =====================================================
// HC1 / HC2 helpers
// =====================================================

static Solution startPartialConstructive(const VSBPPCInstance& inst,
                                         int kw,
                                         int kc,
                                         const Solution* partial,
                                         std::vector<char>& packed,
                                         int& packedCount,
                                         std::vector<int>& openBins) {
    Solution sol = partial == nullptr
                 ? Solution(inst, kw, kc)
                 : Solution(*partial);

    packed.assign(inst.N, 0);
    packedCount = 0;
    openBins.clear();

    for (int b = 0; b < sol.B; ++b) {
        if (sol.itemsInBin[b].empty()) continue;

        openBins.push_back(b);

        for (int item : sol.itemsInBin[b]) {
            if (!packed[item]) {
                packed[item] = 1;
                ++packedCount;
            }
        }
    }

    return sol;
}

static int firstEmptyBinHC(const Solution& sol) {
    for (int b = 0; b < sol.B; ++b) {
        if (sol.itemsInBin[b].empty()) return b;
    }
    return -1;
}

static double hc1ExistingCost(const Solution& sol, int item, int bin) {
    static constexpr double LAMBDA = 1e-1;

    if (sol.confCount[item][bin] != 0) {
        return std::numeric_limits<double>::infinity();
    }

    const auto& inst = sol.inst;

    int oldType = sol.binType[bin];
    int oldCost = oldType < 0 ? 0 : inst.binTypes[oldType].cost;

    int newLoad = sol.binLoad[bin] + inst.weights[item];
    int newType = smallestFeasibleType(inst, newLoad);

    if (newType == -1) {
        return std::numeric_limits<double>::infinity();
    }

    int newCost = inst.binTypes[newType].cost;
    int delta = newCost - oldCost;

    // HC1 tie-break: if the bin type does not change, prefer tighter fill.
    if (delta == 0 && oldType >= 0 && newType == oldType) {
        return LAMBDA *
               (1.0 - static_cast<double>(newLoad) /
                      static_cast<double>(inst.binTypes[oldType].capacity));
    }

    return static_cast<double>(delta) /
           static_cast<double>(inst.weights[item]);
}

static double hc1NewBinCost(const VSBPPCInstance& inst, int item) {
    int type = smallestFeasibleType(inst, inst.weights[item]);

    if (type == -1) {
        return std::numeric_limits<double>::infinity();
    }

    return static_cast<double>(inst.binTypes[type].cost) /
           static_cast<double>(inst.weights[item]);
}

static double hc2ExistingCost(const Solution& sol, int item, int bin) {
    if (sol.confCount[item][bin] != 0) {
        return std::numeric_limits<double>::infinity();
    }

    const auto& inst = sol.inst;

    int newLoad = sol.binLoad[bin] + inst.weights[item];
    int newType = smallestFeasibleType(inst, newLoad);

    if (newType == -1) {
        return std::numeric_limits<double>::infinity();
    }

    return static_cast<double>(inst.binTypes[newType].cost) /
           static_cast<double>(newLoad);
}

static double hc2NewBinCost(const VSBPPCInstance& inst, int item) {
    int type = smallestFeasibleType(inst, inst.weights[item]);

    if (type == -1) {
        return std::numeric_limits<double>::infinity();
    }

    return static_cast<double>(inst.binTypes[type].cost) /
           static_cast<double>(inst.weights[item]);
}

static Solution HC_constructiveCore(const VSBPPCInstance& inst,
                                    int kw,
                                    int kc,
                                    const Solution* partial,
                                    bool useHC2) {
    std::vector<char> packed;
    std::vector<int> openBins;
    int packedCount = 0;

    Solution sol = startPartialConstructive(
        inst, kw, kc, partial, packed, packedCount, openBins
    );

    while (packedCount < inst.N) {
        int bestItem = -1;
        int bestBin = -1;
        bool useNewBin = false;
        double bestCost = std::numeric_limits<double>::infinity();

        for (int item = 0; item < inst.N; ++item) {
            if (packed[item]) continue;

            for (int bin : openBins) {
                double cost = useHC2
                            ? hc2ExistingCost(sol, item, bin)
                            : hc1ExistingCost(sol, item, bin);

                if (cost < bestCost) {
                    bestCost = cost;
                    bestItem = item;
                    bestBin = bin;
                    useNewBin = false;
                }
            }

            double newCost = useHC2
                           ? hc2NewBinCost(inst, item)
                           : hc1NewBinCost(inst, item);

            if (newCost < bestCost) {
                bestCost = newCost;
                bestItem = item;
                bestBin = -1;
                useNewBin = true;
            }
        }

        if (bestItem == -1 || !std::isfinite(bestCost)) {
            throw std::runtime_error(
                useHC2
                ? "HC2 failed to construct a feasible solution."
                : "HC1 failed to construct a feasible solution."
            );
        }

        if (useNewBin) {
            bestBin = firstEmptyBinHC(sol);

            if (bestBin == -1) {
                throw std::runtime_error("No empty bin available.");
            }

            openBins.push_back(bestBin);
        }

        sol.addItem(bestItem, bestBin);
        packed[bestItem] = 1;
        ++packedCount;
    }

    return sol;
}

// =====================================================
// HC1: Ekici constructive with zero-delta fill tie-break
// =====================================================
Solution Builder::HC1(const VSBPPCInstance& inst,
                      int kw,
                      int kc,
                      const Solution* partial) {
    return HC_constructiveCore(inst, kw, kc, partial, false);
}

// =====================================================
// HC2: Best-Fit-style constructive
// =====================================================
Solution Builder::HC2(const VSBPPCInstance& inst,
                      int kw,
                      int kc,
                      const Solution* partial) {
    return HC_constructiveCore(inst, kw, kc, partial, true);
}