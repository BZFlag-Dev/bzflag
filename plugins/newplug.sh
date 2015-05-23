#!/bin/sh

NARG="$#"
ARGS="$*"
ARG1="$1"
PATH_TO_HERE="`dirname $0`"

SAMPLE_PLUGIN=SAMPLE_PLUGIN

# make sure user gives a plugin name
if [ $NARG -lt 1 ] ;then
    echo "syntax: $0 <new_plugin_name>"
    exit 1
elif [ $NARG -gt 1 ] ;then
    echo "syntax: $0 <new_plugin_name>"
    exit 1
fi

# make sure it doesn't already exist
if [ -d "$PATH_TO_HERE/$ARG1" ] ; then
    echo "ERROR: $ARG1 already exists, remove it or use a different name"
    exit 1
fi

# make sure the sample exists
if [ ! -d "$PATH_TO_HERE/$SAMPLE_PLUGIN" ] ; then
    echo "ERROR: $SAMPLE_PLUGIN seems to be missing..."
    exit 1
fi

# Create the target directory
mkdir "$PATH_TO_HERE/$ARG1"
if [ $? != 0 ] ; then
    echo "ERROR: mkdir failed"
    exit 1
fi

# copy the template files (Add new files as necessary)
for file in Makefile.am README.txt SAMPLE_PLUGIN.cpp SAMPLE_PLUGIN.def SAMPLE_PLUGIN.sln SAMPLE_PLUGIN.vcxproj ;do
    echo "cp $PATH_TO_HERE/$SAMPLE_PLUGIN/$file $PATH_TO_HERE/$ARG1"
    cp "$PATH_TO_HERE/$SAMPLE_PLUGIN/$file" "$PATH_TO_HERE/$ARG1"
    if [ $? != 0 ] ; then
        echo "cp $PATH_TO_HERE/$SAMPLE_PLUGIN/$file $PATH_TO_HERE/$ARG1 failed"
        exit 1
    fi
done

# replace $SAMPLE_PLUGIN within files
echo "find $ARG1 -type f -exec perl -pi -e \"s/$SAMPLE_PLUGIN/$ARG1/g\" '{}' \;"
find $PATH_TO_HERE/$ARG1 -type f -exec perl -pi -e "s/$SAMPLE_PLUGIN/$ARG1/g" '{}' \;
if [ $? != 0 ] ; then
    echo "ERROR: find failed"
    exit 1
fi

# rename files
for file in $PATH_TO_HERE/$ARG1/*$SAMPLE_PLUGIN* ;do
    echo "mv $file `echo $file | sed \"s/$SAMPLE_PLUGIN/$ARG1/\"`"
    mv $file `echo $file | sed "s/$SAMPLE_PLUGIN/$ARG1/"`
    if [ $? != 0 ] ; then
	echo "mv $file `echo $file | sed s/$SAMPLE_PLUGIN/$ARG1/` failed"
	exit 1
    fi
done

echo "---"
echo "New plug-in \"$ARG1\" is ready.  A directory for your plug-in was created."
echo ""

bash addToBuild.sh $ARG1