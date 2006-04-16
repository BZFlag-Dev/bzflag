<?php



include('bzfquery.php');



// Query the server

$data = bzfquery("localhost:5154");



// Display the server info

echo'<pre>';

bzfdump($data);

echo'</pre>';



?>
