# Makefile for RectangularMembrane Unit Test

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g -O0
INCLUDE_DIRS = -I../include -I../dependencies
SRC_DIR = ../src
TEST_DIR = .

# Source files
SOURCES = $(SRC_DIR)/RectangularMembrane.cc

TEST_SOURCES = $(TEST_DIR)/rectangularMembraneUnitTest.cc

OBJECTS = $(SOURCES:.cpp=.o) $(SOURCES:.c=.o) $(TEST_SOURCES:.cc=.o)

TARGET = $(TEST_DIR)/test_rectangular_membrane

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -o $@ $^
	@echo "Build complete: $@"

# Compile C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile C source files
%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile C++ test files
%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Run the test
run: $(TARGET)
	$(TARGET) ./output.bin

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all run clean
