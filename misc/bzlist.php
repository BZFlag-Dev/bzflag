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

$fp = fopen("$listserver", "r");
if ($fp) {
?>
<pre><table border=0 cellpadding=0 cellspacing=0>
  <tr>
    <td width=300>Server Name</td>
    <td width=125>Server Address</td>
    <td>Server Description (short)</td>
  </tr>
  <tr><td colspan=3>&nbsp;</td></tr>
<?
  while (!feof($fp)) {
    $buffer = fgets($fp, 1024);
    $array = preg_split('/\s+/', $buffer);
    $count = count($array);
    list($bzhost, $bzport) = split(":", $array[0], 2);
    $LINK = "<a href=$_SERVER[PHP_SELF]?bzhost=$bzhost&bzport=$bzport>$bzhost:$bzport</a>";
    if (($bzhost)|($bzport)) {
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
  print "</table></pre><br>";
  fclose ($fp);
}

if(($_GET["bzhost"])&&($_GET["bzport"])) {
  ?>
  <br><table border=0 cellpadding=0 cellspacing=0>
    <tr>
      <td><? print "$_GET[bzhost]:$_GET[bzport] Stats:"; ?></td>
    </tr>
    <tr><td>&nbsp;</td></tr>
    <tr>
      <td><pre><? system("$bzfquery $_GET[bzhost] $_GET[bzport]"); ?></pre></td>
    </tr>
  </table>
  <?
}

# Local Variables: ***
# mode:php ***
# tab-width: 8 ***
# c-basic-offset: 2 ***
# indent-tabs-mode: t ***
# End: ***
# ex: shiftwidth=2 tabstop=8

?>
