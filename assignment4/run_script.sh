#!/bin/bash
#SBATCH --job-name=mpi_merge_sort
#SBATCH --nodes=6
#SBATCH --ntasks=16
#SBATCH --time=00:20:00
#SBATCH --output=output.txt

make mpi_merge_sort
make std_sort
srun --mpi=pmix -n 16 -N 6 ./mpi_merge_sort -r 1 -t 32 -s 300M
srun ./std_sort -r 1 -s 300M