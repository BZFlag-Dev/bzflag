#!/bin/sh
#
#  p i n g t i m e s . s h
#
# Computes BZFlag server ping times from the caller's location.
# Servers are then returned, sorted in order from high to low ping.
#
# Version 1.0.0
# Initially written by Sean Morrison aka brlcad aka learner in 2004
# this script is in the public domain
###

echo "BZFlag Server Ping Times"
echo "========================"
echo
echo "Processing BEGINS ..."



echo -n "... testing tempfile usage ..."
tempfile="/tmp/pingtimes.sh.$RANDOM.$$"
touch "$tempfile"
[ ! -f "$tempfile" ] && echo " NOT okay" && echo "ERROR: unable to create temp file ($tempfile)" && exit 1
[ ! -w "$tempfile" ] && echo " NOT okay" && echo "ERROR: unable to write to temp file ($tempfile)" && exit 1
[ ! -r "$tempfile" ] && echo " NOT okay" && echo "ERROR: unable to read temp file ($tempfile)" && exit 1
echo " okay"


echo -n "... getting list of servers ..."
servers=`curl "http://my.BZFlag.org/db/?action=LIST" 2>/dev/null`
[ $? != 0 ] && echo " NOT okay" && echo "ERROR: unable to curl http://my.BZFlag.org/db" && exit 1
echo " okay"


echo -n "... writing server list to temporary file ..."
cat > "$tempfile" <<EOF
$servers
EOF
[ $? != 0 ] && echo " NOT okay" && echo "ERROR: unable to write server list to file ($tempfile)" && exit 1
echo " okay"


echo -n "... getting server count ..."
servercount="`cat $tempfile | wc | awk '{print $1}'`"
[ $? != 0 ] && echo " NOT okay" && echo "ERROR: unable to get server count (using wc and awk)" && exit 1
[ $servercount -le 0 ] && echo " NOT okay" && echo "ERROR: no servers found" && exit 1
echo " okay"


echo -n "... getting server names .."
servers=""
line=1
while [ $line -le $servercount ] ; do
    fileline=`head -n $line "$tempfile" | tail -n 1`
    server="`echo $fileline | awk '{print $1}'`"
    servers="$servers $server"
    line="`expr $line + 1`"
    echo -n "."
done
echo " okay"


echo -n "... validating server count ... "
servercount2="`echo $servers | wc | awk '{print $2}'`"
if [ $servercount -ne $servercount2 ] ; then
    echo "NOT okay"
    echo "WARNING: servers parsed does not match initial count"
else
    echo "okay"
fi


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


echo -n "... deleting temporary file ..."
rm -f "$tempfile"
[ $? != 0 ] && echo " NOT okay" && echo "ERROR: unable to properly remove tempfile ($tempfile)" && exit 1
[ -f "$tempfile" ] && echo " NOT okay" && echo "ERROR: tempfile ($tempfile) deleted yet still exits?!" && exit 1
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
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8
