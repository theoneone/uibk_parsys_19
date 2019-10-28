#!/bin/bash

./heat_stencil_2D_seq "$1" 1>./temp"$1".tar
./tar2gif.sh temp"$1".tar
rm -r frames temp"$1".tar
mv heat.gif heat_stencil_2D_seq_"$1".gif
