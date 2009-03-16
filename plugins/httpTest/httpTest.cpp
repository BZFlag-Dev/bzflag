// httpTest.cpp : Defines the entry point for the DLL application.
//

#include <map>
#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

BZ_GET_PLUGIN_VERSION

#define DO_BASIC_AUTH false

class HTTPTest : public BZFSHTTP
{
public:
  HTTPTest(): BZFSHTTP(){registerVDir();}
  virtual ~HTTPTest(){};

  virtual const char * getVDir ( void ){return "test";}
  virtual bool supportPut ( void ){return true;}

  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply )
  {
    if (DO_BASIC_AUTH)
    {
      if (!request.authType.size() || !request.authCredentials.size())
      {
	reply.returnCode = HTTPReply::e401Unauthorized;
	reply.authType = "Basic";
	reply.authRealm = "BZFS_Test_Access";
	reply.docType = HTTPReply::eHTML;
	reply.body = "Authentication Required";
	return true;
      }
    }

    reply.returnCode = HTTPReply::e200OK;
    reply.docType = HTTPReply::eHTML;
    if (request.request == ePut)
    {
      // read in what we got
      // send them back what we got
      reply.docType = HTTPReply::eOctetStream;
      reply.body = request.body;
    }
    else
    {
      reply.body = format("<html><head></head><body>\n");

      reply.body += format("Your sessionID is %d<br>\n",request.sessionID);
      reply.body += "<a href=\"" + request.baseURL + "link1\">Link1</a><br>";
      reply.body += "<a href=\"" + request.baseURL + "link2\">Link2</a>";

      if (request.authType.size() && request.authCredentials.size())
      {
	reply.body +="<br>You authenticated, using the type: " + request.authType + "<br>";
	reply.body +="<br>Your credentials are: " + request.authCredentials + "<br>";

	if (request.username.size() && request.password.size())
	{
	  reply.body +="<br>Your username is: " + request.username + "<br>";
	  reply.body +="<br>Your password is: " + request.password + "<br>";
	}
      }

      reply.body += "</body></html>";
    }
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
