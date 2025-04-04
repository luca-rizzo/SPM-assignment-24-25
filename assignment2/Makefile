CXX                = g++ -std=c++17
OPTFLAGS	   = -O3
AUTOFLAGS          = -march=native -ffast-math -funroll-loops
AVXFLAGS           = -mavx
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./include
LIBS               = #-pthread -fopenmp
SOURCES            = $(wildcard *.cpp)
TARGET             = $(SOURCES:.cpp=)

.PHONY: all clean cleanall diff_outputs launch_benchmark

%: %.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -o $@ $< $(LIBS)

# For files with _avx in their name, append flags to CXXFLAGS
%_avx: CXXFLAGS += ${AVXFLAGS}

# For files with _auto in their name, append flags to CXXFLAGS
%_auto: CXXFLAGS += ${AUTOFLAGS}

all: $(TARGET)

clean: 
	-rm -fr *.o *~
	-rm -fr ./out/*

cleanall: clean
	-rm -fr $(TARGET)

launch_benchmark: cleanall $(TARGET)
	./run_benchmark.sh