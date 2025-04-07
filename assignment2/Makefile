CXX                = g++ -std=c++17
OPTFLAGS	   = -O3 -march=native -ffast-math
CXXFLAGS          += -Wall 
INCLUDES	   = -I. -I./include
TARGET = collatz_seq collatz_par
SCHED_OBJ = obj/block_cyclic_scheduling.o obj/dynamic_index_scheduling.o obj/dynamic_TP_scheduling.o
PARSE_OBJ = obj/parse_utility.o

.PHONY: clean cleanall diff_outputs launch_benchmark

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@

collatz_par: $(SCHED_OBJ) $(PARSE_OBJ) obj/collatz_par.o
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) $(AUTOFLAGS) -o $@ $^

collatz_seq: $(PARSE_OBJ) obj/collatz_seq.o
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(OPTFLAGS) $(AUTOFLAGS) -o $@ $^

clean:
	-rm -fr *.o *~
	-rm -fr ./obj/*.o *~
	-rm -fr ./out/*

cleanall: clean
	-rm -fr $(TARGET)

launch_benchmark: cleanall $(TARGET)
	./run_benchmark.sh