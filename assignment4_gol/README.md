# Particle Simulation

This directory contains the source for 2 programs, one called `particle` and
one called `bin2png`. The former runs a randomized particle simulation that
can be parameterized using command line flags. Simply run `particle --help`
on how to change the defaults.

The `particle` program will produce a binary dump for each time step, so you
should run it in from a different working directory to not clog everything
up here.

The `bin2png` program can convert the numerous binary dumps that the `particle`
program produces into PNG files. The binary is read from `stdin`, the picture
is written to `stdout`. It too has a `--help` option.

The particles are colored from green to red depending on their mass.

## Compile

To compile it, simply run `make`. You will need `zlib` to for `bin2png`.

## Converting all the Dumps

The simplest way to convert all the output dumps is as follows:

	for f in step_*.bin; do
		png="$(basename $f .bin).png";
		bin2png < $f > $png;
	done
