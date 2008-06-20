// httpTest.cpp : Defines the entry point for the DLL application.
//

#include <map>
#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"

BZ_GET_PLUGIN_VERSION

class HTTPTest : public BZFSHTTPVDir
{
public:
  HTTPTest(): BZFSHTTPVDir(){registerVDir();}
  virtual ~HTTPTest(){};

  virtual const char * getVDir ( void ){return "test";}
  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply, int userID )
  {
    reply.returnCode = HTTPReply::e200OK;
    reply.docType = HTTPReply::eHTML;
    reply.body = format("<html><head></head><body>Your userID is %d<br>\n",userID);

    reply.body += format("Your sessionID is %d<br>\n",request.sessionID);
    reply.body += "<a href=\"" + request.baseURL + "link1\">Link1</a>";
    reply.body += "<a href=\"" + request.baseURL + "link2\">Link2</a>";
 
    reply.body += "</body></html>";
    return true;
  }
};

HTTPTest *server = NULL;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"httpTest plugin loaded");
  if(server)
    delete(server);

  server = new HTTPTest;
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  if(server)
    delete(server);
  server = NULL;

  bz_debugMessage(4,"httpTest plugin unloaded");
 return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
