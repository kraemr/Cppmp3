# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -lm -O2 -lraylib
# Source directory
SRC_DIR = ../src

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object directory
OBJ_DIR = .

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Executable name
TARGET = mp3player.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
