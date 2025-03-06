CXX=g++
CXXFLAGS=-mavx2 -O3 -std=c++23 -D SIMD
TARGET=bin/main
ASMFILE=bin/main.s
SRCFILE=main.cpp

.PHONY: profile run-profile clean

all: $(TARGET)

$(TARGET): $(SRCFILE)
	$(CXX) $(CXXFLAGS) $(SRCFILE) -o $(TARGET)

$(ASMFILE): $(SRCFILE)
	$(CXX) $(CXXFLAGS) -S $(SRCFILE) -o $(ASMFILE) 

profile: all run-profile

run-profile:
	rm -f callgrind.out.*
	valgrind --tool=callgrind --callgrind-out-file=./bin/callgrind.out $(TARGET) 

clean:
	rm -f $(TARGET) $(ASMFILE)
