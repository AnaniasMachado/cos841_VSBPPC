#include "qrvnd.hpp"

// -------------------- Constructor --------------------
QRVND::QRVND(Solution& solution,
             ImprovementType improvement_type_,
             std::mt19937& rng_,
             double a,
             double g,
             double e)
    : sol(&solution),
      ls(*sol, improvement_type_, rng_),
      rng(rng_),
      dist01(0.0, 1.0),
      alpha(a),
      gamma(g),
      epsilon(e),
      initialized(false),
      current_p(0),
      iter(-1) {
    perms = generatePermutations(5);

    Q = std::vector<std::vector<double>>(
        perms.size(),
        std::vector<double>(perms.size(), 0.0)
    );
}

// -------------------- Generate Permutations --------------------
std::vector<std::vector<int>> QRVND::generatePermutations(int n) {
    std::vector<std::vector<int>> perms;
    std::vector<int> current;
    std::vector<bool> used(n, false);

    backtrack(n, current, used, perms);

    return perms;
}

// -------------------- Backtrack --------------------
void QRVND::backtrack(int n,
                      std::vector<int>& current,
                      std::vector<bool>& used,
                      std::vector<std::vector<int>>& perms) {
    if (static_cast<int>(current.size()) == n) {
        perms.push_back(current);
        return;
    }

    for (int i = 0; i < n; ++i) {
        if (!used[i]) {
            used[i] = true;
            current.push_back(i);

            backtrack(n, current, used, perms);

            current.pop_back();
            used[i] = false;
        }
    }
}

// -------------------- Select --------------------
int QRVND::selectPermutation(int current_p) {
    // Exploration
    if (dist01(rng) < epsilon) {
        std::uniform_int_distribution<int> d(
            0,
            static_cast<int>(perms.size()) - 1
        );

        return d(rng);
    }

    // Exploitation
    return static_cast<int>(
        std::distance(
            Q[current_p].begin(),
            std::max_element(Q[current_p].begin(), Q[current_p].end())
        )
    );
}

// -------------------- Apply ONE RVND --------------------
bool QRVND::applyOrder(const std::vector<int>& order) {
    bool improved_global = false;

    std::vector<int> c = order;
    int k = 0;

    while (k < static_cast<int>(c.size())) {
        bool improved = false;

        switch (c[k]) {
            case 0: improved = ls.classic();                        break;
            case 1: improved = ls.ejectionGlobal();                 break;
            case 2: improved = ls.assignment((int)sol->N / 20);     break;
            case 3: improved = ls.ejectionChain();                  break;
            case 4: improved = ls.grenade();                        break;
        }

        if (improved) {
            improved_global = true;

            // Restart RVND with the same selected permutation
            k = 0;
        } else {
            k++;
        }
    }

    iter++;

    return improved_global;
}

// -------------------- Run ONE QRVND iteration --------------------
void QRVND::run() {
    if (!initialized) {
        std::uniform_int_distribution<int> d(
            0,
            static_cast<int>(perms.size()) - 1
        );

        current_p = d(rng);
        initialized = true;
    }

    int next_p = selectPermutation(current_p);

    int before = sol->computeObjective();

    bool improved = applyOrder(perms[next_p]);

    int after = sol->computeObjective();

    // Reward
    double reward = 0.0;

    if (before > 0 && improved) {
        reward = (before - after) / static_cast<double>(before);
    }

    // Q-learning update
    double max_next = *std::max_element(
        Q[next_p].begin(),
        Q[next_p].end()
    );

    Q[current_p][next_p] += alpha *
        (reward + gamma * max_next - Q[current_p][next_p]);

    current_p = next_p;

    // Mild epsilon decay
    epsilon = std::max(0.05, epsilon * 0.9);
}

// -------------------- Set Solution --------------------
void QRVND::setSolution(Solution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}