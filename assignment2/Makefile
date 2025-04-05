CXX                = g++ -std=c++17
OPTFLAGS	   = -O3
AUTOFLAGS          = -march=native -ffast-math -funroll-loops
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./include
LIBS               = #-pthread -fopenmp
SOURCES            = $(wildcard *.cpp)
TARGET             = $(SOURCES:.cpp=)

.PHONY: all clean cleanall diff_outputs launch_benchmark

%: %.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) $(AUTOFLAGS) -o $@ $< $(LIBS)

all: $(TARGET)

clean: 
	-rm -fr *.o *~
	-rm -fr ./out/*

cleanall: clean
	-rm -fr $(TARGET)

launch_benchmark: cleanall $(TARGET)
	./run_benchmark.sh

launch_simple_benchmark: cleanall $(TARGET)
	./run_simple_benchmark.sh