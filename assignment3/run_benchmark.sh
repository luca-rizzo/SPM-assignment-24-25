#!/bin/bash

make compile_all
make clean_test_file

./create_test_file.sh
OMP_NUM_THREADS=32 ./minizpar -r 1 -C 0 ./test_file/many_small_files
OMP_NUM_THREADS=32 ./minizpar -r 1 -C 0 ./test_file/few_big_file

make clean_compression_test_file

./minizseq -r 1 -C 0 ./test_file/many_small_files
./minizseq -r 1 -C 0 ./test_file/few_big_file
make clean_test_file