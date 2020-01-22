use Random;

config const n = 1000000000,
             seed = 589494289;

var rs = new RandomStream(real, seed, parSafe=true);

//
// This is a "domain", not an actual array. It contains a high level
// description of the range and does not actually allocate memory.
//
var DOM = {1..n};

//
// We generate the random points by creating two iterators from the random
// stream over the domain and zipping them together to form a sequence of
// touples.
//
// We reduce the list of points to a single integer by evaluating the
// expression in the second line and using the '+' reduction operator.
//
var count = + reduce [(x, y) in zip(rs.iterate(DOM), rs.iterate(DOM))]
            if x**2 + y**2 <= 1.0 then 1 else 0;

writeln("Number of points    = ", n);
writeln("Random number seed  = ", seed);
writeln("Approximation of pi = ", count * 4.0 / n);
