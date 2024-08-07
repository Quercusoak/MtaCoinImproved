CXX = g++
CXXFLAGS = -Wall -std=c++11 -static
LIBS = -lz

EXEC = server

SRC = server.cpp ../utils/utils.cpp
OBJ = $(SRC:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

# Compile all .cpp into .o
%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Clean rule to remove generated files
clean:
	rm -f $(OBJ) $(EXEC)

# Phony targets
.PHONY: all clean

