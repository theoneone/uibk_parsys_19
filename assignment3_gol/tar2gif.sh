#!/bin/sh

mkdir -p frames

tar xf "$1" -C frames

cd frames
convert -delay 10 frame_*.ppm heat.gif
mv heat.gif ..
