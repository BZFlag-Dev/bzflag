#!/bin/bash
# cleanup whitespace issues
# could use new sed, but not everybody has that yet
# sed -i -e 's/search/replace/g' filename1 ...
# perl -pi -e 's/search/replace/g;' filename1 ...
# grep them first to not touch the file date/time
files=`find . -name \*.cxx -o -name \*.h -o -name \*.cpp -o -name \*.c -o -name Makefile.am | sort`
# convert 8 spaces to tab
for file in $files ; do
 # don't actually include 8 spaces or they might get replaced. ;-)
 # that's a space in the []
 if grep -q '[ ]       ' $file ; then
  #sed -i -e 's/[ ]       /\t/g' $file
  perl -pi -e 's/[ ]       /\t/g' $file
 fi
done
files=`find . -name \*.cxx -o -name \*.h -o -name \*.cpp -o -name \*.c -o -name Makefile.am -o -name README\* -o -name \*.dsp -o -name \*.fmt | sort`
# remove trailing whitespace and convert spacetab to tab
for file in $files ; do
 # that's a tab in the []
 if grep -q ' [	]' $file ; then
  #sed -i -e 's/ \t/\t/g' $file
  perl -pi -e 's/ *\t/\t/g' $file
 fi
 if grep -q '[[:space:]][[:space:]]*$' $file ; then
  #sed -i -e 's/[[:space:]]*$//' $file
  perl -pi -e 's/\s*$/\n/' $file
 fi
done
echo files with trailing whitespace:
grep -Irsl '[[:space:]][[:space:]]*$' . | grep -v Makefile$
