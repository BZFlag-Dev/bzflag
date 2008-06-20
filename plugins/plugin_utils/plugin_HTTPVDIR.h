/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// a base class for plugins that want to do HTTP VDIRS

/*
 *	
 Notes on indexing and error handling
 1) make default pages for the error codes, send that page if the child dosn't set data.
 2) have accept and pending send in a param saying what callback is the LAST one.
    a) If your the last one, check the URL, if the dir isn't in the vdir list, 404.
    b) If your the last one, check the URL, if it's the root, then index the vdirs  
 3)Add an eOther MimeType enum, and a string to store it. Override the setmime type to take a string and set other.
 */
#ifndef _PLUGIN_HTTPVDIR_H_
#define _PLUGIN_HTTPVDIR_H_

#include <string>
#include <vector>
#include <map>
#include "bzfsAPI.h"

class BZFSHTTPVDir 
{
public:
  BZFSHTTPVDir();
  virtual ~BZFSHTTPVDir();

  typedef enum
  {
    e404 = 0, // not found
    e200,     // ok
    eDeffer
  }PageStatus;

  void registerVDir ( void );

  virtual const char * getVDir ( void ){return NULL;}
  virtual bool useAuth ( void ){return false;}

  virtual PageStatus generatePage ( const char* /*vdir*/, const char* /*resource*/, int /*userID*/, int /*requestID*/ ){return e404;}
};

#endif //_PLUGIN_HTTP_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
