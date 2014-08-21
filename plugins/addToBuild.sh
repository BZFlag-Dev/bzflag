#!/bin/sh

NARG="$#"
ARG1="${1%/}"
PATH_TO_HERE="`dirname $0`"

# make sure user gives a plugin name
if [ $NARG -lt 1 ] ; then
    echo "syntax: $0 <plugin_to_add>"
    exit 1
elif [ $NARG -gt 1 ] ; then
    echo "syntax: $0 <plugin_to_add>"
    exit 1
fi

# make sure it doesn't already exist
if [ ! -d "$PATH_TO_HERE/$ARG1" ] ; then
    echo "ERROR: $ARG1 does not exist, please use the newplug.sh script to create the plug-in instead."
    exit 1
fi

# update the Makefile.am
expression="s/(.*SAMPLE_PLUGIN.*)/\1\n\t$ARG1 \\\\/"
echo "perl -pi -e '$expression' $PATH_TO_HERE/Makefile.am"
perl -pi -e "$expression" "$PATH_TO_HERE/Makefile.am"
if [ $? != 0 ] ; then
    echo "ERROR: Update of Makefile.am failed"
    exit 1
fi

# update ../configure.ac
expression="s/(.*SAMPLE_PLUGIN.*)/\1\n\tplugins\\/$ARG1\\/Makefile/"
echo "perl -pi -e '$expression' $PATH_TO_HERE/../configure.ac"
perl -pi -e "$expression" "$PATH_TO_HERE/../configure.ac"
if [ $? != 0 ] ; then
    echo "ERROR: Update of configure.ac failed"
    exit 1
fi

echo "---"
echo "$ARG1 has been added to the build system, but not to the Mac Xcode project"
echo "or Windows FullBuild.sln file, where you have to add it manually."
echo ""
echo "You then need to rerun autogen.sh and configure just once to enable your"
echo "plugin with the build system."
echo ""