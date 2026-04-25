#include "rvnd.hpp"

// -------------------- Constructor --------------------
RVND::RVND(Solution& solution,
           ImprovementType improvement_type_,
           std::mt19937& rng_)
    : sol(&solution),
      ls(*sol, improvement_type_, rng_),
      rng(rng_),
      iter(-1) {}

// -------------------- RVND --------------------
void RVND::run() {
    bool improved_global = false;

    std::vector<int> p = {0, 1, 2, 3, 4};
    std::shuffle(p.begin(), p.end(), rng);
    int k = 0;

    while (k < static_cast<int>(p.size())) {
        bool improved = false;

        switch (p[k]) {
            case 0: improved = ls.classic();                        break;
            case 1: improved = ls.ejectionGlobal();                 break;
            case 2: improved = ls.assignment((int)sol->N / 20);     break;
            case 3: improved = ls.ejectionChain();                  break;
            case 4: improved = ls.grenade();                        break;
        }

        if (improved) {
            improved_global = true;
            k = 0;   // restart using same permutation
        } else {
            k++;
        }
    }

    iter++;
}

// -------------------- Set solution --------------------
void RVND::setSolution(Solution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}