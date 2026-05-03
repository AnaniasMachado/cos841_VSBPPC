#include "ails.hpp"

// -------------------- Constructor --------------------
AILS::AILS(const VSBPPCInstance& instance,
           int kw_,
           int kc_,
           std::mt19937& rng_,
           int max_it,
           int max_no_imp,
           AcceptanceType acceptance_type_,
           ImprovementType improvement_type_,
           bool use_ucb,
           double c_,
           BuilderType builder_type_,
           bool use_qrvnd_,
           double alpha_,
           double gamma_,
           double epsilon_,
           bool exact_cover_,
           bool verbose_,
           double time_limit_)
    : inst(instance),
      kw(kw_),
      kc(kc_),
      max_iterations(max_it),
      max_no_improve(max_no_imp),
      acceptance_type(acceptance_type_),
      improvement_type(improvement_type_),
      builder_type(builder_type_),
      useUCB(use_ucb),
      c(c_),
      useQRVND(use_qrvnd_),
      alpha(alpha_),
      gamma(gamma_),
      epsilon(epsilon_),
      exactCover(exact_cover_),
      verbose(verbose_),
      time_limit(time_limit_),
      rng(rng_),
      dist(0, 3),
      perturbation_count(4, 0),
      perturbation_success(4, 0) {}

int AILS::usedBinCount(const Solution& sol) const {
    int count = 0;

    for (int b = 0; b < sol.B; ++b) {
        if (!sol.itemsInBin[b].empty()) {
            ++count;
        }
    }

    return count;
}

// -------------------- Adaptive k --------------------
int AILS::computeK(PerturbationType perturbation_type,
                   const Solution& sol,
                   int no_improve) const {
    int n_items = inst.N;
    int n_bins = std::max(1, usedBinCount(sol));

    double stagnation = static_cast<double>(no_improve) / static_cast<double>(max_no_improve);

    double intensity = std::min(1.0, stagnation * 2.0);

    if (perturbation_type == PerturbationType::RELOCATEK ||
        perturbation_type == PerturbationType::EXCHANGEK) {

        int k_min = std::max(1, static_cast<int>(0.005 * n_items));
        int k_max = std::max(k_min + 1, static_cast<int>(0.025 * n_items));

        int k = k_min + static_cast<int>((k_max - k_min) * intensity);
        return std::max(1, k);
    }

    if (perturbation_type == PerturbationType::MERGEK) {
        int k_min = std::max(1, static_cast<int>(0.0025 * n_bins));
        int k_max = std::max(k_min + 1, static_cast<int>(0.01 * n_bins));

        int k = k_min + static_cast<int>((k_max - k_min) * intensity);
        return std::max(1, k);
    }

    if (perturbation_type == PerturbationType::SPLITK) {
        int k_min = std::max(1, static_cast<int>(0.0025 * n_bins));
        int k_max = std::max(k_min + 1, static_cast<int>(0.0075 * n_bins));

        int k = k_min + static_cast<int>((k_max - k_min) * intensity);
        return std::max(1, k);
    }

    return 1;
}

// -------------------- Switch perturbation --------------------
PerturbationType AILS::getPerturbationType(int idx) const {
    switch (idx) {
        case 0: return PerturbationType::RELOCATEK;
        case 1: return PerturbationType::EXCHANGEK;
        case 2: return PerturbationType::MERGEK;
        case 3: return PerturbationType::SPLITK;
    }
    throw std::runtime_error("Invalid perturbation index in AILS::getPerturbationType.");
}

// -------------------- Select perturbation --------------------
PerturbationType AILS::selectPerturbation(int iter) {
    if (!useUCB) {
        return getPerturbationType(dist(rng));
    }

    for (int i = 0; i < 4; ++i) {
        if (perturbation_count[i] == 0) {
            return getPerturbationType(i);
        }
    }

    double bestScore = std::numeric_limits<double>::lowest();
    int best = 0;

    int safeIter = std::max(2, iter);

    for (int i = 0; i < 4; ++i) {
        double mean =
            static_cast<double>(perturbation_success[i]) /
            static_cast<double>(perturbation_count[i]);

        double bonus =
            c * std::sqrt(std::log(static_cast<double>(safeIter)) /
                          static_cast<double>(perturbation_count[i]));

        double score = mean + bonus;

        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }

    return getPerturbationType(best);
}

// -------------------- Apply perturbation --------------------
void AILS::applyPerturbation(PerturbationType perturbation_type,
                             Solution& sol,
                             int no_improve) {
    int k = computeK(perturbation_type, sol, no_improve);

    switch (perturbation_type) {
        case PerturbationType::RELOCATEK:
            Perturbation::relocateK(sol, k, rng);
            break;

        case PerturbationType::EXCHANGEK:
            Perturbation::exchangeK(sol, k, rng);
            break;

        case PerturbationType::MERGEK:
            Perturbation::mergeK(sol, k, rng);
            break;

        case PerturbationType::SPLITK:
            Perturbation::splitK(sol, k, rng);
            break;
    }
}

// -------------------- Update UCB --------------------
void AILS::updateUCB(PerturbationType perturbation_type,
                     bool reward) {
    int p = 0;

    switch (perturbation_type) {
        case PerturbationType::RELOCATEK: p = 0; break;
        case PerturbationType::EXCHANGEK: p = 1; break;
        case PerturbationType::MERGEK:    p = 2; break;
        case PerturbationType::SPLITK:    p = 3; break;
    }

    ++perturbation_count[p];

    if (reward) {
        ++perturbation_success[p];
    }
}

// -------------------- Builder selection --------------------
Solution AILS::buildInitialSolution() const {
    switch (builder_type) {
        case BuilderType::GREEDY_COST:
            return Builder::greedyCost(inst, kw, kc, nullptr);

        case BuilderType::DEGREE_GREEDY:
            return Builder::degreeGreedy(inst, kw, kc, nullptr);

        case BuilderType::FFD:
            return Builder::firstFitDecreasing(inst, kw, kc, nullptr);

        case BuilderType::BFD:
            return Builder::bestFitDecreasing(inst, kw, kc, nullptr);

        case BuilderType::ALI_EKICI:
            return Builder::aliEkiciConstructive(inst, kw, kc, nullptr);

        case BuilderType::HC1:
            return Builder::HC1(inst, kw, kc, nullptr);

        case BuilderType::HC2:
            return Builder::HC2(inst, kw, kc, nullptr);

        case BuilderType::IFFD: {
            std::vector<double> alphas = {0.0, 0.25, 0.5, 0.75, 1.0};

            Solution bestSol = Builder::IFFD(inst, kw, kc, alphas[0], nullptr);
            int bestObj = bestSol.computeObjective();

            for (std::size_t idx = 1; idx < alphas.size(); ++idx) {
                Solution sol = Builder::IFFD(inst, kw, kc, alphas[idx], nullptr);
                int obj = sol.computeObjective();

                if (obj < bestObj) {
                    bestObj = obj;
                    bestSol = sol;
                }
            }

            return bestSol;
        }

        case BuilderType::IBFD: {
            std::vector<double> alphas = {0.0, 0.25, 0.5, 0.75, 1.0};

            Solution bestSol = Builder::IBFD(inst, kw, kc, alphas[0], nullptr);
            int bestObj = bestSol.computeObjective();

            for (std::size_t idx = 1; idx < alphas.size(); ++idx) {
                Solution sol = Builder::IBFD(inst, kw, kc, alphas[idx], nullptr);
                int obj = sol.computeObjective();

                if (obj < bestObj) {
                    bestObj = obj;
                    bestSol = sol;
                }
            }

            return bestSol;
        }
    }
    throw std::runtime_error("Invalid builder type in AILS::buildInitialSolution.");
}

// -------------------- Main AILS --------------------
Solution AILS::run() {
    auto start = std::chrono::high_resolution_clock::now();

    Solution current = buildInitialSolution();

    if (verbose) {
        std::cout << "===== INITIAL CONSTRUCTIVE RESULT =====\n";
        std::cout << "Objective: " << current.computeObjective() << "\n";
        std::cout << "Feasible: " << (current.isFeasible() ? "YES" : "NO") << "\n\n";
    }

    RVND rvnd(current, improvement_type, rng);
    QRVND qrvnd(current, improvement_type, rng, alpha, gamma, epsilon);

    // -------------------- INITIAL LOCAL SEARCH --------------------
    if (useQRVND) {
        qrvnd.run();
    } else {
        rvnd.run();
    }

    Solution best = current;

    if (verbose) {
        std::cout << "===== INITIAL AILS RESULT =====\n";
        std::cout << "Objective: " << best.computeObjective() << "\n";
        std::cout << "Feasible: " << (best.isFeasible() ? "YES" : "NO") << "\n\n";
    }

    int iter = 0;
    int no_improve = 0;

    auto applySetCoveringIfNeeded = [&](int iter) {
        if (iter % 10 != 0 || iter < 10) return;

        auto& ls = useQRVND ? qrvnd.ls : rvnd.ls;

        // Add columns from the current solution to the SC pool.
        ls.addToSetCoveringPool(*ls.solution);

        // Try the set-covering neighborhood.
        if (ls.setCoveringNeighborhood(30.0, exactCover)) {
            // If it improved, refresh the elite and add the new bins too.
            ls.initializeSetCoveringFromElite(*ls.solution);
            ls.addToSetCoveringPool(*ls.solution);
        }
    };

    auto initSetCoveringIfEnabled = [&]() {
        auto& ls = useQRVND ? qrvnd.ls : rvnd.ls;
        ls.initializeSetCoveringFromElite(best);
    };

    auto updateSetCoveringPoolWithLocalOptimum = [&]() {
        auto& ls = useQRVND ? qrvnd.ls : rvnd.ls;

        if (ls.solution && ls.solution->isFeasible()) {
            ls.addToSetCoveringPool(*ls.solution);
        }
    };

    updateSetCoveringPoolWithLocalOptimum();

    // -------------------- MAIN LOOP --------------------
    while (iter < max_iterations && no_improve < max_no_improve) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - start).count();

        if (elapsed > time_limit) {
            if (verbose) {
                std::cout << "ExceedTimeLimit: " << elapsed << "\n";
            }
            break;
        }

        int currentObj = current.computeObjective();
        int bestObj = best.computeObjective();

        if (verbose && iter % 5 == 0) {
            std::cout << "Iteration: " << iter
                      << " | Current: " << currentObj
                      << " | Best: " << bestObj
                      << " | No improve: " << no_improve
                      << "\n";
        }

        Solution candidate = current;

        // ---- Perturbation ----
        PerturbationType perturbationType = selectPerturbation(iter + 1);
        applyPerturbation(perturbationType, candidate, no_improve);

        // ---- Local Search ----
        if (useQRVND) {
            qrvnd.setSolution(candidate);
            qrvnd.run();
        } else {
            rvnd.setSolution(candidate);
            rvnd.run();
        }

        updateSetCoveringPoolWithLocalOptimum();

        applySetCoveringIfNeeded(iter);

        int candObj = candidate.computeObjective();

        bool improvedBest = false;

        // ---- Best update ----
        if (candidate.isFeasible() && candObj < bestObj) {
            best = candidate;
            no_improve = 0;
            improvedBest = true;
            initSetCoveringIfEnabled();
        } else {
            ++no_improve;
        }

        if (useUCB || verbose) {
            updateUCB(perturbationType, improvedBest);
        }

        // ---- Acceptance ----
        bool acceptCurrent = false;

        if (acceptance_type == AcceptanceType::BEST) {
            acceptCurrent = improvedBest;
        } else if (acceptance_type == AcceptanceType::ITERATIVE) {
            acceptCurrent = candidate.isFeasible() && candObj < currentObj;
        } else if (acceptance_type == AcceptanceType::RW) {
            acceptCurrent = candidate.isFeasible();
        }

        if (acceptCurrent) {
            current = candidate;

            if (useQRVND) {
                qrvnd.setSolution(current);
            } else {
                rvnd.setSolution(current);
            }
        }

        iter++;
    }

    // -------------------- FINAL OUTPUT --------------------
    if (verbose) {
        std::cout << "\n===== FINAL AILS RESULT =====\n";
        std::cout << "Iterations: " << iter << "\n";
        std::cout << "Best objective: " << best.computeObjective() << "\n";
        std::cout << "Feasible: " << (best.isFeasible() ? "YES" : "NO") << "\n";

        for (int i = 0; i < static_cast<int>(perturbation_count.size()); ++i) {
            std::cout << "Perturbation: " << i
                      << " | Count: " << perturbation_count[i]
                      << " | Success: " << perturbation_success[i]
                      << "\n";
        }
    }

    return best;
}