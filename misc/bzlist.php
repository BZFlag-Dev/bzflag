<?php

// bzlist.php
//
// original by D. John <g33k@despammed.com>
//
// Copyright (c) 1993 - 2004 Tim Riker
//
// This package is free software;  you can redistribute it and/or
// modify it under the terms of the license found in the file
// named COPYING that should have accompanied this file.
//
// THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// This is a simple script that reports current public servers
// and creates links to server stats via bzfquery.pl.


$listserver = 'http://db.bzflag.org/db/?action=LIST';
$bzfquery = './bzfquery.pl';

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<title>temp</title>
</head>
<body>
<?
if($_GET["hostport"]) {
  ?>
  <table border=0 cellpadding=0 cellspacing=0>
    <tr>
      <td><? print "$_GET[hostport] Stats:"; ?></td>
    </tr>
    <tr><td>&nbsp;</td></tr>
    <tr>
      <td><pre><? system("$bzfquery $hostport"); ?></pre></td>
    </tr>
  </table>
  <?
}

$fp = fopen("$listserver", "r");
if ($fp) {
?>
<table border=0 cellpadding=0 cellspacing=0>
  <tr>
    <td width=300>Server Name</td>
    <td width=125>Server Address</td>
    <td>Server Description (short)</td>
  </tr>
  <!-- <tr><td colspan=3>&nbsp;</td></tr> -->
<?
  while (!feof($fp)) {
    $buffer = fgets($fp, 1024);
    $array = preg_split('/\s+/', $buffer);
    $count = count($array);
    $LINK = "<a href=\"$_SERVER[PHP_SELF]?hostport=$array[0]\">$array[0]</a>";
    if (($array[0])) {
      print "<tr>";
      print "<td>$LINK</td>";
      print "<td>$array[3]</td>";
      unset($array[0], $array[1], $array[2], $array[3]);
      foreach ( $array as $line ) {
	$description .= "$line ";
      }
      //$bzdesc = substr("$description", 0, 50);
      //print "<td>$bzdesc</td>";
      print "<td>$description</td>";
      unset($description);
      print "</tr>\n";
    }
  }
  print "</table>\n";
  fclose ($fp);
} ?>

<p><a href="http://validator.w3.org/check?uri=referer"><img border="0" src="http://www.w3.org/Icons/valid-html401" alt="Valid HTML 4.01!" height="31" width="88"></a></p>
</body>
</html>

<?

# Local Variables: ***
# mode:php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8

?>
