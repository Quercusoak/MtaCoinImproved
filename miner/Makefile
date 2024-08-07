CXX = g++
CXXFLAGS = -Wall -std=c++11 -static
LIBS = -lz

SRC = miner.cpp utils.cpp
OBJ = $(SRC:.cpp=.o)

all: miner

miner: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

# Compile all .cpp into .o
%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Clean rule to remove generated files
clean:
	rm -f $(OBJ) miner

# Phony targets
.PHONY: all clean

