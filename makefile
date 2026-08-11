SRCS = $(wildcard src/*.cpp)
CXX = g++
CXXFLAGS = -std=c++17 -O2
LDFLAGS = -lraylib

TARGET = ECNES

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
