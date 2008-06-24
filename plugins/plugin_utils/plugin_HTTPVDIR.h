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
#include "plugin_HTTPTemplates.h"

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

  //connetion info
  std::string ip;
  std::string hostmask;

  // basic auth
  std::string authType;
  std::string authCredentials;

  bool getParam ( const char* param, std::string &val ) const; 
  bool getParam ( const std::string &param, std::string &val ) const;

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

  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply ) = 0;

  virtual bool resumeTask (  int /*requestID*/ ) {return true;}

protected:
  std::string getBaseURL ( void );
};

class BZFSHTTPVDirAuth : public BZFSHTTPVDir
{
public:
  BZFSHTTPVDirAuth();
  virtual ~BZFSHTTPVDirAuth(){};

  virtual const char * getVDir ( void ){return NULL;}

  // do not overide these, they are used buy the auth system
  // use the 2 variants below
  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply );
  virtual bool resumeTask (  int requestID );

  // authed versions of the main callbacks
  virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply ) = 0;
  virtual bool resumeAuthedTask ( int requestID ){return true;}

protected:
  Templateiser	templateSystem;

  double	sessionTimeout;
  std::string	authPage;

  // a map for each authentication level
  // the value is a list of groups that get that level
  // the highest level for a user will be returned
  // users who have all the groups in the andGroups will be
  // granted the level
  // users who have any of the groups in the orGroups will be
  // granted the level
  typedef struct 
  {
    std::vector<std::string> andGroups;
    std::vector<std::string> orGroups;
  }AuthGroups;

  std::map<int,AuthGroups > authLevels;

  std::vector<int> defferedAuthedRequests;

  int getLevelFromGroups (const std::vector<std::string> &groups );
private:
  void flushTasks ( void );
  bool verifyToken ( const HTTPRequest &request, HTTPReply &reply );

  typedef struct  
  {
    double time;
    int level;
  }AuthInfo;

  std::map<int,AuthInfo> authedSessions;
};

#endif //_PLUGIN_HTTP_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
