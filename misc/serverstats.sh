#!/bin/sh
#
#  s e r v e r s t a t s . s h
#
# Computes BZFlag server statistics by querying all public servers
# listed by the list server with bzadmin, and parsing the server
# identification signature.
#
# Version 1.0.0
# Initially written by Sean Morrison aka brlcad aka learner in 2004
# this script is in the public domain
###

echo "BZFlag Server Statistics"
echo "========================"
echo
echo "Processing BEGINS ..."


echo -n "... testing bzadmin ..."
bzadmin="bzadmin"
bzadmin_version="`$bzadmin -help 2>&1`"
[ $? != 1 ] && echo " NOT okay" && echo "ERROR: bzadmin is expected to be in your path" && exit 1
[ ! "x`echo $bzadmin_version | awk '{print $1}'`" = "xbzadmin" ] && echo " NOT okay" && echo "ERROR: bzadmin is expected to be in your path" && exit 1
echo " okay"


echo -n "... testing tempfile usage ..."
tempfile="/tmp/servers.sh.$RANDOM.$$"
touch "$tempfile"
[ ! -f "$tempfile" ] && echo " NOT okay" && echo "ERROR: unable to create temp file ($tempfile)" && exit 1
[ ! -w "$tempfile" ] && echo " NOT okay" && echo "ERROR: unable to write to temp file ($tempfile)" && exit 1
[ ! -r "$tempfile" ] && echo " NOT okay" && echo "ERROR: unable to read temp file ($tempfile)" && exit 1
echo " okay"


echo -n "... getting list of servers ..."
servers=`curl http://my.BZFlag.org/db/?action=LIST 2>/dev/null`
[ $? != 0 ] && echo " NOT okay" && echo "ERROR: unable to curl http://my.BZFlag.org/db/?action=LIST" && exit 1
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
    fileline=`head -$line "$tempfile" | tail -1`
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
server_signatures=""
for server in $servers ; do
    echo -n "... verifying (#$count) $server .."

    for sleeptime in 1 2 4 8 16 ; do
	echo -n "."
	echo "$server" >> $tempfile
	( sleep $sleeptime ; echo "/quit" ) | $bzadmin -ui stdboth serverstats_sh_$RANDOM@$server 2>&1 | grep "BZFlag server" >> $tempfile &
	[ $? != 0 ] && echo ". NOT okay" && echo "WARNING: unable to use bzadmin on $server" && break

	sleep $sleeptime
	server_version="`tail -1 $tempfile | grep "BZFlag server" | awk '{print $4}'`"
	if [ ! "x$server_version" = "x" ] ; then
	    break
	fi
    done
    if [ "x$server_version" = "x" ] ; then
	echo " NOT okay"
	echo "WARNING: $server seems to be unresponsive"
    else
	echo " okay"
	server_signatures="$server_signatures $server_version"
	reachcount="`expr $reachcount + 1`"
    fi
    count="`expr $count + 1`"
done

# !!!
#fi

echo -n "... sorting versions .."
server_versions=""
server_dates=""
server_types=""
server_ostypes=""
server_signatures="`echo $server_signatures | sed 's/,//g'`"
for server in $server_signatures ; do
    serverunder="`echo $server | sed 's/[-.]/_/g'`"
    server2="`echo $serverunder | sed 's/_/ /g'`"

    version="`echo $server2 | awk '{printf "%d.%d.%d", $1, $2, $3}'`"
    server_versions="$server_versions $version"

    builddate="`echo $server2 | awk '{print $4}'`"
    server_dates="$server_dates $builddate"

    buildtype="`echo $server2 | awk '{print $5}'`"
    server_types="$server_types $buildtype"

    ostype="`echo $server | sed 's/.*-[A-Z]*-\(.*\)/\1/'`"
    server_ostypes="$server_ostypes $ostype"

    echo -n "."
done
echo " okay"


echo -n "... killing unterminated bzadmins ..."
killall bzadmin
[ $? = 0 ] && echo " NOT okay" && echo "WARNING: unable to killall bzadmin"
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
echo "Versions:"
seen_versions=""
for i in $server_versions ; do
    seen="no"
    for ver in $seen_versions ; do
	if [ "x$ver" = "x$i" ] ; then
	    seen="yes"
	    break
	fi
    done

    if [ "x$seen" = "xno" ] ; then
	count=0
	for ver in $server_versions ; do
	    if [ "x$ver" = "x$i" ] ; then
		count="`expr $count + 1`"
	    fi
	done
	echo "	$count of $i"
	seen_versions="$seen_versions $i"
    fi
done
echo

echo "Build dates:"
seen_dates=""
for i in $server_dates ; do
    seen="no"
    for date in $seen_dates ; do
	if [ "x$date" = "x$i" ] ; then
	    seen="yes"
	    break
	fi
    done
    if [ "x$seen" = "xno" ] ; then
	count=0
	for date in $server_dates ; do
	    if [ "x$date" = "x$i" ] ; then
		count="`expr $count + 1`"
	    fi
	done
	echo "	$count of `echo $i | sed 's/\([0-9][0-9][0-9][0-9]\)\([0-9][0-9]\)\([0-9][0-9]\)/\1.\2.\3/'`"
	seen_dates="$seen_dates $i"
    fi
done
echo

echo "Build types:"
seen_types=""
for i in $server_types ; do
    seen="no"
    for type in $seen_types ; do
	if [ "x$type" = "x$i" ] ; then
	    seen="yes"
	    break
	fi
    done
    if [ "x$seen" = "xno" ] ; then
	count=0
	for type in $server_types ; do
	    if [ "x$type" = "x$i" ] ; then
		count="`expr $count + 1`"
	    fi
	done
	echo "	$count of $i"
	seen_types="$seen_types $i"
    fi
done
echo

echo "Operating systems:"
seen_os=""
for i in $server_ostypes ; do
    seen="no"
    for os in $seen_os ; do
	if [ "x$os" = "x$i" ] ; then
	    seen="yes"
	    break
	fi
    done
    if [ "x$seen" = "xno" ] ; then
	count=0
	for os in $server_ostypes ; do
	    if [ "x$os" = "x$i" ] ; then
		count="`expr $count + 1`"
	    fi
	done
	echo "	$count of $i"
	seen_os="$seen_os $i"
    fi
done
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
