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

# copy the template
echo "cp -pR $PATH_TO_HERE/$SAMPLE_PLUGIN $PATH_TO_HERE/$1"
cp -pR "$PATH_TO_HERE/$SAMPLE_PLUGIN" "$PATH_TO_HERE/$ARG1"
if [ $? != 0 ] ; then
    echo "ERROR: copy of $SAMPLE_PLUGIN failed"
    exit 1
fi

# Don't copy CVS dir
if [ -d "$PATH_TO_HERE/$ARG1/CVS" ] ; then
    echo "rm -rf $PATH_TO_HERE/$ARG1/CVS"
    rm -rf $PATH_TO_HERE/$ARG1/CVS
fi

# Don't copy .svn dir
if [ -d "$PATH_TO_HERE/$ARG1/.svn" ] ; then
    echo "rm -rf $PATH_TO_HERE/$ARG1/.svn"
    rm -rf $PATH_TO_HERE/$ARG1/.svn
fi

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

# update the Makefile.am
expression="s/(.*SAMPLE_PLUGIN.*)/\1\n\t$ARG1 \\\\/"
echo "perl -pi -e '$expression' $PATH_TO_HERE/Makefile.am"
perl -pi -e "$expression" "$PATH_TO_HERE/Makefile.am"
if [ $? != 0 ] ; then
    echo "ERROR: Update of Makefile.am failed"
    exit 1
fi

echo "---"
echo "New plug-in \"$ARG1\" is ready.  A directory for your plug-in was created."
echo ""
echo "To add $1 to the build system, you need to edit two files:"
echo "  Edit plugins/Makefile.am and add a line for your plugin to the SUBDIRS list"
echo "  Edit configure.ac and add a line for the plugins/$1/Makefile near the end"
echo ""
echo "You then need to rerun autogen.sh and configure just once to enable your"
echo "new plugin with the build system."
echo ""
echo "Get started coding here: $PATH_TO_HERE/$ARG1"
echo ""
