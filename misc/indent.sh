#!/bin/sh


MIN_MAJOR=2

MIN_MINOR=2


VERSION=`astyle --version 2>&1`

if [ $? -ne 0 ]; then
  echo Could not find astyle.
  exit
fi
 
echo $VERSION


MAJOR=`echo $VERSION | sed 's/^Artistic Style Version \([0-9]*\)\.[0-9]*$/\1/'`
MINOR=`echo $VERSION | sed 's/^Artistic Style Version [0-9]*\.\([0-9]*\)$/\1/'`

if [ "$MAJOR" = "$VERSION"  -o  "$MINOR" = "$VERSION" ]; then
  echo Error parsing the astyle version.
  exit
fi

if [ $MAJOR -lt $MIN_MAJOR ]; then
  echo Need astyle 2.02 or better.
  exit
elif [ $MAJOR -eq $MIN_MAJOR   -a  $MINOR -lt $MIN_MINOR ]; then
  echo Need astyle 2.02 or better.
  exit
fi


astyle --options=misc/astyle.conf -r '*.h' '*.c' '*.cpp'



