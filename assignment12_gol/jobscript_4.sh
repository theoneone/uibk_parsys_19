#!/bin/bash

#$ -q std.q
#$ -cwd
#$ -N goliath_pi_4
#$ -o goliath_pi_4.dat
#$ -j yes
#$ -pe openmp 4
#$ -l excl=1

export CHPL_RT_NUM_THREADS_PER_LOCALE=4
time ./pi
