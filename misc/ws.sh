#!/bin/sh
files=`find . -name \*.cxx -o -name \*.h -o -name \*.cpp -o -name \*.c -o -name Makefile.am | sort`
# convert 8 spaces to tab
for file in $files ; do
 sed -i -e 's/[ ][ ][ ][ ][ ][ ][ ][ ]$/\t/' $file
done
files=`find . -name \*.cxx -o -name \*.h -o -name \*.cpp -o -name \*.c -o -name Makefile.am -o -name README\* -o -name \*.dsp -o -name \*.fmt | sort`
# remove trailing whitespace and convert spacetab to tab
for file in $files ; do
 sed -i -e 's/[[:space:]]*$//' -e 's/ \t/\t/' $file
done
echo files with trailing whitespace:
grep -Irsl '[[:space:]][[:space:]]*$' . | grep -v Makefile$
