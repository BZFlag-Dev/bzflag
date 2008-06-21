// httpTest.cpp : Defines the entry point for the DLL application.
//

#include <map>
#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include <string>
#include <sstream>

BZ_GET_PLUGIN_VERSION

class Fastmap : public BZFSHTTPVDir, public bz_EventHandler
{
public:
  Fastmap(): BZFSHTTPVDir(),mapData(NULL),mapDataSize(0){registerVDir();}
  virtual ~Fastmap(){};

  virtual const char * getVDir ( void ){return "fastmap";}

  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply, int userID )
  {
    reply.returnCode = HTTPReply::e200OK;
    reply.docType = HTTPReply::eOctetStream;

    if (mapData && mapDataSize)
    {
      reply.body.resize(mapDataSize);

      std::stringstream stream(reply.body);
      stream.write(mapData,(std::streamsize)mapDataSize);
    }
    else
      reply.returnCode = HTTPReply::e404NotFound;

   
    return true;
  }

  virtual void process(bz_EventData * eventData)
  {
    if (eventData->eventType == bz_eWorldFinalized)
    {
      if (mapData)
	free(mapData);

      mapData = NULL;
      mapDataSize = 0;

      if (!bz_getPublic() || bz_getClientWorldDownloadURL().size())
	return;

      mapDataSize = bz_getWorldCacheSize();
      if (!mapDataSize)
	return;

      mapData = (char *) malloc(mapDataSize);
      if (!mapData)
      {
	mapDataSize = 0;
	return;
      }

      bz_getWorldCacheData((unsigned char*)mapData);

      bz_debugMessagef(2, "FastMap: Running local HTTP server for maps using URL %s", getBaseURL().c_str());
      bz_setClientWorldDownloadURL(getBaseURL().c_str());
    }
  }
  char *mapData;
  size_t mapDataSize;
};

Fastmap *server = NULL;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"Fastmap plugin loaded");
  if(server)
    delete(server);

  server = new Fastmap;

  bz_registerEvent(bz_eWorldFinalized, server);

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_eWorldFinalized, server);

  if(server)
    delete(server);
  server = NULL;

  bz_debugMessage(4,"Fastmap plugin unloaded");
  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
