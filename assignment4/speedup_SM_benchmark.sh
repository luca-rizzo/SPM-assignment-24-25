#!/bin/bash
#SBATCH --job-name=shared_mem_speedup
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=00:30:00
#SBATCH --nodelist=node04
#SBATCH --exclusive
#SBATCH --output=./out/shared_mem_speedup_log.txt

cd ./include/ff || exit

echo y | ./mapping_string.sh

cd ../..

make cleanall
make ff_merge_sort
make std_sort
mkdir -p ./out
echo "Running on node: $(hostname)"

# === Global variables ===
export CSV_FILE="./out/shared_mem_speedup.csv"
export NUM_RUNS=2

echo "target,num_threads,dataset_size,time1,time2" > "$CSV_FILE"

run_and_measure() {
  local target="$1"
  local num_threads="$2"
  local dataset="$3"
  local cmd="$4"

  echo "Running: $cmd"
  local csv_line="$target,$num_threads,$dataset"

  for ((i = 1; i <= NUM_RUNS; i++)); do
    output=$(eval "$cmd" 2>>./out/shared_mem_speedup_log.txt)
    current_run_time=$(echo "$output" | grep -i "elapsed time" | sed 's/.*: \(.*\)s/\1/')
    csv_line="$csv_line,$current_run_time"
  done

  echo "$csv_line" >> "$CSV_FILE"
}



for DATASET_SIZE in 10K 100K 1M 50M 300M; do
  # ff_merge_sort varying number of threads
  for t in 2 4 8 16 32; do
    run_and_measure "ff_merge_sort" "$t" "$DATASET_SIZE" "./ff_merge_sort -s $DATASET_SIZE -t $t -r 1"
  done
  # std_sort
  run_and_measure "std_sort" 1 "$DATASET_SIZE" "./std_sort -s $DATASET_SIZE -t 1 -r 1"
done