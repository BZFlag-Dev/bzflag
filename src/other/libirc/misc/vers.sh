#!/bin/sh
# $(#)$Header$
# Version: 1.0
#
#  v e r s . s h
#
###
# BSD License
# Copyright (c) 2004, Christopher Sean Morrison
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
#   Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
###
#
# Update the "version" file for creation of a new "vers.c" from it.
# May be run in any subdirectory of the source tree.  Output goes to
# stdout now so you'll likely need to run:
#
#	sh vers.sh variable_name "this is a title" > vers.c
#
#  Optional args:
#	variable name to put version string in (default="version")
#	title
###

path_to_vers_sh="`dirname $0`"

if test $# -gt 0 ; then
	VARIABLE="$1"
	shift
else
	VARIABLE="version"
fi

if test $# -gt 0 ; then
	TITLE="$*"
else
	TITLE=""
fi

# Obtain RELEASE number
if test -r $path_to_vers_sh/../configure.ac ; then
	version_script=`grep VERSION $path_to_vers_sh/../configure.ac | grep -v SUBST | head -4`
	eval $version_script
	if test ! "x$LIBIRC_VERSION" = "x" ; then
		RELEASE="$LIBIRC_VERSION"
	else
		RELEASE='??.??.??'
	fi
else
	RELEASE='??.??.??'
fi

DIR=`pwd`
if test x$DIR = x; then DIR="/unknown"; fi

if test ! -w version ; then
	rm -f version; echo 0 > version; chmod 664 version
fi

awk '{version = $1 + 1; };END{printf "%d\n", version > "version"; }' < version

VERSION=`cat version`
DATE=`date`
PATH=$PATH:/usr/ucb:/usr/bsd
export PATH

# figure out what machine this is
HOST=$HOSTNAME
if test "x$HOST" = "x" ; then
    HOST="`hostname`"
fi
if test "x$HOST" = "x" ; then
    HOST="`uname -n`"
fi
if test "x$HOST" = "x" ; then
    HOST="//unknown//"
fi

# figure out who this is
if test "x$USER" = "x" ; then
	USER="$LOGNAME"
fi
if test "x$USER" = "x" ; then
	USER="$LOGIN"
fi
if test "x$USER" = "x" ; then
	USER="`whoami`"
fi
if test "x$USER" = "x" ; then
	USER="//unknown//"
fi

cat << EOF
char ${VARIABLE}[] = "\\
@(#) libIRC ${RELEASE}   ${TITLE}\n\\
    ${DATE}, Compilation ${VERSION}\n\\
    ${USER}@${HOST}:${DIR}\n";
EOF

# Local Variables: ***
# mode: sh ***
# tab-width: 8 ***
# sh-basic-offset: 2 ***
# sh-indentation: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
