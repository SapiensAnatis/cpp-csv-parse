CXX=g++
CXXFLAGS=-mavx2 -g -pg -O3 -std=c++23 -D SIMD
TARGET=bin/main
ASMFILE=bin/main.s
SRCFILE=main.cpp

all: $(TARGET)

$(TARGET): $(SRCFILE)
	$(CXX) $(CXXFLAGS) $(SRCFILE) -o $(TARGET)

asm: $(SRCFILE)
	$(CXX) $(CXXFLAGS) -S $(SRCFILE) -o $(ASMFILE) 

.PHONY: profile
profile: all run-profile

.PHONY: run-profile
run-profile:
	rm -f callgrind.out.*
	valgrind --tool=callgrind --callgrind-out-file=./bin/callgrind.out $(TARGET) 

.PHONY: clean
clean:
	rm -f $(TARGET) $(ASMFILE)
