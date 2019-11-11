#!/bin/bash

# convert all equation.pdf to equation.svg

IN_DIR=./$1
if [ -e $IN_DIR ]
then
	echo $IN_DIR
else
	echo "Folder doesn't exist'"
fi

for FILE in  $(ls $IN_DIR/*.tex)
do
#	inkscape --without-gui --file=$FILE --export-plain-svg=`echo $FILE | sed 's/\.pdf/\.svg/'`
	latex2png -d 200 $FILE
done

