CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
SRC = $(wildcard src/*.cpp)
TARGET = reactor

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
