#!/bin/bash
SAMPLE_PLUGIN=SAMPLE_PLUGIN

# make sure user gives a plugin name
if [ $# -lt 1 ] ;then
 echo "syntax: $0 <new plugin name>"
  exit 1
fi

# make sure it doesn't already exist
if [ -d "./$1" ] ; then
    echo "ERROR: $1 already exists"
    exit 1
fi

# copy the template
#echo cp -pR ./$SAMPLE_PLUGIN ./$1
cp -pR ./$SAMPLE_PLUGIN ./$1

# Don't copy junk
if [ -d "$1/CVS" ] ; then
    #echo "rm -r $1/CVS"
    rm -rf $1/CVS
fi

# Don't copy .svn dir
if [ -d "$PATH_TO_HERE/$ARG1/.svn" ] ; then
    echo "rm -rf $PATH_TO_HERE/$ARG1/.svn"
    rm -rf $PATH_TO_HERE/$ARG1/.svn
fi

# replace $SAMPLE_PLUGIN within files
#echo "find $1 -type f -exec perl -pi -e \"s/$SAMPLE_PLUGIN/$1/g\" '{}' \;"
find $1 -type f -exec perl -pi -e "s/$SAMPLE_PLUGIN/$1/g" '{}' \;

# rename files
for file in $1/*$SAMPLE_PLUGIN* $1/.deps/*$SAMPLE_PLUGIN* ;do
 mv $file ${file//$SAMPLE_PLUGIN/$1}
done

echo "---"
echo "New plug-in '$1' is ready."
echo ""
echo "To add $1 to the build system, you need to edit two files:"
echo "  Edit plugins/Makefile.am and add a line for your plugin to the SUBDIRS list"
echo "  Edit configure.ac and add a line for the plugins/$1/Makefile near the end"
echo ""
echo "You then need to rerun autogen.sh and configure just once to enable your"
echo "new plugin with the build system."
echo ""
