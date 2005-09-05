#!/bin/bash
# Rename SAMPLE_PLUGIN here
SAMPLE_NAME=SAMPLE_PLUGIN
#
if [ $# -lt 1 ] ;then
 echo "syntax: $0 <new plugin name>"
  exit 1
fi
#
# copy
cp -a ./$SAMPLE_NAME ./$1
rm -r $1/CVS # Don't copy junk
#
# replace $SAMPLE_NAME within files
find $1 -type f -exec sed -i "s/$SAMPLE_NAME/$1/g" '{}' \;
#
# replace within filenames
for file in $1/*$SAMPLE_NAME* ;do
 mv $file ${file//$SAMPLE_NAME/$1}
done

echo "Edit configure.ac and add a line for the plugins/$1/Makefile"