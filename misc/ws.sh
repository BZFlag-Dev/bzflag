#!/bin/sh

# Note: this script will not alter any files that already
# meet the BZFlag whitespace usage guidelines.
# if your files are being messed up by this, then chances
# are they are _already_ messed up when others look at them
# ie: you are now seeing what others see when they look at your
# messed up whitespace. If you find that this script can be
# improved to help your files be more compliant with the
# BZFlag whitespace guidelines, feel free to improve the script
# it will soon be run automatically from time to time, so
# if it messes up your code, then either fix it, or fix your code.
# Feel free to send specific examples to Tim Riker and he'll try
# to improve the script.
#	Thanx! Tim "ws king" Riker

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
