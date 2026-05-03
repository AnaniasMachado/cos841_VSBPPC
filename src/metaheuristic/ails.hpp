#ifndef AILS_HPP
#define AILS_HPP

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/perturbation.hpp"
#include "../util/rvnd.hpp"
#include "../util/qrvnd.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <stdexcept>

enum class AcceptanceType {
    BEST,
    ITERATIVE,
    RW
};

enum class BuilderType {
    GREEDY_COST,
    DEGREE_GREEDY,
    FFD,
    BFD,
    ALI_EKICI,
    HC1,
    HC2,
    IFFD,
    IBFD
};

enum class PerturbationType {
    RELOCATEK,
    EXCHANGEK,
    MERGEK,
    SPLITK
};

class AILS {
private:
    const VSBPPCInstance& inst;

    int kw;
    int kc;

    int max_iterations;
    int max_no_improve;

    AcceptanceType acceptance_type;
    ImprovementType improvement_type;

    BuilderType builder_type;

    bool useUCB;
    double c;

    bool useQRVND;
    double alpha;
    double gamma;
    double epsilon;

    bool exactCover;

    bool verbose;
    double time_limit;

    std::mt19937& rng;
    std::uniform_int_distribution<int> dist;

    std::vector<int> perturbation_count;
    std::vector<int> perturbation_success;

    int usedBinCount(const Solution& sol) const;

    int computeK(PerturbationType perturbation_type,
                 const Solution& sol,
                 int no_improve) const;

    PerturbationType getPerturbationType(int idx) const;

    PerturbationType selectPerturbation(int iter);

    void applyPerturbation(PerturbationType perturbation_type,
                           Solution& sol,
                           int no_improve);

    void updateUCB(PerturbationType perturbation_type,
                   bool reward);

    Solution buildInitialSolution() const;

public:
    AILS(const VSBPPCInstance& instance,
        int kw,
        int kc,
        std::mt19937& rng,
        int max_it = 1000,
        int max_no_imp = 150,
        AcceptanceType acceptance_type = AcceptanceType::ITERATIVE,
        ImprovementType improvement_type = ImprovementType::BI,
        bool use_ucb = false,
        double c = 1.0,
        BuilderType builder_type = BuilderType::ALI_EKICI,
        bool use_qrvnd = false,
        double alpha = 0.1,
        double gamma = 0.9,
        double epsilon = 0.1,
        bool exactCover = false,
        bool verbose = false,
        double time_limit = 3600.0);

    Solution run();
};

#endif