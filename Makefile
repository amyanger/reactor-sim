CXX      ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O2
CPPFLAGS += -Isrc -MMD -MP

SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)
LIB := $(filter-out build/main.o,$(OBJ))
TEST_SRC := $(wildcard tests/*.cpp)
TEST_OBJ := $(TEST_SRC:tests/%.cpp=build/tests/%.o)
TARGET := reactor

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/tests/%.o: tests/%.cpp | build/tests
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build build/tests:
	mkdir -p $@

build/run_tests: $(TEST_OBJ) $(LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

test: build/run_tests
	./build/run_tests

# Debug and sanitizer builds share the build/ dir, so clean first when switching.
debug: CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O0 -g
debug: $(TARGET)

asan: CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address,undefined
asan: $(TARGET) build/run_tests
	./build/run_tests

clean:
	rm -rf build $(TARGET)

-include $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

.PHONY: test debug asan clean
