/*
  simple matrix multiplication algorithm
 */

use CyclicDist;
use Random;

// configurable values
config const a_cols: int = 2552;
config const a_rows: int = 2552;
config const b_cols: int = 2552;
config const min_value: real = 10.0;
config const max_value: real = 100.0;
config const rand_seed: int = 0;
config const print_in: bool = false;
config const print_out: bool = false;

var a: [1..a_cols, a_rows] real;
var b: [1..b_cols, 1..a_cols] real;
var c: [1..b_cols, 1..a_rows] real;

fillRandom(a, rand_seed);
fillRandom(b, rand_seed);

// TODO for loop - calculation

// TODO if print_in then println(a); println(); println(b); println();

// TODO if print_out then println(c);

