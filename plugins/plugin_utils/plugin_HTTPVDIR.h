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

typedef enum 
{
  eUnknown = 0,
  eHead,
  eGet,
  ePost,
  ePut,
  eDelete,
  eTrace,
  eOptions,
  eConnect,
  eOther
}HTTPRequestType;

class HTTPRequest
{
public:
  HTTPRequest() : requestID(-1), request(eUnknown){};

  int		  sessionID;
  int		  requestID;
  HTTPRequestType request;

  std::string vdir;
  std::string resource;
  std::map<std::string, std::string> paramaters;

  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> cookies;

  std::string body;
  std::string baseURL;

  // basic auth
  std::string authType;
  std::string authCredentials;
};

class HTTPReply
{
public:
  HTTPReply(): docType(eText), returnCode(e404NotFound), forceNoCache(true){};

  typedef enum
  {
    eText,
    eOctetStream,
    eBinary,
    eHTML
  }DocumentType;

  typedef enum
  {
    e200OK,
    e301Redirect,
    e401Unauthorized,
    e403Forbiden,
    e404NotFound,
    e500ServerError
  }ReturnCode;

  DocumentType docType;
  ReturnCode  returnCode;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> cookies;

  std::string body;

  // authentication method
  std::string authType;
  std::string authRealm;

  bool forceNoCache;

  // content info
  std::string lastUpdateTime;
  std::string md5;

  
  std::string redirectLoc;
};

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

  virtual const char * getVDir ( void ) = 0;
  virtual const char * getDescription ( void ){return "";}
  virtual bool supportPut ( void ){return false;}

  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply, int userID ) = 0;

  virtual bool resumeTask ( int /*userID*/, int /*requestID*/ ) {return true;}

protected:
  std::string getBaseURL ( void );
};

#endif //_PLUGIN_HTTP_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
