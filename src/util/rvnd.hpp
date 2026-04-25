#ifndef RVND_H
#define RVND_H

#include "solution.hpp"
#include "local_search.hpp"

#include <algorithm>
#include <random>
#include <vector>

class RVND {
public:
    Solution* sol;
    LocalSearch ls;

    std::mt19937& rng;
    int iter;

    RVND(Solution& solution,
         ImprovementType improvement_type_,
         std::mt19937& rng_);

    void run();
    void setSolution(Solution& solution);
};

#endif