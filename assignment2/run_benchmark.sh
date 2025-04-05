#!/bin/bash

RANGE_VALUES="1-1000 50000000-100000000 1000000000-1100000000"
out_dir="./out"
scheduling_policy=("d" "s" "t")
chunk_size=("8" "32" "512" "1024" "5000" "10000")
num_thread=("4" "16" "32" "40")
NUM_RUNS=2
CSV_FILE="./out/results.csv"

mkdir -p "$out_dir"

# Header CSV
echo "target,policy,chunk_size,num_threads,ranges, run1,run2" > "$CSV_FILE"

for policy in "${scheduling_policy[@]}"; do
  for c in "${chunk_size[@]}"; do
    for n in "${num_thread[@]}"; do
        csv_line="collatz_par, $policy,$c,$n,$RANGE_VALUES"
        echo "Running with -$policy -c $c -n $n $RANGE_VALUES"
        for ((i = 1; i <= NUM_RUNS; i++)); do
          output=$(./collatz_par -$policy -c $c -n $n $RANGE_VALUES 2>/dev/null)
          current_run_time=$(echo "$output" | sed 's/.*: \(.*\)s/\1/')
          csv_line="$csv_line,$current_run_time"
        done
        echo "$csv_line" >> "$CSV_FILE"
    done
  done
done


csv_line="collatz_seq, none, none, none,$RANGE_VALUES"
echo "Running collatz_seq $RANGE_VALUES"
for ((i = 1; i <= NUM_RUNS; i++)); do
  output=$(./collatz_seq $RANGE_VALUES 2>/dev/null)
  current_run_time=$(echo "$output" | sed 's/.*: \(.*\)s/\1/')
  csv_line="$csv_line,$current_run_time"
done
echo "$csv_line" >> "$CSV_FILE"
