# Makefile for C++ Nuclear Reactor Simulator

# Compiler selection (auto-detect)
ifeq ($(OS),Windows_NT)
    CXX = g++
    EXECUTABLE = reactor.exe
    RM = del /Q
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        CXX = clang++
    else
        CXX = g++
    endif
    EXECUTABLE = reactor
    RM = rm -f
endif

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Source files
SOURCES = main.cpp Reactor_impl.cpp PhysicsEngine.cpp UI.cpp GameController.cpp RandomEvents.cpp

# Default target
all: $(EXECUTABLE)

# Build executable
$(EXECUTABLE): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXECUTABLE)
	@echo "Build complete! Run with: ./$(EXECUTABLE)"

# Debug build with symbols
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: clean $(EXECUTABLE)
	@echo "Debug build complete!"

# Clean build artifacts
clean:
	$(RM) $(EXECUTABLE)

# Run the simulator
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# Help target
help:
	@echo "Available targets:"
	@echo "  make        - Build the reactor simulator"
	@echo "  make run    - Build and run the simulator"
	@echo "  make debug  - Build with debug symbols"
	@echo "  make clean  - Remove build artifacts"
	@echo "  make help   - Show this help message"

.PHONY: all clean run debug help