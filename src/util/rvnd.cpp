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

    std::vector<int> p = {0, 1, 2};
    std::shuffle(p.begin(), p.end(), rng);
    int k = 0;

    while (k < static_cast<int>(p.size())) {
        bool improved = false;

        switch (p[k]) {
            case 0: improved = ls.classic();                        break;
            case 1: improved = ls.ejectionGlobal();                 break;
            case 2: improved = ls.ejectionChain();                  break;
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

// void RVND::run() {
//     bool improved_global = false;

//     std::vector<int> p = {0, 1, 2, 3, 4};
//     std::shuffle(p.begin(), p.end(), rng);
//     int k = 0;

//     auto neighborhoodName = [](int id) -> const char* {
//         switch (id) {
//             case 0: return "classic";
//             case 1: return "ejectionGlobal";
//             case 2: return "assignment";
//             case 3: return "ejectionChain";
//             case 4: return "grenade";
//             default: return "unknown";
//         }
//     };

//     while (k < static_cast<int>(p.size())) {
//         bool improved = false;

//         const int neigh = p[k];
//         const char* name = neighborhoodName(neigh);

//         std::cout << "[RVND] iter=" << iter
//                   << " k=" << k
//                   << " start " << name
//                   << " obj=" << sol->computeObjective()
//                   << " feasible=" << (sol->isFeasible() ? "yes" : "no")
//                   << " badBins=" << sol->badBins.size()
//                   << std::endl;

//         auto start = std::chrono::high_resolution_clock::now();

//         switch (neigh) {
//             case 0:
//                 improved = ls.classic();
//                 break;

//             case 1:
//                 improved = ls.ejectionGlobal();
//                 break;

//             // case 2:
//             //     improved = ls.assignment(static_cast<int>(sol->N) / 20);
//             //     break;

//             case 3:
//                 improved = ls.ejectionChain();
//                 break;

//             // case 4:
//             //     improved = ls.grenade();
//             //     break;

//             default:
//                 improved = false;
//                 break;
//         }

//         auto end = std::chrono::high_resolution_clock::now();
//         std::chrono::duration<double> elapsed = end - start;

//         std::cout << "[RVND] iter=" << iter
//                   << " k=" << k
//                   << " end " << name
//                   << " improved=" << (improved ? "yes" : "no")
//                   << " runtime=" << elapsed.count() << " s"
//                   << " obj=" << sol->computeObjective()
//                   << " feasible=" << (sol->isFeasible() ? "yes" : "no")
//                   << " badBins=" << sol->badBins.size()
//                   << std::endl;

//         if (improved) {
//             improved_global = true;
//             k = 0;   // restart using same permutation
//         } else {
//             k++;
//         }
//     }

//     iter++;
// }

// -------------------- Set solution --------------------
void RVND::setSolution(Solution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}