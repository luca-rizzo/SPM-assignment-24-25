#!/bin/bash

K_VALUES=(32 1024 10204 102040 1048576 1048580 1048583)
NUM_RUNS=5
CSV_FILE="./out/benchmark_results.csv"

if [ "$#" -eq 0 ]; then
  echo "Error: No target specified"
  exit 1
fi

for K in "${K_VALUES[@]}"; do
  echo "==========================================="
  echo " Running benchmarks with K=$K"
  echo "==========================================="
  for target in "$@"; do
      if [ ! -x "./$target" ]; then
        echo "Error: Executable ./$target not found or not executable!"
        continue
      fi
      csv_line="$target, $K"
      for ((i=1; i<=NUM_RUNS; i++)); do
        output=$(./"$target" "$K")
        current_run_time=$(echo "$output" | sed 's/.*: \(.*\)s/\1/')
        csv_line="$csv_line, $current_run_time"
        echo $output
      done
      echo "$csv_line" >> "$CSV_FILE"
      echo "-------------------------------------------"
  done
  echo ""
done
