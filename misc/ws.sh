#!/bin/sh
# do NOT strip ws from the dsp files. loath M$
files=`find . -name \*.cxx -o -name \*.h -o -name \*.cpp -o -name Makefile.am -o -name README\* | sort`
for file in $files ; do
 sed -i -e 's/[[:space:]]*$//' $file
done
grep -Irsl '[[:space:]][[:space:]]*$' . | grep -v Makefile$
