// webReport.cpp : Defines the entry point for the DLL application.
//

#ifndef _PLUGIN_AUTH_HTTP_H
#define _PLUGIN_AUTH_HTTP_H

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_HTTPTemplates.h"
#include "plugin_groups.h"

class BZFSAUTHHTTPServer;


// used to do the async token verification

class TokenTask : public bz_BaseURLHandler
{
public:
  TokenTask(BZFSAUTHHTTPServer *s);

  virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
  virtual void timeout ( const char* /*URL*/, int /*errorCode*/ );
  virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ );

  BZFSAUTHHTTPServer *server;
  int requestID;
  std::string URL;
  std::vector<std::string> groups;

  std::string data;
};

class BZFSAUTHHTTPServer : public BZFSHTTPServer
{
public:
  BZFSAUTHHTTPServer(const char * p);
  ~BZFSAUTHHTTPServer();

  void init( const char* searchDir, const char *displayName, const char* description );
  void stop( void );

  virtual bool accept ( const std::string& /*URL*/, const std::string &/*ip ){return true;}
  virtual bool getPage ( const char* /*url*/, int /*user*/, const URLParams* &/*parameters*/, bool /*get*/ = true) = 0;

  void setLoginMessage ( const char* page );
  void setLoginMessage ( const std::string& page );

  std::map<int,bool> pendingAuths;

  std::vector<TokenTask*> tasksToFlush;
  std::vector<TokenTask*> tasks;

protected:
  virtual bool acceptURL ( const char * /*url*/ ){return true;}
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );

  void flushTasks ( void );

  Templateiser templateSystem;
  std::string plugInName;

  std::string loginMessage;
};


#endif //_PLUGIN_AUTH_HTTP_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
