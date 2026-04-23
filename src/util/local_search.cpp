#include "local_search.hpp"
#include <algorithm>
#include <limits>

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

// ---- Set solution ----
void LocalSearch::setSolution(Solution& sol) {
    solution = &sol;
}