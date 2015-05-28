#!/bin/bash

NARG="$#"
ARG1="${1%/}"
PATH_TO_HERE="`dirname $0`"

# make sure user gives a plugin name
if [[ $NARG != 1 ]]
then
    echo "syntax: $0 <plugin_to_remove>"
    exit 1
fi

# remove from the plugin from the important files
makeFile="$PATH_TO_HERE/Makefile.am"
mkPattern="!/\\t${ARG1}\\ \\\\/";
awk "$mkPattern" "$makeFile" > "$makeFile.new" && mv "$makeFile.new" "$makeFile"

confFile="$PATH_TO_HERE/../configure.ac"
confPattern="!/\\tplugins\/${ARG1}\/Makefile/"
awk "$confPattern" "$confFile" > "$confFile.new" && mv "$confFile.new" "$confFile"

echo "---"
echo "$ARG1 has been removed from the Linux build system. This does NOT remove"
echo "the plugin from the Mac Xcode project or Windows FullBuild.sln. This script"
echo "also does NOT remove the folder contents, you have to remove it manually to"
echo "prevent accidental deletion."
echo ""
echo "You then need to rerun autogen.sh and configure just once to fully remove"
echo "your plugin from the build system."
echo ""
