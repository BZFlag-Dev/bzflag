<?php

// This function will check a username and token returned by the bzflag
// weblogin page at http://my.bzflag.org/weblogin.php?action=weblogin. You can use
// this URL to ask a user for his bzflag global login. Your page needs to pass
// in an URL paramater to the weblogin that contains your URL to be called with
// the username and token. This allows your site to use the same usernames and
// passwords as the forums with out having to worry about being accused of
// stealing passwords. The URL paramater can have the keys %TOKEN% and
// %USERNAME% that will be replaced with the real username and token when the
// URL is called. For example:
//
// http://my.bzflag.org/weblogin.php?action=weblogin&url=http://www.mysite.com/mydir/login.php?token=%TOKEN%&username=%USERNAME%
//
// NOTE: The URL passed MUST be URL encoded.  The example above shows the URL
// in plain text to make it clearer what is happening.
//
// This would call mysite.com with the token and username passed in as
// paramaters after the user has given the page a valid username and password.
//
// This function should be used after you get the info from the login callback,
// to verify that it is a valid token, and to test which groups the user is a
// member of.
//
// Sites MUST redirect the user to the login form. Sites that send the login
// info from any other form will automaticly be rejected. The aim of this
// service to to show the user that login info is being sent to bzflag.org.
//
// Sites can send a CSS file to us, using the 'css' paramater to tweak the 
// look of the login page to better match the site that is calling it.
//
// TODO: Add some error handling/reporting

function validate_token($token, $username, $groups = array(), $checkIP = true)
{
  // We should probably do a little more error checking here and
  // provide an error return code (define constants?)
  if (isset($token, $username) && strlen($token) > 0 && strlen($username) > 0)
  {
    $listserver = Array();

    // First off, start with the base URL
    $listserver['url'] = 'http://my.bzflag.org/db/';
    // Add on the action and the username
    $listserver['url'] .= '?action=CHECKTOKENS&checktokens='.urlencode($username);
    // Make sure we match the IP address of the user
    if ($checkIP) $listserver['url'] .= '@'.$_SERVER['REMOTE_ADDR'];
    // Add the token
    $listserver['url'] .= '%3D'.$token;
    // If use have groups to check, add those now
    if (is_array($groups) && sizeof($groups) > 0)
      $listserver['url'] .= '&groups='.implode("%0D%0A", $groups);

    // Run the web query and trim the result
    // An alternative to this method would be to use cURL
    $listserver['reply'] = trim(file_get_contents($listserver['url']));

  //EXAMPLE TOKGOOD RESPONSE
  /*
  MSG: checktoken callsign=SuperAdmin, ip=, token=1234567890  group=SUPER.ADMIN group=SUPER.COP group=SUPER.OWNER
  TOKGOOD: SuperAdmin:SUPER.ADMIN:SUPER.OWNER
  BZID: 123456 SuperAdmin
  */

    // Fix up the line endings just in case
    $listserver['reply'] = str_replace("\r\n", "\n", $listserver['reply']);
    $listserver['reply'] = str_replace("\r", "\n", $listserver['reply']);
    $listserver['reply'] = explode("\n", $listserver['reply']);

    // Grab the groups they are in, and their BZID
    foreach ($listserver['reply'] as $line)
    {
      if (substr($line, 0, strlen('TOKGOOD: ')) == 'TOKGOOD: ')
      {
        if (strpos($line, ':', strlen('TOKGOOD: ')) == FALSE) continue;
        $listserver['groups'] = explode(':', substr($line, strpos($line, ':', strlen('TOKGOOD: '))+1 ));
      }
      else if (substr($line, 0, strlen('BZID: ')) == 'BZID: ')
      {
        list($listserver['bzid'],$listserver['username']) = explode(' ', substr($line, strlen('BZID: ')), 2);
      }
    }

    if (isset($listserver['bzid']) && is_numeric($listserver['bzid']))
    {
      $return['username'] = $listserver['username'];
      $return['bzid'] = $listserver['bzid'];

      if (isset($listserver['groups']) && sizeof($listserver['groups']) > 0)
      {
        $return['groups'] = $listserver['groups'];
      }
      else
      {
        $return['groups'] = Array();
      }

      return $return;
    }
  } // if (isset($token, $username))
} // validate_token(...)

?>
