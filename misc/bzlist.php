<?

// bzlist.php D. John < g33k@despammed.com >
//
// Copyright (c) 1993 - 2003 Tim Riker
//
// This package is free software;  you can redistribute it and/or
// modify it under the terms of the license found in the file
// named LICENSE that should have accompanied this file.
//
// THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// This is a simple script that reports current public servers
// and creates links to server stats via bzfquery.pl.

$listserver  = 'http://list.bzflag.org:5156';
$bzfquery    = '/usr/X11R6/bin/bzfquery.pl';

$fp = fopen("$listserver", "r");
if ($fp) {
	?>
	<PRE><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>
	  <TR>
	    <TD WIDTH=300>Server Name</TD>
	    <TD WIDTH=125>Server Address</TD>
	    <TD>Server Description (short)</TD>
	  </TR>
	  <TR><TD COLSPAN=3>&nbsp;</TD></TR>
	<?
	while (!feof($fp)) {
		$buffer = fgets($fp, 1024);
		$array  = preg_split('/\s+/', $buffer);
		$count  = count($array);
		list($bzhost, $bzport) = split(":", $array[0], 2);
		$LINK   = "<A HREF=$_SERVER[PHP_SELF]?bzhost=$bzhost&bzport=$bzport>$bzhost:$bzport</A>";
		if(($bzhost)|($bzport)) {
			print "<TR>";
			print "<TD>$LINK</TD>";
			print "<TD>$array[3]</TD>";
			unset($array[0], $array[1], $array[2], $array[3]);
			foreach ( $array as $line ) {
				$description .= "$line ";
			}
			$bzdesc = substr("$description", 0, 50);
			unset($description);
			print "<TD>$bzdesc</TD>";
			print "</TR>";
		}
	}
	?></TABLE></PRE><BR><?
	fclose ($fp);
}

if(($_GET["bzhost"])&&($_GET["bzport"])) {
	?>
	<BR><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>
	  <TR>
	    <TD><? print "$_GET[bzhost]:$_GET[bzport] Stats:"; ?></TD>
	  </TR>
	    <TR><TD>&nbsp;</TD></TR>
	  <TR>
	    <TD><PRE><? system("$bzfquery $_GET[bzhost] $_GET[bzport]"); ?></PRE></TD>
	  </TR>
	</TABLE>
	<?
}

?>
