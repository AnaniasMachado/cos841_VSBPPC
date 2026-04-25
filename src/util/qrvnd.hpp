#ifndef QRVND_HPP
#define QRVND_HPP

#include "solution.hpp"
#include "local_search.hpp"

#include <algorithm>
#include <vector>
#include <random>

class QRVND {
private:
    Solution* sol;
    LocalSearch ls;

    std::mt19937& rng;
    std::uniform_real_distribution<double> dist01;

    double alpha;
    double gamma;
    double epsilon;

    bool initialized;
    int current_p;
    int iter;

    std::vector<std::vector<int>> perms;
    std::vector<std::vector<double>> Q;

    std::vector<std::vector<int>> generatePermutations(int n);

    void backtrack(int n,
                   std::vector<int>& current,
                   std::vector<bool>& used,
                   std::vector<std::vector<int>>& perms);

    int selectPermutation(int current_p);

    bool applyOrder(const std::vector<int>& order);

public:
    QRVND(Solution& solution,
          ImprovementType improvement_type_,
          std::mt19937& rng_,
          double a,
          double g,
          double e);

    void run();

    void setSolution(Solution& solution);
};

#endif