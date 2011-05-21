#!/bin/sh
#
#  p i n g t i m e s . s h
#
# Computes BZFlag server ping times from the caller's location.
# Servers are then returned, sorted in order from high to low ping.
#
# derived from Version 1.0.0
# Initially written by Sean Morrison aka brlcad aka learner in 2004
# this script is in the public domain
###

echo "BZFlag Server Ping Times"
echo "========================"
echo
echo "Processing BEGINS ..."


echo -n "... getting list of servers ..."
servers=`curl "http://my.BZFlag.org/db/?action=LIST" 2> /dev/null | sed 's/[ :].*//' | sort -u`
[ $? != 0 ] && echo " NOT okay" && echo "ERROR: unable to curl http://my.BZFlag.org/db" && exit 1
echo " okay"


reachcount=0
count=1
server_pings=""
server_names=""
for server in $servers ; do
    echo -n "... pinging (#$count) $server .."

    server_name="`echo $server | sed 's/\(.*\):.*/\1/'`"
    ping_result="`ping -q -c 2 $server_name`"
    if [ $? != 0 ] ; then
	echo " NOT okay"
	echo "WARNING: $server seems to be unresponsive"
    else

	if test "`echo $ping_result | awk '{print $18}'`" = "100%" ; then
		echo " NOT okay"
		echo "WARNING: $server seems to be unresponsive"
	else
		server_ping="`echo $ping_result | grep avg | cut -d \/ -f 5`"
		echo " $server_ping"
		server_pings="$server_pings
`echo $server_ping | awk '{print $1}'` ms to $server"
		server_names="$server_names $server"
		reachcount="`expr $reachcount + 1`"
	fi
    fi

    count="`expr $count + 1`"
done


echo -n "... sorting pings .."
sorted_pings="`sort -r -n <<EOF
$server_pings
EOF`"
echo " okay"


echo "... processing COMPLETE"

echo
echo "Server Summary"
echo "--------------"
echo "Servers Listed:	$servercount "
echo "Servers Reached:	$reachcount "
echo
echo "Ping Times:"
echo
echo "$sorted_pings"
echo

echo "Done."

# Local Variables: ***
# mode: sh ***
# tab-width: 8 ***
# sh-basic-offset: 2 ***
# sh-indentation: 2 ***
# indent-tabs-mode: nil ***
# End: ***
# ex: shiftwidth=2 tabstop=8
