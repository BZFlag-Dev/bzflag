#!/bin/sh
# script to prepare bzflag sources
aclocal
autoheader
automake --add-missing
autoconf

if [ -z "$1" ] ; then
 echo BZFlag sources are now prepared. To build here, run:
 echo " ./configure --enable-robots"
 echo " make"
else
 ./configure --enable-robots
 make
fi
