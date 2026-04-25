# ---- compiler ----
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -MMD -MP

# ---- gurobi ----
GUROBI_HOME = /home/pc-515/gurobi1301/linux64
GUROBI_INC = -I$(GUROBI_HOME)/include
GUROBI_LIB = -L$(GUROBI_HOME)/lib -lgurobi_c++ -lgurobi130

export LD_LIBRARY_PATH := $(GUROBI_HOME)/lib:$(LD_LIBRARY_PATH)

BIN_DIR = bin
UTIL_DIR = util
META_DIR = metaheuristic
TEST_DIR = test
EXACT_DIR = exact

UTIL_SOURCES = $(wildcard $(UTIL_DIR)/*.cpp)
META_SOURCES = $(wildcard $(META_DIR)/*.cpp)
EXACT_SOURCES = $(wildcard $(EXACT_DIR)/*.cpp)
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)

UTIL_OBJECTS = $(patsubst $(UTIL_DIR)/%.cpp,$(BIN_DIR)/%.o,$(UTIL_SOURCES))
META_OBJECTS = $(patsubst $(META_DIR)/%.cpp,$(BIN_DIR)/meta_%.o,$(META_SOURCES))
EXACT_OBJECTS = $(patsubst $(EXACT_DIR)/%.cpp,$(BIN_DIR)/exact_%.o,$(EXACT_SOURCES))
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%.test.o,$(TEST_SOURCES))

TARGETS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SOURCES))

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ---- util objects ----
$(BIN_DIR)/%.o: $(UTIL_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- metaheuristic objects ----
$(BIN_DIR)/meta_%.o: $(META_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- exact objects ----
$(BIN_DIR)/exact_%.o: $(EXACT_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- test objects ----
$(BIN_DIR)/%.test.o: $(TEST_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUROBI_INC) -c $< -o $@

# ---- link ----
$(BIN_DIR)/%: $(BIN_DIR)/%.test.o $(UTIL_OBJECTS) $(META_OBJECTS) $(EXACT_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUROBI_LIB) -Wl,-rpath,$(GUROBI_HOME)/lib

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean

-include $(BIN_DIR)/*.d