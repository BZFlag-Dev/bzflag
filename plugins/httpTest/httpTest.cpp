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
  virtual bool generatePage ( HTTPReply &reply, const char* /*vdir*/, const char* /*resource*/, int /*userID*/, int /*requestID*/ )
  {
    reply.returnCode = HTTPReply::e200OK;
    reply.docType = HTTPReply::eText;
    reply.body = "test";
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
