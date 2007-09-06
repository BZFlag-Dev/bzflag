/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// a base class for plugins that want to do HTTP can use
#ifndef _PLUGIN_HTTP_H_
#define _PLUGIN_HTTP_H_

#include <string>
#include <vector>
#include <map>
#include "bzfsAPI.h"

class BZFSHTTPServer : public bz_EventHandler, bz_NonPlayerConnectionHandler
{
public:
  BZFSHTTPServer( const char * plugInName );
  virtual ~BZFSHTTPServer();


  void startupHTTP ( void );
  void shutdownHTTP ( void );

  // BZFS callback methods
  virtual void process ( bz_EventData *eventData );
  virtual void pending ( int connectionID, void *d, unsigned int s );
  virtual void disconnect ( int connectionID );

  // virtual functions to implement
  virtual bool acceptURL ( const char *url ) = 0;
  virtual void getURLData ( const char* url, int requestID, const std::map<std::string,std::string> &paramaters, bool get = true ) = 0;

protected:
  // called inside getURLData to set the data for the job
  void setURLDataSize ( unsigned int size, int requestID );
  void setURLData ( const char * data, int requestID );

  typedef enum
  {
    eText,
    eOctetStream,
    eBinary
  }HTTPDocumentType;

  void setURLDocType ( HTTPDocumentType docType, int requestID );

  // called internaly to update any transfers
  // but can be called externaly to force updates if need be
  void update ( void );

  // so the server can know what it's address is
  const char * getBaseServerURL ( void );

protected:
  typedef enum
  {
    eGet
  }HTTPRequest;

  typedef struct 
  {
    HTTPRequest request;
    std::string URL;
    char	*data;
    unsigned int size;
    HTTPDocumentType  docType;
  }HTTPCommand;

  class HTTPConectedUsers
  {
  public:
    HTTPConectedUsers(int connectionID );
    ~HTTPConectedUsers();
    
    bool transfering ( void );
    void startTransfer ( HTTPCommand *command );
    void update ( void );

    std::string getMimeType ( HTTPDocumentType docType );

    std::vector<HTTPCommand*> pendingCommands;

    std::string commandData;
    double	aliveTime;

    unsigned int pos;

    int connection;

    void killMe ( void );
  };

private:
  std::map<int,HTTPConectedUsers*> users;

  std::string baseURL;
  std::string vdir;
  bool	      listening;
  float	      savedUpdateTime;

  HTTPCommand *theCurrentCommand;
};

#endif //_PLUGIN_HTTP_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
