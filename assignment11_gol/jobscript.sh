#!/bin/bash

#$ -N real_goliath
#$ -q std.q
#$ -cwd
#$ -j yes
#$ -pe openmp 8

export OMP_NUM_THREADS=$NSLOTS
./real
