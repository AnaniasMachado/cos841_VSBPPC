#include "builder.hpp"
#include <algorithm>
#include <limits>

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

// =====================================================
// 1. GREEDY COST (cost-based)
// =====================================================
Solution Builder::greedyCost(const VSBPPCInstance& inst,
                             double kw, double kc) {
    Solution sol(inst, kw, kc);

    int nextBin = 0;

    for (int i = 0; i < inst.N; i++) {
        int best = -1;
        double bestVal = std::numeric_limits<double>::infinity();

        for (int b = 0; b < nextBin; b++) {
            if (!feasible(sol, i, b)) continue;

            double val = deltaCost(sol, i, b);

            if (val < bestVal) {
                bestVal = val;
                best = b;
            }
        }

        if (best == -1) best = nextBin++;

        sol.addItem(i, best);
    }

    return sol;
}

// =====================================================
// 2. FIRST-FIT DECREASING (FFD)
// =====================================================
Solution Builder::firstFitDecreasing(const VSBPPCInstance& inst,
                                     double kw, double kc) {
    Solution sol(inst, kw, kc);

    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; i++) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            return inst.weights[a] > inst.weights[b];
        });

    int nextBin = 0;

    for (int i : order) {
        bool placed = false;

        for (int b = 0; b < nextBin; b++) {
            if (feasible(sol, i, b)) {
                sol.addItem(i, b);
                placed = true;
                break;
            }
        }

        if (!placed) {
            sol.addItem(i, nextBin++);
        }
    }

    return sol;
}

// =====================================================
// 3. DEGREE GREEDY (BPPC standard)
// =====================================================
Solution Builder::degreeGreedy(const VSBPPCInstance& inst,
                               double kw, double kc) {
    Solution sol(inst, kw, kc);

    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; i++) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            if (inst.degree[a] != inst.degree[b]) {
                return inst.degree[a] > inst.degree[b];
            }

            return inst.weights[a] > inst.weights[b];
        });

    int nextBin = 0;

    for (int i : order) {
        int best = -1;
        int bestSlack = std::numeric_limits<int>::max();

        for (int b = 0; b < nextBin; b++) {
            if (!feasible(sol, i, b)) continue;

            int newLoad = sol.binLoad[b] + inst.weights[i];
            int type = smallestFeasibleType(inst, newLoad);
            int slack = inst.binTypes[type].capacity - newLoad;

            if (slack < bestSlack) {
                bestSlack = slack;
                best = b;
            }
        }

        if (best == -1) best = nextBin++;

        sol.addItem(i, best);
    }

    return sol;
}

// =====================================================
// 4. BEST-FIT DECREASING (BFD)
// =====================================================
Solution Builder::bestFitDecreasing(const VSBPPCInstance& inst,
                                    double kw, double kc) {
    Solution sol(inst, kw, kc);

    std::vector<int> order(inst.N);
    for (int i = 0; i < inst.N; i++) order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](int a, int b) {
            return inst.weights[a] > inst.weights[b];
        });

    int nextBin = 0;

    for (int i : order) {
        int best = -1;
        int bestSpace = std::numeric_limits<int>::max();

        for (int b = 0; b < nextBin; b++) {
            if (!feasible(sol, i, b)) continue;

            int newLoad = sol.binLoad[b] + inst.weights[i];
            int type = smallestFeasibleType(inst, newLoad);
            int space = inst.binTypes[type].capacity - newLoad;

            if (space < bestSpace) {
                bestSpace = space;
                best = b;
            }
        }

        if (best == -1) best = nextBin++;

        sol.addItem(i, best);
    }

    return sol;
}