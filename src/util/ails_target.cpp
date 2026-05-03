#include <iostream>
#include <string>
#include <climits>
#include <random>
#include <stdexcept>

#include "../util/vsbppc.hpp"
#include "../util/solution.hpp"
#include "../metaheuristic/ails.hpp"

// -------------------- Helper: parse instance set --------------------
InstanceSet parseInstanceSet(const std::string& s) {
    if (s == "SET1") return InstanceSet::SET1;
    if (s == "SET2") return InstanceSet::SET2;

    throw std::invalid_argument("Invalid InstanceSet: " + s);
}

// -------------------- Helper: parse cost type --------------------
CostType parseCostType(const std::string& s) {
    if (s == "LINEAR") return CostType::LINEAR;
    if (s == "CONCAVE") return CostType::CONCAVE;
    if (s == "CONVEX") return CostType::CONVEX;

    throw std::invalid_argument("Invalid CostType: " + s);
}

// -------------------- Helper: parse bin size setting --------------------
BinSizeSetting parseBinSizeSetting(const std::string& s) {
    if (s == "THREE_TYPES") return BinSizeSetting::THREE_TYPES;
    if (s == "FIVE_TYPES") return BinSizeSetting::FIVE_TYPES;
    if (s == "SEVEN_TYPES") return BinSizeSetting::SEVEN_TYPES;

    throw std::invalid_argument("Invalid BinSizeSetting: " + s);
}

// -------------------- Helper: parse builder --------------------
BuilderType parseBuilder(const std::string& s) {
    if (s == "IFFD") return BuilderType::IFFD;
    if (s == "IBFD") return BuilderType::IBFD;

    throw std::invalid_argument("Invalid BuilderType: " + s);
}

// -------------------- Helper: parse acceptance --------------------
AcceptanceType parseAcceptance(const std::string& s) {
    if (s == "BEST") return AcceptanceType::BEST;
    if (s == "ITERATIVE") return AcceptanceType::ITERATIVE;
    if (s == "RW") return AcceptanceType::RW;

    throw std::invalid_argument("Invalid AcceptanceType: " + s);
}

// -------------------- Helper: parse improvement --------------------
ImprovementType parseImprovement(const std::string& s) {
    if (s == "BI") return ImprovementType::BI;
    if (s == "FI") return ImprovementType::FI;

    throw std::invalid_argument("Invalid ImprovementType: " + s);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr
            << "Usage: " << argv[0] << " instance_file [options]\n\n"
            << "Options:\n"
            << "  --set_type SET1|SET2\n"
            << "  --cost_type LINEAR|CONCAVE|CONVEX\n"
            << "  --bin_size THREE_TYPE|FIVE_TYPES|SEVEN_TYPES\n"
            << "  --kw value\n"
            << "  --kc value\n"
            << "  --max_it value\n"
            << "  --max_no_imp value\n"
            << "  --acceptance BEST|ITERATIVE|RW\n"
            << "  --improvement BI|FI\n"
            << "  --builder IBFD\n"
            << "  --use_ucb 0|1\n"
            << "  --c value\n"
            << "  --use_qrvnd 0|1\n"
            << "  --alpha value\n"
            << "  --gamma value\n"
            << "  --epsilon value\n"
            << "  --exact_cover 0|1\n"
            << "  --verbose 0|1\n"
            << "  --time_limit value\n"
            << "  --seed value\n";

        return 1;
    }

    std::string instanceFile = argv[1];

    // -------------------- Default instance settings --------------------
    InstanceSet setType = InstanceSet::SET1;
    CostType costType = CostType::LINEAR;
    BinSizeSetting binSizeSetting = BinSizeSetting::THREE_TYPES;

    // -------------------- Default penalty parameters --------------------
    int kw = 400;
    int kc = 275;

    // -------------------- Default AILS parameters --------------------
    int maxIterations = 1000;
    int maxNoImprove = 25;

    AcceptanceType acceptance = AcceptanceType::BEST;
    ImprovementType improvement = ImprovementType::BI;

    BuilderType builder = BuilderType::IBFD;

    bool useUCB = false;
    double c = 0.5718;

    bool useQRVND = false;
    double alpha = 0.1495;
    double gamma = 0.7455;
    double epsilon = 0.0377;

    bool exactCover = false;

    bool verbose = false;
    double timeLimit = 600.0;

    int seed = 42;

    // -------------------- Parse optional arguments --------------------
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--set_type" && i + 1 < argc) {
            setType = parseInstanceSet(argv[++i]);
        }
        else if (arg == "--cost_type" && i + 1 < argc) {
            costType = parseCostType(argv[++i]);
        }
        else if (arg == "--bin_size" && i + 1 < argc) {
            binSizeSetting = parseBinSizeSetting(argv[++i]);
        }
        else if (arg == "--kw" && i + 1 < argc) {
            kw = std::stoi(argv[++i]);
        }
        else if (arg == "--kc" && i + 1 < argc) {
            kc = std::stoi(argv[++i]);
        }
        else if (arg == "--max_it" && i + 1 < argc) {
            maxIterations = std::stoi(argv[++i]);
        }
        else if (arg == "--max_no_imp" && i + 1 < argc) {
            maxNoImprove = std::stoi(argv[++i]);
        }
        else if (arg == "--acceptance" && i + 1 < argc) {
            acceptance = parseAcceptance(argv[++i]);
        }
        else if (arg == "--improvement" && i + 1 < argc) {
            improvement = parseImprovement(argv[++i]);
        }
        else if (arg == "--builder" && i + 1 < argc) {
            builder = parseBuilder(argv[++i]);
        }
        else if (arg == "--use_ucb" && i + 1 < argc) {
            useUCB = std::stoi(argv[++i]) != 0;
        }
        else if (arg == "--c" && i + 1 < argc) {
            c = std::stod(argv[++i]);
        }
        else if (arg == "--use_qrvnd" && i + 1 < argc) {
            useQRVND = std::stoi(argv[++i]) != 0;
        }
        else if (arg == "--alpha" && i + 1 < argc) {
            alpha = std::stod(argv[++i]);
        }
        else if (arg == "--gamma" && i + 1 < argc) {
            gamma = std::stod(argv[++i]);
        }
        else if (arg == "--epsilon" && i + 1 < argc) {
            epsilon = std::stod(argv[++i]);
        }
        else if (arg == "--exact_cover" && i + 1 < argc) {
            exactCover = std::stoi(argv[++i]) != 0;
        }
        else if (arg == "--verbose" && i + 1 < argc) {
            verbose = std::stoi(argv[++i]) != 0;
        }
        else if (arg == "--time_limit" && i + 1 < argc) {
            timeLimit = std::stod(argv[++i]);
        }
        else if (arg == "--seed" && i + 1 < argc) {
            seed = std::stoi(argv[++i]);
        }
    }

    // -------------------- Read VSBPPC instance --------------------
    VSBPPCInstance inst =
        readInstance(instanceFile, setType, costType, binSizeSetting);

    std::mt19937 rng(seed);

    // -------------------- Run AILS --------------------
    AILS ails(inst,
                kw, kc, rng,
                maxIterations,
                maxNoImprove,
                acceptance,
                improvement,
                useUCB,
                c,
                builder,
                useQRVND,
                alpha,
                gamma,
                epsilon,
                exactCover,
                verbose,
                timeLimit);

    Solution best = ails.run();

    // -------------------- Output for irace --------------------
    if (best.isFeasible()) {
        std::cout << best.computeObjective() << std::endl;
    } else {
        std::cout << INT_MAX << std::endl;
    }

    return 0;
}