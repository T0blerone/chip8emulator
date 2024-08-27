# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++14 -Iinclude -g

# Paths and Libraries
LDFLAGS := -Llib
LDLIBS := -lSDL2-2.0.0

# Source and Target
SRC := main.cpp chip8.cpp platform.cpp
TARGET := main

# Default rule
all: $(TARGET)

# Link object files to create the target executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

# Clean rule to remove the built executable
clean:
	rm -f $(TARGET)

# Phony targets to prevent make from confusing them with files
.PHONY: all clean
