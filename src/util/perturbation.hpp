#ifndef PERTURBATION_H
#define PERTURBATION_H

#include "solution.hpp"
#include <random>

class Perturbation {
public:
    static void relocateK(Solution& sol, int k, std::mt19937& rng);
    static void split(Solution& sol, std::mt19937& rng);
};

#endif