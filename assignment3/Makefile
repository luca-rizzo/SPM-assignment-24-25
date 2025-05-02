CXX		= g++ -std=c++20
INCLUDES	= -I . -I miniz -I ./include
CXXFLAGS  	+= -Wall 

LDFLAGS 	= -pthread -fopenmp
OPTFLAGS	= -O3 -ffast-math -DNDEBUG

TARGETS		= minizseq minizpar
PAR_DEPENDENCIES = par_ver/par_utility.hpp par_ver/cmdline_param_parser.hpp par_ver/par_block_compressor.hpp par_ver/par_block_decompressor.hpp

.PHONY: all clean clean_exec clean_test_file clean_compression_test_file compile_all
.SUFFIXES: .cpp

example%: miniz/examples/example%.c
	$(CC) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< ./miniz/miniz.c $(LDFLAGS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< ./miniz/miniz.c $(LDFLAGS)

all	: $(TARGETS)

minizseq	: minizseq.cpp cmdline.hpp utility.hpp

minizpar	: par_ver/minizpar.cpp $(PAR_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< ./miniz/miniz.c $(LDFLAGS)

clean_test_file:
	rm -rf test_file

clean_compression_test_file:
	find test_file -type f -name "*.zip" -exec rm -f {} +

clean_exec:
	rm -f $(TARGETS)

clean: clean_exec clean_test_file
	\rm -f *.o *~

compile_all: clean $(TARGETS)

