# Assignment 1, due October 16th 2019

## Exercise 1

### Tasks

- Study how to submit jobs in SGE, how to check their state and how to cancel them.
- Prepare a submission script that starts an arbitrary executable, e.g. `/bin/hostname`
- In your opionion, what are the 5 most important parameters available when submitting a job and why? What are possible settings of these parameters, and what effect do they have?
- How do you run your program in parallel? What environment setup is required?

Most of this was done during the last proseminar and is outlined on the slides.

Job submission and status checking was performed as follows:

	$ squb <scriptfile>
	$ qstat

The `/bin/hostname` job script was modified to look like this:

	#!/bin/bash

	# Execute job in the queue "std.q" unless you have special requirements.
	#$ -q std.q

	# The batch system should use the current directory as working directory.
	#$ -cwd

	# Name your job. Unless you use the -o and -e options, output will
	# go to a unique file name.ojob_id for each job.
	#$ -N my_test_job

	# Redirect output stream to this file.
	#$ -o output.dat

	# Join the error stream to the output stream.
	#$ -j yes

	#$ -pe openmpi-2perhost 8

	module load openmpi/4.0.1
	mpiexec -n 8 /bin/hostname

The parameters specified in the script could also be given on the qsub
command line.

## Exercise 2

This exercise consists in running an MPI microbenchmark in order to examine the
impact of HPC topologies on performance.

### Description

The OSU Micro-Benchmarks suite holds multiple benchmarks that measure low-level
performance properties such as latency and bandwidth between MPI ranks.
Specifically, for this exercise, we are interested in the *point-to-point*
ones, which exchange messages between 2 MPI ranks.

### Tasks

- Download and build the OSU Micro-Benchmarks available at http://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-5.6.2.tar.gz. You can also use available binaries on LCC2 at `/scratch/c703429/osu-benchmark/libexec/osu-micro-benchmarks/mpi/pt2pt` (built with `openmpi/4.0.1`). Note: If you build yourself, do not forget to set the compiler parameters for `configure`, e.g. `./configure CC=mpicc CXX=mpic++ ...`
- After building, submit SGE jobs that run the `osu_latency` and `osu_bw` executables.
- Create a table and figures that illustrate the measured data and study them. What effects can you observe?
- Modify your experiment such that the 2 MPI ranks are placed on
    - different cores of the same socket,
    - different sockets of the same node, and
    - different nodes.
- Amend your table and figures to include these additional measurements. What effects can you observe? How can you verify rank placement without looking at performance?
- How stable are the measurements when running the experiments multiple times?


Getting available configurations:

[cb761030@login.lcc2 ~]$ qconf -spl
make
mpi
openmp
openmpi-1perhost
openmpi-2perhost
openmpi-4perhost
openmpi-8perhost
openmpi-fillup
smp



# OSU MPI Latency Test v5.6.2


$ -pe openmpi-1perhost 2

# Size          Latency (us)
0                       3.50
1                       3.54
2                       3.54
4                       3.55
8                       3.60
16                      3.65
32                      3.67
64                      3.82
128                     4.82
256                     5.35
512                     6.18
1024                    7.42
2048                    9.96
4096                   12.59
8192                   18.46
16384                  26.21
32768                  36.74
65536                  57.64
131072                100.43
262144                187.23
524288                357.93
1048576               698.79
2097152              1379.46
4194304              2740.45


$ -pe openmpi-2perhost 2

# Size          Latency (us)
0                       0.41
1                       0.46
2                       0.46
4                       0.46
8                       0.46
16                      0.47
32                      0.48
64                      0.52
128                     0.55
256                     0.59
512                     0.88
1024                    1.02
2048                    1.32
4096                    4.07
8192                    4.99
16384                   6.71
32768                   9.32
65536                  14.74
131072                 25.05
262144                 45.81
524288                 87.76
1048576               178.18
2097152              1299.89
4194304              2895.62






# OSU MPI Bandwidth Test v5.6.2

$ -pe openmpi-1perhost 2

# Size      Bandwidth (MB/s)
1                       4.09
2                       8.31
4                      16.50
8                      32.89
16                     58.59
32                    118.81
64                    201.53
128                   358.09
256                   613.55
512                  1333.89
1024                 2451.41
2048                 4095.29
4096                 1551.21
8192                 2391.92
16384                3093.03
32768                4131.71
65536                4948.74
131072               5543.93
262144               5920.97
524288               6113.73
1048576              5884.94
2097152              5691.76
4194304              1617.76


$ -pe openmpi-2perhost 2

# Size      Bandwidth (MB/s)
1                       0.63
2                       1.27
4                       2.53
8                       5.11
16                     10.25
32                     20.49
64                     40.69
128                    77.81
256                   153.04
512                   295.16
1024                  530.52
2048                  752.95
4096                  923.51
8192                 1062.20
16384                1108.45
32768                1303.20
65536                1421.70
131072               1484.27
262144               1502.51
524288               1519.37
1048576              1529.24
2097152              1535.33
4194304              1537.88
