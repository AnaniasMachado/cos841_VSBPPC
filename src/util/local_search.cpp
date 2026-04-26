#include "local_search.hpp"

// ---- Constructor ----
LocalSearch::LocalSearch(Solution& sol,
                         ImprovementType improvementType_,
                         std::mt19937& rng_)
    : solution(&sol),
      improvementType(improvementType_),
      rng(rng_) {
}

// ---- Apply swap subsets ----
void LocalSearch::applySwapSubsets(const std::vector<int>& A,
                                   const std::vector<int>& B,
                                   int b1, int b2) {
    if (A.size() == 1 && B.empty()) {
        solution->moveItem(A[0], b1, b2);
        return;
    }

    if (A.size() == 1 && B.size() == 1) {
        solution->swapItems(A[0], B[0]);
        return;
    }

    for (int i : A) {
        solution->removeItem(i);
    }

    for (int j : B) {
        solution->removeItem(j);
    }

    for (int i : A) {
        solution->addItem(i, b2);
    }

    for (int j : B) {
        solution->addItem(j, b1);
    }
}

// ---- Relocate ----
bool LocalSearch::relocate() {
    std::vector<int> badBins = solution->badBins;
    if (badBins.empty()) return false;

    if (improvementType == ImprovementType::FI) {
        std::shuffle(badBins.begin(), badBins.end(), rng);

        for (int b1 : badBins) {
            std::vector<int> items = solution->itemsInBin[b1];
            std::shuffle(items.begin(), items.end(), rng);

            std::vector<int> bins(solution->B);
            for (int b = 0; b < solution->B; b++) bins[b] = b;
            std::shuffle(bins.begin(), bins.end(), rng);

            for (int i : items) {
                for (int b2 : bins) {
                    if (b2 == b1) continue;

                    std::vector<int> A = {i};
                    std::vector<int> B;

                    int delta = deltaSwapSubsets(*solution, A, B, b1, b2);

                    if (delta < 0) {
                        applySwapSubsets(A, B, b1, b2);
                        return true;
                    }
                }
            }
        }

        return false;
    }

    int bestDelta = 0;
    int bestB1 = -1, bestB2 = -1, bestI = -1;

    for (int b1 : badBins) {
        for (int i : solution->itemsInBin[b1]) {
            for (int b2 = 0; b2 < solution->B; b2++) {
                if (b2 == b1) continue;

                std::vector<int> A = {i};
                std::vector<int> B;

                int delta = deltaSwapSubsets(*solution, A, B, b1, b2);

                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestB1 = b1;
                    bestB2 = b2;
                    bestI = i;
                }
            }
        }
    }

    if (bestB1 != -1) {
        applySwapSubsets({bestI}, {}, bestB1, bestB2);
        return true;
    }

    return false;
}

// ---- Exchange ----
bool LocalSearch::exchange() {
    std::vector<int> badBins = solution->badBins;
    if (badBins.empty()) return false;

    if (improvementType == ImprovementType::FI) {
        std::shuffle(badBins.begin(), badBins.end(), rng);

        for (int b1 : badBins) {
            std::vector<int> items1 = solution->itemsInBin[b1];
            std::shuffle(items1.begin(), items1.end(), rng);

            std::vector<int> bins(solution->B);
            for (int b = 0; b < solution->B; b++) bins[b] = b;
            std::shuffle(bins.begin(), bins.end(), rng);

            for (int i : items1) {
                for (int b2 : bins) {
                    if (b2 == b1 || solution->itemsInBin[b2].empty()) continue;

                    std::vector<int> items2 = solution->itemsInBin[b2];
                    std::shuffle(items2.begin(), items2.end(), rng);

                    for (int j : items2) {
                        std::vector<int> A = {i};
                        std::vector<int> B = {j};

                        int delta = deltaSwapSubsets(*solution, A, B, b1, b2);

                        if (delta < 0) {
                            applySwapSubsets(A, B, b1, b2);
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    int bestDelta = 0;
    int bestB1 = -1, bestB2 = -1, bestI = -1, bestJ = -1;

    for (int b1 : badBins) {
        for (int i : solution->itemsInBin[b1]) {
            for (int b2 = 0; b2 < solution->B; b2++) {
                if (b2 == b1 || solution->itemsInBin[b2].empty()) continue;

                for (int j : solution->itemsInBin[b2]) {
                    std::vector<int> A = {i};
                    std::vector<int> B = {j};

                    int delta = deltaSwapSubsets(*solution, A, B, b1, b2);

                    if (delta < bestDelta) {
                        bestDelta = delta;
                        bestB1 = b1;
                        bestB2 = b2;
                        bestI = i;
                        bestJ = j;
                    }
                }
            }
        }
    }

    if (bestB1 != -1) {
        applySwapSubsets({bestI}, {bestJ}, bestB1, bestB2);
        return true;
    }

    return false;
}

// ---- Exchange 2-1 ----
bool LocalSearch::exchange21() {
    std::vector<int> badBins = solution->badBins;
    if (badBins.empty()) return false;

    if (improvementType == ImprovementType::FI) {
        std::shuffle(badBins.begin(), badBins.end(), rng);

        for (int b1 : badBins) {
            std::vector<int> items1 = solution->itemsInBin[b1];
            if (items1.size() < 2) continue;

            std::shuffle(items1.begin(), items1.end(), rng);

            std::vector<int> bins(solution->B);
            for (int b = 0; b < solution->B; b++) bins[b] = b;
            std::shuffle(bins.begin(), bins.end(), rng);

            for (std::size_t p = 0; p + 1 < items1.size(); p++) {
                for (std::size_t q = p + 1; q < items1.size(); q++) {
                    int i = items1[p];
                    int j = items1[q];

                    for (int b2 : bins) {
                        if (b2 == b1 || solution->itemsInBin[b2].empty()) continue;

                        std::vector<int> items2 = solution->itemsInBin[b2];
                        std::shuffle(items2.begin(), items2.end(), rng);

                        for (int k : items2) {
                            std::vector<int> A = {i, j};
                            std::vector<int> B = {k};

                            int delta = deltaSwapSubsets(*solution, A, B, b1, b2);

                            if (delta < 0) {
                                applySwapSubsets(A, B, b1, b2);
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    int bestDelta = 0;
    int bestB1 = -1, bestB2 = -1;
    int bestI = -1, bestJ = -1, bestK = -1;

    for (int b1 : badBins) {
        const std::vector<int>& items1 = solution->itemsInBin[b1];
        if (items1.size() < 2) continue;

        for (std::size_t p = 0; p + 1 < items1.size(); p++) {
            for (std::size_t q = p + 1; q < items1.size(); q++) {
                int i = items1[p];
                int j = items1[q];

                for (int b2 = 0; b2 < solution->B; b2++) {
                    if (b2 == b1 || solution->itemsInBin[b2].empty()) continue;

                    for (int k : solution->itemsInBin[b2]) {
                        std::vector<int> A = {i, j};
                        std::vector<int> B = {k};

                        int delta = deltaSwapSubsets(*solution, A, B, b1, b2);

                        if (delta < bestDelta) {
                            bestDelta = delta;
                            bestB1 = b1;
                            bestB2 = b2;
                            bestI = i;
                            bestJ = j;
                            bestK = k;
                        }
                    }
                }
            }
        }
    }

    if (bestB1 != -1) {
        applySwapSubsets({bestI, bestJ}, {bestK}, bestB1, bestB2);
        return true;
    }

    return false;
}

// ---- Classic ----
bool LocalSearch::classic() {
    if (solution->isFeasible()) {
        return false;
    }

    bool overallImproved = false;

    std::vector<int> order = {0, 1, 2};
    std::shuffle(order.begin(), order.end(), rng);

    while (true) {
        bool improvedThisLoop = false;

        for (int op : order) {
            bool moved = false;

            switch (op) {
                case 0:
                    moved = relocate();
                    break;
                case 1:
                    moved = exchange();
                    break;
                case 2:
                    moved = exchange21();
                    break;
            }

            if (moved) {
                improvedThisLoop = true;
                overallImproved = true;
            }

            if (solution->isFeasible()) {
                break;
            }
        }

        if (!improvedThisLoop) {
            break;
        }
    }

    return overallImproved;
}

// ---- Ejection Global ----
bool LocalSearch::ejectionGlobal() {
    if (solution->isFeasible()) {
        return false;
    }

    const int TOP_K = 10;
    const int MAX_K = 3;

    struct Candidate {
        int item;
        int bin;
        int cost;
    };

    auto firstEmptyBin = [&]() -> int {
        for (int b = 0; b < solution->B; ++b) {
            if (solution->itemsInBin[b].empty()) return b;
        }
        return -1;
    };

    auto compactSolution = [&](const Solution& src) -> Solution {
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
    };

    int newBin = firstEmptyBin();
    if (newBin == -1) return false;

    std::vector<Candidate> pool;
    pool.reserve(64);

    for (int b : solution->badBins) {
        if (b < 0 || b >= solution->B) continue;
        if (solution->itemsInBin[b].size() <= 1) continue;

        for (int item : solution->itemsInBin[b]) {
            std::vector<int> A = {item};
            std::vector<int> B;

            int delta = deltaSwapSubsets(*solution, A, B, b, newBin);

            pool.push_back({item, b, delta});
        }
    }

    if (pool.empty()) return false;

    int limit = std::min(TOP_K, static_cast<int>(pool.size()));

    std::nth_element(
        pool.begin(),
        pool.begin() + limit,
        pool.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.cost < b.cost;
        }
    );

    pool.resize(limit);

    const int n = static_cast<int>(pool.size());
    const int maxMask = 1 << n;

    int bestDelta = 0;
    std::vector<int> bestSubset;
    std::vector<int> bestFroms;

    int subsetItems[MAX_K];
    int subsetFroms[MAX_K];

    for (int mask = 1; mask < maxMask; ++mask) {
        if (__builtin_popcount(static_cast<unsigned>(mask)) > MAX_K) {
            continue;
        }

        int sz = 0;

        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) {
                subsetItems[sz] = pool[i].item;
                subsetFroms[sz] = pool[i].bin;
                ++sz;
            }
        }

        bool duplicate = false;
        for (int i = 0; i < sz && !duplicate; ++i) {
            for (int j = i + 1; j < sz; ++j) {
                if (subsetItems[i] == subsetItems[j]) {
                    duplicate = true;
                    break;
                }
            }
        }
        if (duplicate) continue;

        int delta = 0;
        bool used[MAX_K] = {false};

        Solution temp = *solution;
        bool valid = true;

        for (int i = 0; i < sz; ++i) {
            if (used[i]) continue;

            int fromBin = subsetFroms[i];

            std::vector<int> A;
            std::vector<int> B;

            for (int j = i; j < sz; ++j) {
                if (!used[j] && subsetFroms[j] == fromBin) {
                    A.push_back(subsetItems[j]);
                    used[j] = true;
                }
            }

            int d = deltaSwapSubsets(temp, A, B, fromBin, newBin);
            delta += d;

            for (int item : A) {
                temp.moveItem(item, fromBin, newBin);
            }

            if (temp.itemBin[A.front()] != newBin) {
                valid = false;
                break;
            }
        }

        if (!valid) continue;

        if (improvementType == ImprovementType::FI) {
            if (delta < 0) {
                *solution = compactSolution(temp);
                return true;
            }
        } else {
            if (delta < bestDelta) {
                bestDelta = delta;
                bestSubset.assign(subsetItems, subsetItems + sz);
                bestFroms.assign(subsetFroms, subsetFroms + sz);
            }
        }
    }

    if (improvementType == ImprovementType::BI && bestDelta < 0) {
        int target = firstEmptyBin();
        if (target == -1) return false;

        Solution candidate = *solution;

        for (std::size_t i = 0; i < bestSubset.size(); ++i) {
            candidate.moveItem(bestSubset[i], bestFroms[i], target);
        }

        *solution = compactSolution(candidate);
        return true;
    }

    return false;
}

// ---- Hungarian (min-cost assignment) ----
int LocalSearch::hungarian(const std::vector<std::vector<int>>& cost,
                           std::vector<int>& assignment) {
    int n = static_cast<int>(cost.size());

    std::vector<int> u(n + 1), v(n + 1), p(n + 1), way(n + 1);

    for (int i = 1; i <= n; ++i) {
        p[0] = i;
        int j0 = 0;

        std::vector<int> minv(n + 1, INT_MAX);
        std::vector<char> used(n + 1, false);

        do {
            used[j0] = true;

            int i0 = p[j0];
            int delta = INT_MAX;
            int j1 = 0;

            for (int j = 1; j <= n; ++j) {
                if (used[j]) continue;

                int cur = cost[i0 - 1][j - 1] - u[i0] - v[j];

                if (cur < minv[j]) {
                    minv[j] = cur;
                    way[j] = j0;
                }

                if (minv[j] < delta) {
                    delta = minv[j];
                    j1 = j;
                }
            }

            for (int j = 0; j <= n; ++j) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }

            j0 = j1;

        } while (p[j0] != 0);

        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    assignment.assign(n, -1);

    for (int j = 1; j <= n; ++j) {
        assignment[p[j] - 1] = j - 1;
    }

    return -v[0];
}

// ---- Assignment neighborhood ----
bool LocalSearch::assignment(int N_ASSIGN) {
    if (solution->badBins.empty()) {
        return false;
    }

    const int EPS = 1;

    int usedBinCount = 0;
    for (int b = 0; b < solution->B; ++b) {
        if (!solution->itemsInBin[b].empty()) {
            ++usedBinCount;
        }
    }

    if (usedBinCount <= 1) {
        return false;
    }

    N_ASSIGN = std::min(N_ASSIGN, usedBinCount - 1);

    auto randInt = [&](int l, int r) {
        std::uniform_int_distribution<int> dist(l, r);
        return dist(rng);
    };

    int pivotIndex = randInt(0, static_cast<int>(solution->badBins.size()) - 1);
    int pivotBin = solution->badBins[pivotIndex];

    if (pivotBin < 0 || pivotBin >= solution->B) return false;
    if (solution->itemsInBin[pivotBin].empty()) return false;

    const auto& pivotItems = solution->itemsInBin[pivotBin];

    int pivotItem =
        pivotItems[randInt(0, static_cast<int>(pivotItems.size()) - 1)];

    std::vector<int> items;
    std::vector<int> froms;

    items.push_back(pivotItem);
    froms.push_back(pivotBin);

    std::vector<char> usedBin(solution->B, 0);
    usedBin[pivotBin] = 1;

    std::vector<int> binOrder;
    binOrder.reserve(solution->B);

    for (int b = 0; b < solution->B; ++b) {
        if (!solution->itemsInBin[b].empty()) {
            binOrder.push_back(b);
        }
    }

    std::shuffle(binOrder.begin(), binOrder.end(), rng);

    for (int b : binOrder) {
        if (static_cast<int>(items.size()) >= N_ASSIGN + 1) {
            break;
        }

        if (usedBin[b]) continue;
        if (solution->itemsInBin[b].empty()) continue;

        const auto& binItems = solution->itemsInBin[b];

        int item =
            binItems[randInt(0, static_cast<int>(binItems.size()) - 1)];

        items.push_back(item);
        froms.push_back(b);
        usedBin[b] = 1;
    }

    int n = static_cast<int>(items.size());

    if (n <= 1) {
        return false;
    }

    Solution temp = *solution;

    for (int i = 0; i < n; ++i) {
        temp.removeItem(items[i]);
    }

    std::vector<std::vector<int>> cost(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            Solution cand = temp;

            int before = cand.computeObjective();
            cand.addItem(items[i], froms[j]);
            int after = cand.computeObjective();

            cost[i][j] = after - before;

            if (items[i] == pivotItem && i == j) {
                cost[i][j] += EPS;
            }
        }
    }

    std::vector<int> assign;
    hungarian(cost, assign);

    bool identity = true;

    for (int i = 0; i < n; ++i) {
        if (assign[i] != i) {
            identity = false;
            break;
        }
    }

    if (identity) {
        return false;
    }

    Solution candidate = temp;

    for (int i = 0; i < n; ++i) {
        int item = items[i];
        int toBin = froms[assign[i]];

        candidate.addItem(item, toBin);
    }

    int objBefore = solution->computeObjective();
    int objAfter = candidate.computeObjective();

    if (objAfter <= objBefore) {
        *solution = candidate;
        return true;
    }

    return false;
}

// ---- Ejection Chain ----
bool LocalSearch::ejectionChain() {
    if (!solution || solution->B <= 1) return false;

    enum class NodeType { SOURCE, ITEM, ZERO };

    struct Pred {
        bool hasPred = false;
        NodeType type = NodeType::SOURCE;
        int id = -1;
    };

    struct NodeRef {
        NodeType type;
        int id;
    };

    struct Chain {
        double cost = 0.0;
        int zeroBin = -1;
        std::vector<NodeRef> path;
        std::unordered_set<int> itemsUsed;
        std::unordered_set<int> binsUsed;
    };

    const int nBins = solution->B;
    const int nItems = solution->N;
    const double INF = 1e18;

    auto binCostLS = [&](int type) -> int {
        return type < 0 ? 0 : solution->inst.binTypes[type].cost;
    };

    auto smallestType = [&](int load) -> int {
        if (load <= 0) return -1;
        for (int k = 0; k < static_cast<int>(solution->inst.binTypes.size()); ++k) {
            if (load <= solution->inst.binTypes[k].capacity) return k;
        }
        return static_cast<int>(solution->inst.binTypes.size()) - 1;
    };

    auto objectiveOfBin = [&](int b, int load, int conflicts) -> int {
        int type = smallestType(load);
        int cap = type < 0 ? 0 : solution->inst.binTypes[type].capacity;
        int excess = std::max(0, load - cap);
        return binCostLS(type) + solution->kw * excess + solution->kc * conflicts;
    };

    auto itemConflictsInBin = [&](int item, int bin, int ignore = -1) -> int {
        int c = 0;
        for (int other : solution->itemsInBin[bin]) {
            if (other == ignore) continue;

            int word = other >> 6;
            int bit = other & 63;

            if (solution->inst.conflicts[item][word] & (1ULL << bit)) {
                ++c;
            }
        }
        return c;
    };

    auto hasConflict = [&](int a, int b) -> bool {
        int word = b >> 6;
        int bit = b & 63;
        return (solution->inst.conflicts[a][word] & (1ULL << bit)) != 0;
    };

    auto removalCost = [&](int item) -> int {
        int b = solution->itemBin[item];
        if (b < 0) return 0;

        int oldObj = objectiveOfBin(
            b,
            solution->binLoad[b],
            solution->binConflicts[b]
        );

        int confRemoved = solution->confCount[item][b];
        int newObj = objectiveOfBin(
            b,
            solution->binLoad[b] - solution->inst.weights[item],
            solution->binConflicts[b] - confRemoved
        );

        return newObj - oldObj;
    };

    auto insertionCost = [&](int item, int bin) -> int {
        int oldObj = objectiveOfBin(
            bin,
            solution->binLoad[bin],
            solution->binConflicts[bin]
        );

        int confAdded = solution->confCount[item][bin];
        int newObj = objectiveOfBin(
            bin,
            solution->binLoad[bin] + solution->inst.weights[item],
            solution->binConflicts[bin] + confAdded
        );

        return newObj - oldObj;
    };

    auto replacementCost = [&](int inItem, int outItem) -> int {
        int b = solution->itemBin[outItem];
        if (b < 0) return 0;

        int oldObj = objectiveOfBin(
            b,
            solution->binLoad[b],
            solution->binConflicts[b]
        );

        int confOut = solution->confCount[outItem][b];
        int confIn = itemConflictsInBin(inItem, b, outItem);

        int newObj = objectiveOfBin(
            b,
            solution->binLoad[b] - solution->inst.weights[outItem] + solution->inst.weights[inItem],
            solution->binConflicts[b] - confOut + confIn
        );

        return newObj - oldObj;
    };

    std::vector<int> usedBins;
    for (int b = 0; b < nBins; ++b) {
        if (!solution->itemsInBin[b].empty()) usedBins.push_back(b);
    }

    if (usedBins.size() <= 1) return false;

    std::vector<int> pi = usedBins;
    std::shuffle(pi.begin(), pi.end(), rng);

    std::vector<int> rank(nBins, -1);
    for (int pos = 0; pos < static_cast<int>(pi.size()); ++pos) {
        rank[pi[pos]] = pos;
    }

    std::vector<std::vector<int>> itemsAtPos(pi.size());
    for (int item = 0; item < nItems; ++item) {
        int b = solution->itemBin[item];
        if (b >= 0 && rank[b] >= 0) {
            itemsAtPos[rank[b]].push_back(item);
        }
    }

    std::vector<double> distItem(nItems, INF);
    std::vector<double> distZero(nBins, INF);
    std::vector<Pred> predItem(nItems);
    std::vector<Pred> predZero(nBins);

    for (int item = 0; item < nItems; ++item) {
        if (solution->itemBin[item] >= 0) {
            distItem[item] = removalCost(item);
            predItem[item] = {true, NodeType::SOURCE, -1};
        }
    }

    for (int pos = 0; pos < static_cast<int>(pi.size()); ++pos) {
        int binFrom = pi[pos];

        std::vector<NodeRef> frontier;

        for (int item : itemsAtPos[pos]) {
            if (distItem[item] < INF) {
                frontier.push_back({NodeType::ITEM, item});
            }
        }

        if (distZero[binFrom] < INF) {
            frontier.push_back({NodeType::ZERO, binFrom});
        }

        if (frontier.empty()) continue;

        for (const NodeRef& u : frontier) {
            double du = (u.type == NodeType::ITEM)
                      ? distItem[u.id]
                      : distZero[u.id];

            for (int nextPos = pos + 1; nextPos < static_cast<int>(pi.size()); ++nextPos) {
                int binTo = pi[nextPos];

                for (int outItem : itemsAtPos[nextPos]) {
                    double arc = 0.0;

                    if (u.type == NodeType::ITEM) {
                        arc = replacementCost(u.id, outItem);
                    } else {
                        arc = removalCost(outItem);
                    }

                    if (du + arc < distItem[outItem]) {
                        distItem[outItem] = du + arc;
                        predItem[outItem] = {true, u.type, u.id};
                    }
                }

                if (u.type == NodeType::ITEM) {
                    double arc = insertionCost(u.id, binTo);

                    if (du + arc < distZero[binTo]) {
                        distZero[binTo] = du + arc;
                        predZero[binTo] = {true, NodeType::ITEM, u.id};
                    }
                }
            }
        }
    }

    std::vector<Chain> candidateChains;

    for (int zb : usedBins) {
        if (distZero[zb] >= -1e-9) continue;

        std::vector<NodeRef> revPath;
        NodeRef cur{NodeType::ZERO, zb};

        while (true) {
            revPath.push_back(cur);

            if (cur.type == NodeType::ZERO) {
                Pred p = predZero[cur.id];
                if (!p.hasPred || p.type == NodeType::SOURCE) break;
                cur = {p.type, p.id};
            } else {
                Pred p = predItem[cur.id];
                if (!p.hasPred || p.type == NodeType::SOURCE) break;
                cur = {p.type, p.id};
            }
        }

        std::reverse(revPath.begin(), revPath.end());

        Chain ch;
        ch.cost = distZero[zb];
        ch.zeroBin = zb;
        ch.path = revPath;

        for (const NodeRef& node : revPath) {
            if (node.type == NodeType::ITEM) {
                ch.itemsUsed.insert(node.id);
                int b = solution->itemBin[node.id];
                if (b >= 0) ch.binsUsed.insert(b);
            } else if (node.type == NodeType::ZERO) {
                ch.binsUsed.insert(node.id);
            }
        }

        candidateChains.push_back(std::move(ch));
    }

    if (candidateChains.empty()) return false;

    std::sort(candidateChains.begin(), candidateChains.end(),
        [](const Chain& a, const Chain& b) {
            return a.cost < b.cost;
        });

    std::vector<Chain> selected;
    std::unordered_set<int> usedItems;
    std::unordered_set<int> usedBinsSet;

    for (const Chain& ch : candidateChains) {
        bool disjoint = true;

        for (int item : ch.itemsUsed) {
            if (usedItems.count(item)) {
                disjoint = false;
                break;
            }
        }

        if (!disjoint) continue;

        for (int b : ch.binsUsed) {
            if (usedBinsSet.count(b)) {
                disjoint = false;
                break;
            }
        }

        if (!disjoint) continue;

        selected.push_back(ch);
        usedItems.insert(ch.itemsUsed.begin(), ch.itemsUsed.end());
        usedBinsSet.insert(ch.binsUsed.begin(), ch.binsUsed.end());
    }

    if (selected.empty()) return false;

    Solution candidate = *solution;

    for (const Chain& ch : selected) {
        int idx = 0;

        while (idx < static_cast<int>(ch.path.size())) {
            if (ch.path[idx].type != NodeType::ITEM) {
                ++idx;
                continue;
            }

            int carried = ch.path[idx].id;
            int carriedFromBin = candidate.itemBin[carried];
            candidate.removeItem(carried);
            ++idx;

            while (idx < static_cast<int>(ch.path.size())) {
                if (ch.path[idx].type == NodeType::ITEM) {
                    int outItem = ch.path[idx].id;
                    int b = candidate.itemBin[outItem];

                    candidate.removeItem(outItem);
                    candidate.addItem(carried, b);

                    carried = outItem;
                    ++idx;
                } else if (ch.path[idx].type == NodeType::ZERO) {
                    int b = ch.path[idx].id;
                    candidate.addItem(carried, b);
                    ++idx;
                    break;
                } else {
                    ++idx;
                    break;
                }
            }
        }
    }

    int oldObj = solution->computeObjective();
    int newObj = candidate.computeObjective();

    if (newObj < oldObj) {
        *solution = std::move(candidate);
        return true;
    }

    return false;
}

// ---- Grenade ----
bool LocalSearch::grenade() {
    if (!solution || solution->B <= 1) return false;

    const int oldObj = solution->computeObjective();

    auto hasConflict = [&](int a, int b) -> bool {
        int word = b >> 6;
        int bit = b & 63;
        return (solution->inst.conflicts[a][word] & (1ULL << bit)) != 0;
    };

    auto bestInsertionForItem = [&](const Solution& s,
                                    int item,
                                    int forbiddenBin,
                                    int& bestBin,
                                    int& bestDelta) {
        bestBin = -1;
        bestDelta = INT_MAX;

        int fromBin = s.itemBin[item];
        if (fromBin < 0) return false;

        for (int b = 0; b < s.B; ++b) {
            if (b == forbiddenBin) continue;
            if (b == fromBin) continue;

            Solution tmp = s;
            int before = tmp.computeObjective();
            tmp.moveItem(item, fromBin, b);
            int after = tmp.computeObjective();

            int delta = after - before;

            if (delta < bestDelta) {
                bestDelta = delta;
                bestBin = b;
            }
        }

        return bestBin != -1;
    };

    struct Relocation {
        int item = -1;
        int toBin = -1;
    };

    std::vector<int> problematic;
    std::vector<char> used(solution->N, 0);

    for (int b : solution->badBins) {
        if (b < 0 || b >= solution->B) continue;

        for (int item : solution->itemsInBin[b]) {
            if (!used[item]) {
                used[item] = 1;
                problematic.push_back(item);
            }
        }
    }

    std::shuffle(problematic.begin(), problematic.end(), rng);

    for (int k : problematic) {
        int fromBinK = solution->itemBin[k];
        if (fromBinK < 0) continue;

        int bestTotalDelta = INT_MAX;
        int bestTargetBin = -1;
        std::vector<Relocation> bestSecondaryMoves;

        for (int targetBin = 0; targetBin < solution->B; ++targetBin) {
            if (targetBin == fromBinK) continue;

            Solution tmp = *solution;

            int beforeK = tmp.computeObjective();
            tmp.moveItem(k, fromBinK, targetBin);
            int afterK = tmp.computeObjective();

            int totalDelta = afterK - beforeK;
            std::vector<Relocation> secondaryMoves;

            std::vector<int> conflictingItems;
            for (int item : tmp.itemsInBin[targetBin]) {
                if (item != k && hasConflict(k, item)) {
                    conflictingItems.push_back(item);
                }
            }

            std::shuffle(conflictingItems.begin(), conflictingItems.end(), rng);

            bool valid = true;

            for (int i : conflictingItems) {
                int bestBinI = -1;
                int bestDeltaI = INT_MAX;

                if (!bestInsertionForItem(tmp, i, targetBin, bestBinI, bestDeltaI)) {
                    valid = false;
                    break;
                }

                secondaryMoves.push_back({i, bestBinI});
                totalDelta += bestDeltaI;

                int curBin = tmp.itemBin[i];
                tmp.moveItem(i, curBin, bestBinI);
            }

            if (!valid) continue;

            if (totalDelta < bestTotalDelta) {
                bestTotalDelta = totalDelta;
                bestTargetBin = targetBin;
                bestSecondaryMoves = secondaryMoves;
            }
        }

        if (bestTargetBin != -1 && bestTotalDelta < 0) {
            Solution backup = *solution;

            int currentFromBin = solution->itemBin[k];
            solution->moveItem(k, currentFromBin, bestTargetBin);

            for (const Relocation& mv : bestSecondaryMoves) {
                int curBin = solution->itemBin[mv.item];

                if (curBin >= 0 && curBin != mv.toBin) {
                    solution->moveItem(mv.item, curBin, mv.toBin);
                }
            }

            int newObj = solution->computeObjective();

            if (newObj < oldObj) {
                return true;
            }

            *solution = std::move(backup);
        }
    }

    return false;
}

// ---- Set solution ----
void LocalSearch::setSolution(Solution& sol) {
    solution = &sol;
}