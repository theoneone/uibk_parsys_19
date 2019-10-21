# Monte Carlo Pi Approximation

The task of approximating pi is split into two parts:

- Producing random samples and evaluating if they are in a circle
- Summing up all the hit/sample ratios and calculating the final approximation

The first part is implemented by a program called `pi_test` and the second one
by a program called `pi_eval`. This has been done under the assumption that the
sampling will consume most of the time while the evaluation itself is trivial.

The `pi_test` program uses Linux `/dev/random` to produce random bytes which it
interprets as coordinate pairs in a 256 * 256 square and tests if they are
inside a quarter circle of radius 256, centered at the origin.

The resulting implementation of the sampling is straight forward and
embarrassingly parallel. Each instance of `pi_test` produces a single line of
a CSV file that has to be piped into `pi_eval` for serial processing of the
results (i.e. summing up the hits and sample counts).

To build the programs, simply run `make` in the source directory.
