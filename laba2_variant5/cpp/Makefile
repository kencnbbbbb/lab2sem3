CXX = c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
OUT ?= ../laba2-cpp
BUILD ?= build
STR = $(wildcard str/*.cpp)
TASKS = $(wildcard tasks/*.cpp)
HEADERS = $(wildcard str/*.h tasks/*.h)
OBJECTS = $(patsubst %.cpp,$(BUILD)/%.o,$(STR) $(TASKS))
DEPS = $(OBJECTS:.o=.d)

all: $(OUT)

$(OUT): $(OBJECTS) $(BUILD)/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/test-cpp: tests/test_main.cpp $(OBJECTS) $(HEADERS)
	$(CXX) $(CXXFLAGS) tests/test_main.cpp $(OBJECTS) -o $@

test: $(BUILD)/test-cpp
	./$(BUILD)/test-cpp

clean:
	rm -rf $(BUILD)
	rm -f $(OUT)

-include $(DEPS) $(BUILD)/main.d
.PHONY: all test clean
