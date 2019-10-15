# Assignment 1, due October 16th 2019

The goal of this assignment is to get you acquainted with working on a distributed memory cluster as well as obtaining, illustrating, and interpreting measurement data.

## Exercise 2

This exercise consists in running an MPI microbenchmark in order to examine the impact of HPC topologies on performance.

### Description

The OSU Micro-Benchmarks suite holds multiple benchmarks that measure low-level performance properties such as latency and bandwidth between MPI ranks. Specifically, for this exercise, we are interested in the *point-to-point* ones, which exchange messages between 2 MPI ranks.

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

