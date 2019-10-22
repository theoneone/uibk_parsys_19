#!/bin/bash

#$ -q std.q
#$ -cwd
#$ -N goliath_pi_test
#$ -t 1-32
#$ -o pi_test.out
#$ -e pi_test.error

#$ -pe openmpi-fillup 32

./pi_test 1000000 >> pi_test_${SGE_TASK_ID}.csv
