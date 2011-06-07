#!/bin/sh
#
#  Run this script from the top level of the BZFlag source tree.
#  (ex: ./misc/indent.sh). Excluded directories ar listed in the
#  misc/astyle.conf file.
#


cd `dirname "$0"`/..


ASTYLE_EXEC="./misc/astyle-bzflag"

MIN_MAJOR=2

MIN_MINOR=2


# Get the version

VERSION=`"$ASTYLE_EXEC" --version 2>&1`

if [ $? -ne 0 ]; then
  echo Could not find astyle.
  exit
fi
 
echo $VERSION


# Parse the version

MAJOR=`echo $VERSION | sed 's/^Artistic Style Version \([0-9]*\)\.[0-9]*$/\1/'`
MINOR=`echo $VERSION | sed 's/^Artistic Style Version [0-9]*\.\([0-9]*\)$/\1/'`

if [ "$MAJOR" = "$VERSION"  -o  "$MINOR" = "$VERSION" ]; then
  echo Error parsing the astyle version.
  exit
fi


# Check the version

if [ $MAJOR -lt $MIN_MAJOR ]; then
  echo Need astyle 2.02 or better.
  exit
elif [ $MAJOR -eq $MIN_MAJOR  -a  $MINOR -lt $MIN_MINOR ]; then
  echo Need astyle 2.02 or better.
  exit
fi


# It's go time!

"$ASTYLE_EXEC" --options=misc/astyle.conf -r '*.h' '*.c' '*.cpp'


################################################################################
