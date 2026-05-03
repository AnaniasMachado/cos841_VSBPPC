# ---- compiler ----
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -MMD -MP

# ---- gurobi ----
GUROBI_HOME = /home/pc-515/gurobi1301/linux64
GUROBI_INC = -I$(GUROBI_HOME)/include
GUROBI_LIB = -L$(GUROBI_HOME)/lib -lgurobi_c++ -lgurobi130 -lpthread -lm -ldl

export LD_LIBRARY_PATH := $(GUROBI_HOME)/lib:$(LD_LIBRARY_PATH)

BIN_DIR = bin
UTIL_DIR = util
META_DIR = metaheuristic
EXACT_DIR = exact
TEST_DIR = test
EXP_DIR = experiment

# ---- special runner with main() for irace ----
AILS_TARGET_MAIN = $(UTIL_DIR)/ails_target.cpp
AILS_TARGET_OBJECT = $(BIN_DIR)/ails_target.main.o
AILS_TARGET = $(BIN_DIR)/ails_target

# ---- sources ----
UTIL_SOURCES = $(filter-out $(AILS_TARGET_MAIN),$(wildcard $(UTIL_DIR)/*.cpp))
META_SOURCES = $(wildcard $(META_DIR)/*.cpp)

EXACT_MAIN = $(EXACT_DIR)/run_mip_solver.cpp
EXACT_SOURCES = $(filter-out $(EXACT_MAIN),$(wildcard $(EXACT_DIR)/*.cpp))

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
EXP_SOURCES = $(wildcard $(EXP_DIR)/*.cpp)

UTIL_OBJECTS = $(patsubst $(UTIL_DIR)/%.cpp,$(BIN_DIR)/%.o,$(UTIL_SOURCES))
META_OBJECTS = $(patsubst $(META_DIR)/%.cpp,$(BIN_DIR)/meta_%.o,$(META_SOURCES))
EXACT_OBJECTS = $(patsubst $(EXACT_DIR)/%.cpp,$(BIN_DIR)/exact_%.o,$(EXACT_SOURCES))

EXACT_MAIN_OBJECT = $(BIN_DIR)/exact_run_mip_solver.o

TEST_TARGETS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SOURCES))
EXP_TARGETS = $(patsubst $(EXP_DIR)/%.cpp,$(BIN_DIR)/%,$(EXP_SOURCES))
RUN_MIP_TARGET = $(BIN_DIR)/run_mip_solver

TARGETS = $(TEST_TARGETS) $(EXP_TARGETS) $(RUN_MIP_TARGET) $(AILS_TARGET)

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ---- util objects ----
$(BIN_DIR)/%.o: $(UTIL_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- metaheuristic objects (needs gurobi for set covering) ----
$(BIN_DIR)/meta_%.o: $(META_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- exact objects ----
$(BIN_DIR)/exact_%.o: $(EXACT_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- test objects ----
$(BIN_DIR)/%.test.o: $(TEST_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- experiment objects (also need gurobi!) ----
$(BIN_DIR)/%.exp.o: $(EXP_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- irace target object ----
$(AILS_TARGET_OBJECT): $(AILS_TARGET_MAIN) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- link tests ----
$(BIN_DIR)/%: $(BIN_DIR)/%.test.o $(UTIL_OBJECTS) $(META_OBJECTS) $(EXACT_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUROBI_LIB) -Wl,-rpath,$(GUROBI_HOME)/lib

# ---- link experiments (FIXED: now links gurobi) ----
$(BIN_DIR)/%: $(BIN_DIR)/%.exp.o $(UTIL_OBJECTS) $(META_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUROBI_LIB) -Wl,-rpath,$(GUROBI_HOME)/lib

# ---- link exact executable ----
$(RUN_MIP_TARGET): $(EXACT_MAIN_OBJECT) $(UTIL_OBJECTS) $(META_OBJECTS) $(EXACT_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUROBI_LIB) -Wl,-rpath,$(GUROBI_HOME)/lib

# ---- link irace target executable ----
$(AILS_TARGET): $(AILS_TARGET_OBJECT) $(UTIL_OBJECTS) $(META_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUROBI_LIB) -Wl,-rpath,$(GUROBI_HOME)/lib

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean

-include $(BIN_DIR)/*.d