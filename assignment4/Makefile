CXX	= g++ -std=c++20
CXX_MPI = mpicxx -std=c++20
OPTIMIZE_FLAGS  += -O3 -ffast-math
CXXFLAGS += -Wall
LIBS = -pthread
INCLUDES = -I ./include
SOURCES              = $(wildcard *.cpp)
TARGET               = std_sort mpi_merge_sort ff_merge_sort

.PHONY: all clean cleanall 
.SUFFIXES: .c .cpp .o

mpi_merge_sort: mpi_merge_sort.cpp ./include/ff_MergeSort_Map.hpp
	$(CXX_MPI) $(INCLUDES) $(CXXFLAGS)  $(OPTIMIZE_FLAGS) -o $@ $< $(LIBS)

ff_merge_sort: ff_merge_sort.cpp ./include/ff_MergeSort_Map.hpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTIMIZE_FLAGS) -o $@ $< $(LIBS)

%: %.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTIMIZE_FLAGS) -o $@ $< $(LIBS)

all: $(TARGET)

cleanall: clean
	-rm -fr $(TARGET)
