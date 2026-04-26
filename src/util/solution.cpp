#include "solution.hpp"
#include <algorithm>

// ---- Constructor ----
Solution::Solution(const VSBPPCInstance& instance,
                   int kw_, int kc_)
    : inst(instance), N(instance.N), B(instance.N),
      kw(kw_), kc(kc_) {
    itemBin.assign(N, -1);
    itemsInBin.assign(B, {});
    binLoad.assign(B, 0);
    binType.assign(B, -1);

    confCount.assign(N, std::vector<int>(B, 0));
    binConflicts.assign(B, 0);

    totalBinCost = 0;
    totalConflicts = 0;
    totalExcess = 0;
}

static int smallestFeasibleType(const VSBPPCInstance& inst, int load) {
    for (int k = 0; k < static_cast<int>(inst.binTypes.size()); k++) {
        if (load <= inst.binTypes[k].capacity) return k;
    }

    return -1;
}

static int selectedType(const VSBPPCInstance& inst, int load) {
    if (load <= 0) return -1;

    int k = smallestFeasibleType(inst, load);
    if (k != -1) return k;

    return static_cast<int>(inst.binTypes.size()) - 1;
}

static int binCapacity(const VSBPPCInstance& inst, int type) {
    if (type == -1) return 0;
    return inst.binTypes[type].capacity;
}

static int binCost(const VSBPPCInstance& inst, int type) {
    if (type == -1) return 0;
    return inst.binTypes[type].cost;
}

// ---- Helper ----
void Solution::updateBadBin(int b) {
    bool bad = false;

    if (!itemsInBin[b].empty()) {
        int cap = binCapacity(inst, binType[b]);
        bad = (binLoad[b] > cap) || (binConflicts[b] > 0);
    }

    auto it = std::find(badBins.begin(), badBins.end(), b);

    if (bad && it == badBins.end()) {
        badBins.push_back(b);
    } else if (!bad && it != badBins.end()) {
        badBins.erase(it);
    }
}

// ---- ADD ITEM ----
void Solution::addItem(int i, int b) {
    // conflicts
    int conflictsAdded = confCount[i][b];

    for (int v : inst.neighbors[i]) {
        confCount[v][b]++;
    }

    binConflicts[b] += conflictsAdded;
    totalConflicts += conflictsAdded;

    // assign item
    itemBin[i] = b;
    itemsInBin[b].push_back(i);

    // load, type, excess, bin cost
    int oldLoad = binLoad[b];
    int oldType = binType[b];
    int oldCap = binCapacity(inst, oldType);
    int oldCost = binCost(inst, oldType);

    int newLoad = oldLoad + inst.weights[i];
    int newType = selectedType(inst, newLoad);
    int newCap = binCapacity(inst, newType);
    int newCost = binCost(inst, newType);

    totalExcess -= std::max(0, oldLoad - oldCap);
    totalExcess += std::max(0, newLoad - newCap);

    totalBinCost += newCost - oldCost;

    binLoad[b] = newLoad;
    binType[b] = newType;

    updateBadBin(b);
}

// ---- REMOVE ITEM ----
void Solution::removeItem(int i) {
    int b = itemBin[i];

    // conflicts
    int conflictsRemoved = confCount[i][b];

    for (int v : inst.neighbors[i]) {
        confCount[v][b]--;
    }

    binConflicts[b] -= conflictsRemoved;
    totalConflicts -= conflictsRemoved;

    // load, type, excess, bin cost
    int oldLoad = binLoad[b];
    int oldType = binType[b];
    int oldCap = binCapacity(inst, oldType);
    int oldCost = binCost(inst, oldType);

    int newLoad = oldLoad - inst.weights[i];
    int newType = selectedType(inst, newLoad);

    if (newLoad <= 0) {
        newType = -1;
    }

    int newCap = binCapacity(inst, newType);
    int newCost = binCost(inst, newType);

    totalExcess -= std::max(0, oldLoad - oldCap);
    totalExcess += std::max(0, newLoad - newCap);

    totalBinCost += newCost - oldCost;

    binLoad[b] = newLoad;
    binType[b] = newType;

    // remove item
    auto& vec = itemsInBin[b];
    vec.erase(std::find(vec.begin(), vec.end(), i));

    itemBin[i] = -1;

    updateBadBin(b);
}

// ---- MOVE ----
void Solution::moveItem(int i, int from, int to) {
    (void)from;
    removeItem(i);
    addItem(i, to);
}

// ---- SWAP ----
void Solution::swapItems(int i, int j) {
    int b1 = itemBin[i];
    int b2 = itemBin[j];

    removeItem(i);
    removeItem(j);

    addItem(i, b2);
    addItem(j, b1);
}

// ---- IS FEASIBLE ----
bool Solution::isFeasible() const {
    return badBins.empty();
}

// ---- OBJECTIVE ----
int Solution::computeObjective() const {
    return totalBinCost + kw * totalExcess + kc * totalConflicts;
}

// ---- DELTA SWAP SUBSETS ----
int deltaSwapSubsets(
    const Solution& sol,
    const std::vector<int>& A, // from b1 -> b2
    const std::vector<int>& B, // from b2 -> b1
    int b1, int b2) {
    const auto& inst = sol.inst;

    if (b1 == b2) return 0;

    // ---- LOAD DELTA ----
    int wA = 0;
    int wB = 0;

    for (int i : A) wA += inst.weights[i];
    for (int j : B) wB += inst.weights[j];

    int oldL1 = sol.binLoad[b1];
    int oldL2 = sol.binLoad[b2];

    int newL1 = oldL1 - wA + wB;
    int newL2 = oldL2 - wB + wA;

    int oldT1 = sol.binType[b1];
    int oldT2 = sol.binType[b2];

    int newT1 = selectedType(inst, newL1);
    int newT2 = selectedType(inst, newL2);

    if (newL1 <= 0) newT1 = -1;
    if (newL2 <= 0) newT2 = -1;

    int oldC1 = binCapacity(inst, oldT1);
    int oldC2 = binCapacity(inst, oldT2);
    int newC1 = binCapacity(inst, newT1);
    int newC2 = binCapacity(inst, newT2);

    int oldExcess =
        std::max(0, oldL1 - oldC1) +
        std::max(0, oldL2 - oldC2);

    int newExcess =
        std::max(0, newL1 - newC1) +
        std::max(0, newL2 - newC2);

    int deltaExcess = newExcess - oldExcess;

    int deltaCost =
        (binCost(inst, newT1) + binCost(inst, newT2)) -
        (binCost(inst, oldT1) + binCost(inst, oldT2));

    // ---- CONFLICT DELTA ----
    int deltaConf = 0;

    std::vector<char> inA(sol.N, 0);
    std::vector<char> inB(sol.N, 0);

    for (int i : A) inA[i] = 1;
    for (int j : B) inB[j] = 1;

    auto hasConflict = [&](int i, int j) -> bool {
        int word = j >> 6;
        int bit = j & 63;
        return (inst.conflicts[i][word] & (1ULL << bit)) != 0;
    };

    // Remove conflicts involving A in b1, excluding conflicts internal to A.
    for (int i : A) {
        deltaConf -= sol.confCount[i][b1];

        for (int j : A) {
            if (i < j && hasConflict(i, j)) {
                deltaConf += 1;
            }
        }
    }

    // Remove conflicts involving B in b2, excluding conflicts internal to B.
    for (int i : B) {
        deltaConf -= sol.confCount[i][b2];

        for (int j : B) {
            if (i < j && hasConflict(i, j)) {
                deltaConf += 1;
            }
        }
    }

    // Add conflicts caused by A entering b2.
    for (int i : A) {
        for (int other : sol.itemsInBin[b2]) {
            if (inB[other]) continue;

            if (hasConflict(i, other)) {
                deltaConf += 1;
            }
        }

        for (int j : A) {
            if (i < j && hasConflict(i, j)) {
                deltaConf += 1;
            }
        }
    }

    // Add conflicts caused by B entering b1.
    for (int i : B) {
        for (int other : sol.itemsInBin[b1]) {
            if (inA[other]) continue;

            if (hasConflict(i, other)) {
                deltaConf += 1;
            }
        }

        for (int j : B) {
            if (i < j && hasConflict(i, j)) {
                deltaConf += 1;
            }
        }
    }

    return deltaCost + sol.kw * deltaExcess + sol.kc * deltaConf;
}

// COPY OPERATOR
Solution& Solution::operator=(const Solution& other) {
    if (this == &other) return *this;

    // This assignment is only valid for solutions of the same instance.
    if (&inst != &other.inst) {
        throw std::runtime_error(
            "Cannot assign Solution objects from different VSBPPC instances."
        );
    }

    N = other.N;
    B = other.B;

    kw = other.kw;
    kc = other.kc;

    itemBin = other.itemBin;
    itemsInBin = other.itemsInBin;
    binLoad = other.binLoad;
    binType = other.binType;

    confCount = other.confCount;
    binConflicts = other.binConflicts;

    totalBinCost = other.totalBinCost;
    totalConflicts = other.totalConflicts;
    totalExcess = other.totalExcess;

    badBins = other.badBins;

    return *this;
}