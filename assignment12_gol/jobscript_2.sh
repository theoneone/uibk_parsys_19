#!/bin/bash

#$ -q std.q
#$ -cwd
#$ -N goliath_pi_2
#$ -o goliath_pi_2.dat
#$ -j yes
#$ -pe openmp 2
#$ -l excl=1

export CHPL_RT_NUM_THREADS_PER_LOCALE=2
time ./pi
