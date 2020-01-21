use Random;

config const n = 100000000,
             seed = 589494289;

var rs = new RandomStream(real, seed, parSafe=true);

var count = 0;
for i in 1..n do
    if (rs.getNext()**2 + rs.getNext()**2) <= 1.0 then
       count += 1;

writeln("Number of points    = ", n);
writeln("Random number seed  = ", seed);
writeln("Approximation of pi = ", count * 4.0 / n);
