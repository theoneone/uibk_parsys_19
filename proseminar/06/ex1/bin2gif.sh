#!/bin/bash

# convert *.bin files to animated gif

for f in step_*.bin; do
	png="$(basename $f .bin).png";
	./bin2png < $f > $png;
done

ffmpeg -framerate 100 -i step_%06d.png particle.gif

rm *.png

