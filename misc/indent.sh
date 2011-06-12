#!/bin/sh
#
#  Run this script from the top level of the BZFlag source tree.
#  (ex: ./misc/indent.sh). Excluded directories ar listed in the
#  misc/astyle.conf file.
#


cd `dirname "$0"`/..


ASTYLE_EXEC="./misc/astyle-bzflag"


# It's go time!

"$ASTYLE_EXEC" --options=misc/astyle.conf -r '*.h' '*.c' '*.cpp'


################################################################################
