# Assignment 1, due October 16th 2019

The goal of this assignment is to get you acquainted with working on a distributed memory cluster as well as obtaining, illustrating, and interpreting measurement data.

## Exercise 1
### Tasks

#### Study how to submit jobs in SGE, how to check their state and how to cancel them.
- schedule job:

	`qsub job.script`

- check state:

	`qstat`

- delete job:

	`qdel job_ID-list`


#### Prepare a submission script that starts an arbitrary executable, e.g. `/bin/hostname`
e.g. job.script


#### In your opionion, what are the 5 most important parameters available when submitting a job and why? What are possible settings of these parameters, and what effect do they have?
- **Most important options**
	
	-pe parallel-environment number-of-slots: 
	Reserve number-of-slots CPU cores (number-of-slots may be a range min-max, if max cores are not available, then  SGE tries to run the job on a fewer number of cores, but not less then min #cores), available environments are :
		- openmpi-Xperhost number-of-slots - X processors per host (number-of-slots must be multiple of X)
		- openmpi-fillup number-of-slots - fill every host with processes to its host process limit
		- openmp - for threaded applications (OpenMP)
		
	-i path - path to the file to provide stdin data.
	
	-cwd - use the current workung directory (directory containing the script file) instead of the default $HOME.
	
	-t 1-n - Trivial parallelisation using a job array. Start n independent instances of your job (e.g. for extensive parameter studies). When the job is run, you use the environment variable $SGE_TASK_ID, which is set to a unique integer value from 1 .. n, to distinguish between the individual job instances (e.g. to initialize a random number generator, select an input file or compute parameter values).
	
	-w v - check whether the syntax of the job is okay (do not submit the job)
	
- **Maybe useful**
	
	-N name: name for the job. This might be useful because this name propagates to the output files if no -o and -e options are given. Default is file name of script.
	
- **Not reccommended or not useful**
	
	-q qname - select queue: This option may be useful with other system configurations than lcc2. Two alternatives are configured:
		- std.q: this is the default
		- all.q: undocumented, usage is discouraged
	
	
	-o opath - path to the output file. Discouraged, because of possible data loss. Default is name.ojob_id with unique job_id number. Use only if not more than one instance of the job is running
	
	
	-e epath - path to the error output file. Default is name.ejob_id.
	
	-j yes|no - join stderr to stdout in one file. Use with caution. Parsing the output file may become a pain in the ass when it is polluted with error messages.
	

#### How do you run your program in parallel? What environment setup is required?

compile the sources with options:

- `mpicc`

- `mpic++`


add following code lines into the job.script:

choose a rank configuration (see above) - option parsed by qsub:

`#$ -pe openmpi-Xperhost number-of-slots`

load the MPI environment immediately before the program call:

`module load openmpi/4.0.1`

call the program with mpiexec:

`mpiexec -n $NSLOTS path/to/executable`

