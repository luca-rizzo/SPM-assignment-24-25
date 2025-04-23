#!/bin/bash

make compile_all
make clean_test_file

./create_test_file.sh
OMP_NUM_THREADS=32 ./par_ver/minizpar ./test_file/many_small_files
OMP_NUM_THREADS=32 ./par_ver/minizpar ./test_file/few_big_file

make clean_compression_test_file

./minizseq ./test_file/many_small_files
./minizseq ./test_file/few_big_file
make clean_test_file