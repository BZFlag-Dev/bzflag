// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

class FastMap : public BZFSHTTPServer
{
public:
  FastMap(const char * plugInName);

  virtual bool acceptURL(const char *url) { return handler.mapData && handler.mapDataSize; }
  virtual void getURLData(const char* url, int requestID, const URLParams &parameters, bool get = true);

  class Handler : public bz_EventHandler
  {
  public:
    BZFSHTTPServer *server;

    virtual ~Handler ( void )
    {
      if (mapData)
	free(mapData);

      mapData = NULL;
      mapDataSize = 0;
    }

    virtual void process(bz_EventData * eventData)
    {
      if (eventData->eventType == bz_eWorldFinalized)
      {
	if (mapData)
	  free(mapData);

	mapData = NULL;
	mapDataSize = 0;

	if (!server || !bz_getPublic() || bz_getClientWorldDownloadURL().size())
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

	bz_debugMessagef(2, "FastMap: Running local HTTP server for maps using URL %s", server->getBaseServerURL());
	bz_setClientWorldDownloadURL(server->getBaseServerURL());
      }
    }
    char *mapData;
    size_t mapDataSize;
  };

  Handler handler;
};

FastMap fastmap("fastmap");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char *commandLine)
{
  bz_registerEvent(bz_eWorldFinalized, &fastmap.handler);

  bz_setclipFieldString("fastmap_index_description", "Download the map via HTTP");
  fastmap.startupHTTP();
  bz_debugMessage(4, "fastmap plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_eWorldFinalized, &fastmap.handler);

  fastmap.shutdownHTTP();
  bz_debugMessage(4, "fastmap plugin unloaded");
  return 0;
}

// template stuff
// globals

FastMap::FastMap(const char *plugInName): BZFSHTTPServer(plugInName)
{
  handler.mapData = NULL;
  handler.mapDataSize = 0;
  handler.server = this;
}

void FastMap::getURLData(const char* url, int requestID, const URLParams &parameters, bool get)
{
  setURLDocType(eOctetStream, requestID);
  setURLDataSize(handler.mapDataSize, requestID);
  setURLData(handler.mapData, requestID);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
