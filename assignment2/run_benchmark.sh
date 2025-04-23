#!/bin/bash

RANGE_VALUES="1-1000 50000000-100000000 1000000000-1100000000"
out_dir="./out"
scheduling_policy=("d" "s" "t")
NUM_RUNS=3
CSV_FILE="./out/results.csv"

# Interesting combination: chunk_size,num_threads
interesting_combinations=(
  "8,4"
  "32,8"
  "512,16"
  "1024,16"
  "10000,40"
  "10000,16"
  "512,8"

  #chunk_size = 5000
  "5000,4"
  "5000,8"
  "5000,16"
  "5000,32"
  "5000,40"

  #num_threads = 32
  "8,32"
  "32,32"
  "512,32"
  "1024,32"
  "5000,32"
  "10000,32"
)

mkdir -p "$out_dir"

# Header CSV
echo "target,policy,chunk_size,num_threads,ranges,run1,run2,run3" > "$CSV_FILE"

# Loop su policy, e per ogni combinazione interessante
for policy in "${scheduling_policy[@]}"; do
  for combo in "${interesting_combinations[@]}"; do
    IFS=',' read -r c n <<< "$combo"

    csv_line="collatz_par,$policy,$c,$n,$RANGE_VALUES"
    echo "Running with -$policy -c $c -n $n $RANGE_VALUES"
    for ((i = 1; i <= NUM_RUNS; i++)); do
      output=$(./collatz_par -$policy -c $c -n $n $RANGE_VALUES 2>/dev/null)
      current_run_time=$(echo "$output" | sed 's/.*: \(.*\)s/\1/')
      csv_line="$csv_line,$current_run_time"
    done
    echo "$csv_line" >> "$CSV_FILE"

  done
done

# Run sequential version
csv_line="collatz_seq,none,none,none,$RANGE_VALUES"
echo "Running collatz_seq $RANGE_VALUES"
for ((i = 1; i <= NUM_RUNS; i++)); do
  output=$(./collatz_seq $RANGE_VALUES 2>/dev/null)
  current_run_time=$(echo "$output" | sed 's/.*: \(.*\)s/\1/')
  csv_line="$csv_line,$current_run_time"
done
echo "$csv_line" >> "$CSV_FILE"
