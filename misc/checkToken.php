<?php

// this function will check a username and token returned by the bzflag weblogin page
// at http://my.bzflag.org/weblogin?action=weblogin
// you can use this URL to ask a user for his bzflag global login.
// your page needs to pass in a URL paramater to the weblogin that contains the URL to be called with the username
// and token. This prevents allows your site to use the same usernames and passwords as the forums with out having
// to worry about being accused of sealing passwords.
// the URL paramater can have the keys %TOKEN% and %USERNAME% that will be replaced with the real username and token
// when the URL is called.
//
// example: http://my.bzflag.org/weblogin?action=weblogin&url=http://www.mysite.com/mydir/login.php?token=%TOKEN%&callsign=%USERNAME%
//
// this would call mysite.com with the token and username passed in as paramaters after the user has given the page a valid username
// and password.

// this function should be used after you get the info from the login callback, to verify that it is a valid token, and the
// user belongs to any groups you care about.

function validate_token($token, $callsign, $groups=array()){
	//Some config options
	$list_server='http://my.bzflag.org/db/';
	
	//The program
	//$key => $group
	$group_list='&groups=';
	foreach($groups as $group){
		$group_list.="$group%0D%0A";
	}
	//Trim the last 6 characters, wich are "%0D%0A", off of the last group
	$group_list=substr($group_list, 0, strlen($group_list)-6);
	
	$reply=file_get_contents(''.$list_server.'?action=CHECKTOKENS&checktokens='.$callsign.'%3D'.$token.''.$group_list.'');
	
	//If we got a TOKBAD, return false, because the token can't be right
	if(strpos($reply, 'TOKBAD: ')) return false;
	
	//Here's where it gets tricky: making sure the user is in all groups specified
	$group_list='';
	foreach($groups as $group){
		$group_list.=":$group";
	}
	if(strpos($reply, "TOKGOOD: $callsign$group_list")){
		return true;
	}else{
		return false;
	}
}
?>