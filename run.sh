#!/bin/bash
#
#SBATCH --job-name=conflict
#
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=96:00:00
#SBATCH --mem-per-cpu=1G
#
#SBATCH --array=0-8
#
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err


srun ./jobs-$SLURM_ARRAY_TASK_ID.sh