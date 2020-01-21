/*
  simple matrix multiplication algorithm
 */

use BlockDist;
use Random;
use Time;

// configurable values
config const a_cols: int = 2552;
config const a_rows: int = 2552;
config const b_cols: int = 2552;
config const rand_seed: int = 0;
config const print_in: bool = false;
config const print_out: bool = false;

var a: [1..a_rows, 1..a_cols] real;
var b: [1..a_cols, 1..b_cols] real;
var c: newBlockArr({1..a_rows, 1..b_cols}, real);
var t: Timer;

t.start();
fillRandom(a, rand_seed);
fillRandom(b, rand_seed);
t.stop();
writeln("Initialization time: ", t.elapsed(), " s");

t.clear();
t.start();
var tmp: real;
coforfor (i, j) in c do {
		tmp = 0;
		for k in 1..a_cols do {
			tmp += a[i, k] * b[k, j];
		}
		c[i, j] = tmp;
	}
}
t.stop();
writeln("Computation time: ", t.elapsed(), " s");

if print_in then {
	writeln("a = \n", a, "\n");
	writeln("b = \n", b, "\n");
}

if print_out then {
	writeln("c = \n", c, "\n");
}

