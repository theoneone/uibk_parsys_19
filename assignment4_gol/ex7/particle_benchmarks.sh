#!/bin/bash

mkdir -p exec{1..3}/th{1,2,4,8}
mkdir -p exec{1..3}/sz{10_10,10_100,10_1k,100_10,100_100,100_1k,1k_10,1k_100}
for i in {1..3}
do
	# parameter studies number of threads
	cd exec$i/th1
	qsub -pe openmp 1 ../../particle.script
	cd ../th2
	qsub -pe openmp 2 ../../particle.script
	cd ../th4
	qsub -pe openmp 4 ../../particle.script
	cd ../th8
	qsub -pe openmp 8 ../../particle.script

	# parameter studies problem size
	cd ../sz10_10
	qsub -pe openmp 8 ../../particle.script -c 10 -s 10
	cd ../sz10_100
	qsub -pe openmp 8 ../../particle.script -c 10 -s 100
	cd ../sz10_1k
	qsub -pe openmp 8 ../../particle.script -c 10 -s 1000
	cd ../sz100_10
	qsub -pe openmp 8 ../../particle.script -c 100 -s 10
	cd ../sz100_100
	qsub -pe openmp 8 ../../particle.script -c 100 -s 100
	cd ../sz100_1k
	qsub -pe openmp 8 ../../particle.script -c 100 -s 1000
	cd ../sz1k_10
	qsub -pe openmp 8 ../../particle.script -c 1000 -s 10
	cd ../sz1k_100
	qsub -pe openmp 8 ../../particle.script -c 1000 -s 100
	cd ../..
done

