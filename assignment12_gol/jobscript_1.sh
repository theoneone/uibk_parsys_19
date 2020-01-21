#!/bin/bash

#$ -q std.q
#$ -cwd
#$ -N goliath_pi_1
#$ -o goliath_pi_1.dat
#$ -j yes
#$ -pe openmp 1
#$ -l excl=1

export CHPL_RT_NUM_THREADS_PER_LOCALE=1
time ./pi
