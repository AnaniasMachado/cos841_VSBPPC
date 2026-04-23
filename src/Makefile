CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -MMD -MP

BIN_DIR = bin
UTIL_DIR = util
TEST_DIR = test

UTIL_SOURCES = $(wildcard $(UTIL_DIR)/*.cpp)
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)

UTIL_OBJECTS = $(patsubst $(UTIL_DIR)/%.cpp,$(BIN_DIR)/%.o,$(UTIL_SOURCES))
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%.test.o,$(TEST_SOURCES))

TARGETS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SOURCES))

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%.o: $(UTIL_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.test.o: $(TEST_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%: $(BIN_DIR)/%.test.o $(UTIL_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean

-include $(BIN_DIR)/*.d