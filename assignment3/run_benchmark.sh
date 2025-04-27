#!/bin/bash

OUT_DIR="./out"
CSV_FILE="$OUT_DIR/$1.csv"
NUM_RUNS=3
TEST_DIR="./test_file"
THREADS_LIST=(2 4 8 16 32 40)

mkdir -p "$OUT_DIR"

echo "Compiling all sources..."
make compile_all
make clean_test_file

./create_test_file.sh

echo "target,num_threads,dataset,run1,run2,run3" > "$CSV_FILE"

# Helper per eseguire e raccogliere i tempi
run_and_measure() {
  local target="$1"
  local num_threads="$2"
  local dataset="$3"
  local cmd="$4"

  local csv_line="$target,$num_threads,$dataset"
  echo "Running $cmd"
  for ((i = 1; i <= NUM_RUNS; i++)); do
    make clean_compression_test_file > /dev/null 2>&1
    output=$(eval "$cmd")
    current_run_time=$(echo "$output" | grep -i "elapsed time" | sed 's/.*: \(.*\)s/\1/')
    csv_line="$csv_line,$current_run_time"
  done
  echo "$csv_line" >> "$CSV_FILE"
}

for n_threads in "${THREADS_LIST[@]}"; do
  export OMP_NUM_THREADS="$n_threads"
  for dataset in large small mixed; do
    run_and_measure "minizpar" "$n_threads" "$dataset" "./minizpar -r 1 -C 0 $TEST_DIR/$dataset"
  done
  unset OMP_NUM_THREADS
done

for dataset in large small mixed; do
  run_and_measure "minizseq" "1" "$dataset" "./minizseq -r 1 -C 0 $TEST_DIR/$dataset"
done

echo "Cleaning all test files..."
make clean_test_file

echo "All benchmarks completed. Results saved in $CSV_FILE"
