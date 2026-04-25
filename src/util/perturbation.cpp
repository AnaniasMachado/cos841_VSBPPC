#include "perturbation.hpp"
#include <vector>
#include <algorithm>

// ---- RELOCATE K ----
void Perturbation::relocateK(Solution& sol, int k, std::mt19937& rng) {
    std::vector<int> assignedItems;
    assignedItems.reserve(sol.N);

    for (int i = 0; i < sol.N; i++) {
        if (sol.itemBin[i] != -1) {
            assignedItems.push_back(i);
        }
    }

    if (assignedItems.empty() || sol.B <= 1) return;

    k = std::min(k, static_cast<int>(assignedItems.size()));

    std::shuffle(assignedItems.begin(), assignedItems.end(), rng);

    for (int p = 0; p < k; p++) {
        int i = assignedItems[p];
        int from = sol.itemBin[i];

        std::vector<int> candidateBins;
        candidateBins.reserve(sol.B - 1);

        for (int b = 0; b < sol.B; b++) {
            if (b != from) {
                candidateBins.push_back(b);
            }
        }

        std::uniform_int_distribution<int> dist(0, static_cast<int>(candidateBins.size()) - 1);
        int to = candidateBins[dist(rng)];

        sol.moveItem(i, from, to);
    }
}

// ---- EXCHANGE K ----
// Exchanges k random pairs of items.
void Perturbation::exchangeK(Solution& sol, int k, std::mt19937& rng) {
    std::vector<int> assignedItems;
    assignedItems.reserve(sol.N);

    for (int i = 0; i < sol.N; ++i) {
        if (sol.itemBin[i] != -1) {
            assignedItems.push_back(i);
        }
    }

    if (assignedItems.size() <= 1 || k <= 0) return;

    std::uniform_int_distribution<int> dist(
        0,
        static_cast<int>(assignedItems.size()) - 1
    );

    for (int p = 0; p < k; ++p) {
        int i = assignedItems[dist(rng)];
        int j = assignedItems[dist(rng)];

        if (i == j) continue;

        int binI = sol.itemBin[i];
        int binJ = sol.itemBin[j];

        if (binI == -1 || binJ == -1 || binI == binJ) continue;

        sol.swapItems(i, j);
    }
}

// ---- MERGE ----
// Randomly selects two non-empty bins and moves all items
// from the second bin into the first bin.
void Perturbation::merge(Solution& sol, std::mt19937& rng) {
    std::vector<int> usedBins;

    for (int b = 0; b < sol.B; ++b) {
        if (!sol.itemsInBin[b].empty()) {
            usedBins.push_back(b);
        }
    }

    if (usedBins.size() <= 1) return;

    std::shuffle(usedBins.begin(), usedBins.end(), rng);

    int target = usedBins[0];
    int source = usedBins[1];

    std::vector<int> items = sol.itemsInBin[source];

    for (int item : items) {
        sol.moveItem(item, source, target);
    }
}

// ---- MERGE K ----
// Merges k random pairs of non-empty bins.
// For each pair, all items from the second bin are moved into the first bin.
void Perturbation::mergeK(Solution& sol, int k, std::mt19937& rng) {
    if (k <= 0) return;

    for (int p = 0; p < k; ++p) {
        std::vector<int> usedBins;

        for (int b = 0; b < sol.B; ++b) {
            if (!sol.itemsInBin[b].empty()) {
                usedBins.push_back(b);
            }
        }

        if (usedBins.size() <= 1) return;

        std::shuffle(usedBins.begin(), usedBins.end(), rng);

        int b1 = usedBins[0];
        int b2 = usedBins[1];

        std::vector<int> items = sol.itemsInBin[b2];

        for (int item : items) {
            sol.moveItem(item, b2, b1);
        }
    }
}

// ---- SPLIT ----
void Perturbation::split(Solution& sol, std::mt19937& rng) {
    std::vector<int> usedBins;

    for (int b = 0; b < sol.B; b++) {
        if (!sol.itemsInBin[b].empty()) {
            usedBins.push_back(b);
        }
    }

    if (usedBins.size() <= 1) return;

    std::uniform_int_distribution<int> dist(0, usedBins.size() - 1);
    int b = usedBins[dist(rng)];

    std::vector<int> items = sol.itemsInBin[b];

    // remove all items from chosen bin
    for (int i : items) {
        sol.removeItem(i);
    }

    std::uniform_int_distribution<int> binDist(0, sol.B - 1);

    // redistribute randomly (excluding original bin)
    for (int i : items) {
        int to;
        do {
            to = binDist(rng);
        } while (to == b);

        sol.addItem(i, to);
    }
}

// ---- SPLIT K ----
// Splits the contents of k random non-empty bins.
// Each selected bin is emptied, and its items are randomly reassigned
// to other bins.
void Perturbation::splitK(Solution& sol, int k, std::mt19937& rng) {
    std::vector<int> usedBins;

    for (int b = 0; b < sol.B; ++b) {
        if (!sol.itemsInBin[b].empty()) {
            usedBins.push_back(b);
        }
    }

    if (usedBins.size() <= 1 || k <= 0) return;

    k = std::min(k, static_cast<int>(usedBins.size()));

    std::shuffle(usedBins.begin(), usedBins.end(), rng);

    for (int p = 0; p < k; ++p) {
        int b = usedBins[p];

        if (sol.itemsInBin[b].empty()) continue;

        std::vector<int> items = sol.itemsInBin[b];

        for (int item : items) {
            sol.removeItem(item);
        }

        std::vector<int> candidateBins;
        candidateBins.reserve(sol.B - 1);

        for (int to = 0; to < sol.B; ++to) {
            if (to != b) {
                candidateBins.push_back(to);
            }
        }

        if (candidateBins.empty()) {
            for (int item : items) {
                sol.addItem(item, b);
            }
            continue;
        }

        std::uniform_int_distribution<int> dist(
            0,
            static_cast<int>(candidateBins.size()) - 1
        );

        for (int item : items) {
            int to = candidateBins[dist(rng)];
            sol.addItem(item, to);
        }
    }
}