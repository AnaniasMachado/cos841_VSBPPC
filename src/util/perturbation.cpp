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