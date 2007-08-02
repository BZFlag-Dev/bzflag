#!/bin/sh
# $(#)$Header$
# Version: 1.0
#
#  e l a p s e d . s h
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
# Compute the amount of time elapsed and format the results for
# printing.  Script takes three arguments: a number for the hour,
# minute, and seconds.  Presently does not support times that span
# days or multiple days.
#
# Example: sh elapsed.sh 12 03 24
#
###

CONFIG_TIME="$*"

# make sure an argument is given
if test "x$CONFIG_TIME" = "x" ; then
	usage="Usage: $0 time"
	echo "$usage" 1>&2
	exit 1
fi

# if there is no second argument, assume it's just the time
if test "x$2" = "x" ; then
	CONFIG_TIME="`echo $* | tr : ' '`"
fi

# if there is a fourth, assume date format string
if test ! "x$4" = "x" ; then
	CONFIG_TIME="`echo $* | awk '{print $4}' | tr : ' '`"
fi

# parse the start time and convert to a seconds count
time_elapsed=""
pre_hour="`echo $CONFIG_TIME | awk '{print $1}'`"
pre_min="`echo $CONFIG_TIME | awk '{print $2}'`"
pre_sec="`echo $CONFIG_TIME | awk '{print $3}'`"
hour_seconds_before="`expr $pre_hour \* 60 \* 60`"
min_seconds_before="`expr $pre_min \* 60`"
total_pre="`expr $hour_seconds_before + $min_seconds_before + $pre_sec`"

# parse the end time and convert to a seconds count
post_conf_time="`date '+%H %M %S'`"
post_hour="`echo $post_conf_time | awk '{print $1}'`"
post_min="`echo $post_conf_time | awk '{print $2}'`"
post_sec="`echo $post_conf_time | awk '{print $3}'`"
hour_seconds_after="`expr $post_hour \* 60 \* 60`"
min_seconds_after="`expr $post_min \* 60`"
total_post="`expr $hour_seconds_after + $min_seconds_after + $post_sec`"

# if the end time is smaller than the start time, we have gone back in
# time so assume that the clock turned over a day.
if test $total_post -le $total_pre ; then
	total_post="`expr $total_post + 86400`"
fi

# break out the elapsed time into seconds, minutes, and hours
sec_elapsed="`expr $total_post - $total_pre`"
min_elapsed="0"
if test ! "x$sec_elapsed" = "x0" ; then
	min_elapsed="`expr $sec_elapsed / 60`"
	sec_elapsed2="`expr $min_elapsed \* 60`"
	sec_elapsed="`expr $sec_elapsed - $sec_elapsed2`"
fi
hour_elapsed="0"
if test ! "x$min_elapsed" = "x0" ; then
	hour_elapsed="`expr $min_elapsed / 60`"
	min_elapsed2="`expr $hour_elapsed \* 60`"
	min_elapsed="`expr $min_elapsed - $min_elapsed2`"
fi

# generate a human-readable elapsed time message
if test ! "x$hour_elapsed" = "x0" ; then
	if test "x$hour_elapsed" = "x1" ; then
		time_elapsed="$hour_elapsed hour"
	else
		time_elapsed="$hour_elapsed hours"
	fi
fi
if test ! "x$min_elapsed" = "x0" ; then
	if test ! "x$time_elapsed" = "x" ; then
		time_elapsed="${time_elapsed}, "
	fi
	time_elapsed="${time_elapsed}${min_elapsed}"
	if test "x$min_elapsed" = "x1" ; then
		time_elapsed="${time_elapsed} minute"
	else
		time_elapsed="${time_elapsed} minutes"
	fi
fi
if test ! "x$sec_elapsed" = "x0" ; then
	if test ! "x$time_elapsed" = "x" ; then
		time_elapsed="${time_elapsed}, "
	fi
	time_elapsed="${time_elapsed}${sec_elapsed}"
	if test "x$sec_elapsed" = "x1" ; then
		time_elapsed="${time_elapsed} second"
	else
		time_elapsed="${time_elapsed} seconds"
	fi
fi
if test "x$time_elapsed" = "x" ; then
	time_elapsed="0 seconds"
fi

# output the time elapsed
echo "$time_elapsed"

# Local Variables: ***
# mode: sh ***
# tab-width: 8 ***
# sh-basic-offset: 2 ***
# sh-indentation: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
