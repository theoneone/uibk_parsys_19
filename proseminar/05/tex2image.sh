#!/bin/bash

# convert all equation.tex to equation.svg
#
# this script takes one input argument:
# IN_DIR: the directory, which contains tex files to convert
#
# depends on package latex2rtf (on ubuntu use command 'sudo apt install latex2rtf')
# you may need to install the package texlive-latex-extra

IN_DIR=./$1
if [ -e $IN_DIR ]
then
	echo "processing files in " $IN_DIR
else
	echo "Folder doesn't exist'"
fi

for FILE in  $(ls $IN_DIR/*.tex)
do
	latex2png -d 200 $FILE
done

