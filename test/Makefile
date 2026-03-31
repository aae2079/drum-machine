# Makefile for RectangularMembrane Unit Test

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -fopenmp
INCLUDE_DIRS = -I../include -I../dependencies
LIBS = -lportaudio
SRC_DIR = ../src
TEST_DIR = .

# Source files
SOURCES = $(SRC_DIR)/backend/RectangularMembrane.cc $(SRC_DIR)/backend/audioEngine.cc

TEST_SOURCES = $(TEST_DIR)/rectangularMembraneUnitTest.cc

OBJECTS = $(SOURCES:.cpp=.o) $(SOURCES:.c=.o) $(TEST_SOURCES:.cc=.o)

TARGET = $(TEST_DIR)/test_rectangular_membrane

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -o $@ $^ $(LIBS)
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
