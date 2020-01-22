#!/bin/bash

#$ -q std.q
#$ -cwd
#$ -N goliath_pi_8
#$ -o goliath_pi_8.dat
#$ -j yes
#$ -pe openmp 8
#$ -l excl=1

export CHPL_RT_NUM_THREADS_PER_LOCALE=8
time ./pi
