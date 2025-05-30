#!/bin/bash
#SBATCH --job-name=strong_scalability_mpi
#SBATCH --nodes=8
#SBATCH --ntasks=32
#SBATCH --time=00:30:00
#SBATCH --output=./out/strong_scalability_mpi_log.txt
#SBATCH --error=./out/strong_scalability_mpi_log.txt

make cleanall
make mpi_merge_sort
mkdir -p out

# === Global variables ===
export CSV_FILE="./out/strong_scalability_mpi.csv"
export NUM_RUNS=2

echo "target,num_nodes,num_processes,num_ff_threads,size,time1,time2" > "$CSV_FILE"

run_and_measure_mpi() {
  local num_nodes="$1"
  local num_processes="$2"
  local num_ff_threads="$3"
  local size="$4"
  local cmd="./mpi_merge_sort -s $size -t $num_ff_threads -r 1"

  echo "Running: srun -N $num_nodes -n $num_processes $cmd"
  local csv_line="mpi_merge_sort,$num_nodes,$num_processes,$num_ff_threads,$size"

  for ((i = 1; i <= NUM_RUNS; i++)); do
    output=$(eval srun --mpi=pmix -N "$num_nodes" -n "$num_processes" "$cmd" 2>./out/strong_scalability_mpi_log.txt)
    current_run_time=$(echo "$output" | grep -i "elapsed time" | sed 's/.*: \(.*\)s/\1/')
    csv_line="$csv_line,$current_run_time"
  done

  echo "$csv_line" >> "$CSV_FILE"
}

BASE_THREADS=32

for DATASET_SIZE in 10K 100K 1M 50M 300M; do
  # Strong scalability: fixed size but variable number of nodes
  for n in 1 2 4 8; do
    run_and_measure_mpi "$n" "$n" "$BASE_THREADS" "$DATASET_SIZE"
  done

  for n in 2 4; do
    num_processes=$((8 * n))
    run_and_measure_mpi 8 "$num_processes" "$BASE_THREADS" "$DATASET_SIZE"
  done

done