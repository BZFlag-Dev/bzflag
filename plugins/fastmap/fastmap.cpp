// fastmap.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <map>
#include <string>

BZ_GET_PLUGIN_VERSION

class FastMapEventHandler;

class FastMapClient:public bz_NonPlayerConnectionHandler
{
public:
  FastMapClient(int connectionID = -1, FastMapEventHandler * h = NULL);
  virtual ~FastMapClient();
  virtual void pending(int connectionID, void *d, unsigned int s);
  virtual void disconnect(int connectionID);

  void startTransfer(unsigned char *d, unsigned int s);
  bool updateTransfer(void);

  bool transfering(void);
  int conID;

  unsigned char *data;
  unsigned int size;
  unsigned int currerntPos;

  std::string command;

  FastMapEventHandler *handler;

  std::vector < double >updatTimes;

  unsigned int dataSent;
};

class FastMapEventHandler:public bz_EventHandler
{
public:
  FastMapEventHandler();
  virtual ~FastMapEventHandler();

  virtual void process(bz_EventData * eventData);

  void updateHTTPServer(void);
  void killHTTPServer(void);

  std::string mapName;

  std::map < int, FastMapClient * >clients;

  unsigned char *mapData;
  unsigned int mapDataSize;
};

FastMapEventHandler fastMapEventHandler;

BZF_PLUGIN_CALL int bz_Load(const char * /*commandLine */ )
{
  bz_debugMessage(4, "fastmap plugin loaded");
  bz_registerEvent(bz_eWorldFinalized, &fastMapEventHandler);
  bz_registerEvent(bz_eTickEvent, &fastMapEventHandler);
  bz_registerEvent(bz_eNewNonPlayerConnection, &fastMapEventHandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  fastMapEventHandler.killHTTPServer();

  bz_debugMessage(4, "fastmap plugin unloaded");
  bz_removeEvent(bz_eWorldFinalized, &fastMapEventHandler);
  bz_removeEvent(bz_eTickEvent, &fastMapEventHandler);
  bz_removeEvent(bz_eNewNonPlayerConnection, &fastMapEventHandler);
  return 0;
}

FastMapEventHandler::FastMapEventHandler()
{
  mapName = "fastmap/map.bwc";
  mapData = NULL;
  mapDataSize = 0;
}

FastMapEventHandler::~FastMapEventHandler()
{
  killHTTPServer();
}


void FastMapEventHandler::process(bz_EventData * eventData)
{
  if (eventData->eventType == bz_eWorldFinalized) {
    killHTTPServer();

    if (!bz_getPublic() || bz_getClientWorldDownloadURL().size())
      return;

    mapDataSize = bz_getWorldCacheSize();

    mapData = (unsigned char *) malloc(mapDataSize);
    if (!mapDataSize)
      return;

    bz_getWorldCacheData(mapData);

    std::string hostport = "127.0.0.1:5154";
    if (bz_getPublicAddr().size())
      hostport = bz_getPublicAddr().c_str();

    // make sure it has the port
    if (strrchr(hostport.c_str(), ':') == NULL)
      hostport += format(":%d", bz_getPublicPort());

    std::string url = format("HTTP://%s/%s", hostport.c_str(), mapName.c_str());

    bz_debugMessagef(2, "FastMap: Running local HTTP server for maps using URL %s", url.c_str());
    bz_setClientWorldDownloadURL(url.c_str());
  } else if (eventData->eventType == bz_eTickEvent) {
    if (mapDataSize)
      updateHTTPServer();
  } else if (eventData->eventType == bz_eNewNonPlayerConnection) {
    if (mapDataSize) {
      bz_NewNonPlayerConnectionEventData_V1 *connData = (bz_NewNonPlayerConnectionEventData_V1 *) eventData;
      char *temp = (char *) malloc(connData->size + 1);
      memcpy(temp, connData->data, connData->size);
      temp[connData->size] = 0;
      bz_debugMessagef(2, "FastMap: Non ProtoConnection connection from %d with %s", connData->connectionID, temp);
      free(temp);

      FastMapClient *handler = new FastMapClient(connData->connectionID, this);

      if (bz_registerNonPlayerConnectionHandler(connData->connectionID, handler)) {
	bz_debugMessagef(2, "FastMap: Local HTTP connection from %d", connData->connectionID);
	clients[connData->connectionID] = handler;
	handler->pending(connData->connectionID, connData->data, connData->size);
      } else
	delete(handler);
    }
  } else if (eventData->eventType == bz_eIdleNewNonPlayerConnection) {
    bz_NewNonPlayerConnectionEventData_V1 *connData = (bz_NewNonPlayerConnectionEventData_V1 *) eventData;

    std::map < int, FastMapClient * >::iterator itr = clients.find(connData->connectionID);
    if (itr != clients.end()) {
      bz_removeNonPlayerConnectionHandler(itr->second->conID, itr->second);
      delete(itr->second);
      clients.erase(itr);
    }
    bz_disconnectNonPlayerConnection(connData->connectionID);
  }
}

void FastMapEventHandler::updateHTTPServer(void)
{
  std::vector < int >killList;

  std::map < int, FastMapClient * >::iterator itr = clients.begin();
  while (itr != clients.end()) {
    if (itr->second->conID == -1 || !itr->second->updateTransfer())
      killList.push_back(itr->first);
    itr++;
  }

  for (unsigned int i = 0; i < (unsigned int) killList.size(); i++) {
    itr = clients.find(killList[i]);
    FastMapClient *client = itr->second;
    if (bz_getDebugLevel() >= 2 && client->updatTimes.size() > 1) {
      double totalTime = (*(client->updatTimes.end() - 1)) - (*(client->updatTimes.begin()));
      double averageWait;
      double maxWait = 0;
      double minWait = 99999999.0;

      for (unsigned int k = 0; k < (unsigned int) client->updatTimes.size(); k++) {
	if (k > 1) {
	  double wait = client->updatTimes[k] - client->updatTimes[k - 1];
	  averageWait + wait;
	  if (wait > maxWait)
	    maxWait = wait;
	  if (wait < minWait)
	    minWait = wait;
	}
      }
      averageWait /= (double) client->updatTimes.size() - 1;

      bz_debugMessagef(2, "FastMap: File Transfer Complete");
      bz_debugMessagef(2, "FastMap: File Transfer Stats");
      bz_debugMessagef(2, "FastMap:  %f kbytes transfered in %f seconds (%fkbps)", (double) client->dataSent / 1024.0, totalTime,
		       ((double) client->dataSent / 1024.0) / totalTime);
      bz_debugMessagef(3, "FastMap:  %f average wait", averageWait);
      bz_debugMessagef(3, "FastMap:  %f max wait", maxWait);
      bz_debugMessagef(3, "FastMap:  %f min wait", minWait);
    }
    clients.erase(itr);
  }

  if (clients.size())
    bz_setMaxWaitTime(0.01f, "FASTMAP");
  else
    bz_clearMaxWaitTime("FASTMAP");
}

void FastMapEventHandler::killHTTPServer(void)
{
  if (mapData)
    free(mapData);

  bz_clearMaxWaitTime("FASTMAP");

  mapData = NULL;
  mapDataSize = 0;
}

FastMapClient::FastMapClient(int connectionID, FastMapEventHandler * h)
{
  conID = connectionID;

  data = NULL;
  size = 0;
  currerntPos = 0;
  handler = h;
  dataSent = 0;
}

FastMapClient::~FastMapClient()
{
  if (data)
    free(data);
}

void FastMapClient::pending(int connectionID, void *s, unsigned int d)
{
  if (connectionID != conID)
    return;

  if (d == 0)
    return;

  char *c = (char *) malloc(d + 1);
  memcpy(c, s, d);
  c[d] = 0;
  command += c;
  free(c);

  // parse the command
  std::vector < std::string > commandList = tokenize(command, std::string("\r\n"), 0, false);

  if (commandList.size() == 1 && commandList[0].size() == command.size())	// there were no delims, we are done.
    return;

  for (unsigned int i = 0; i < (unsigned int) commandList.size(); i++) {
    command.erase(command.begin(), command.begin() + commandList[i].size());

    std::string thisLine = commandList[i];

    if (strncmp(thisLine.c_str(), "GET", 3) == 0 && !data)	// it's a get and we arn't in a transfer
      {
	bz_debugMessagef(2, "FastMap: HTTP Command: %s", thisLine.c_str());
	std::vector < std::string > params = tokenize(thisLine, std::string(" "), 0, false);
	if (params.size() <= 3) {
	  std::string urlPath = params[1];

	  if (urlPath.size() > 2 && handler->mapName == urlPath.c_str() + 1) {
	    startTransfer(handler->mapData, handler->mapDataSize);
	  }
	}
      } else
      bz_debugMessagef(3, "FastMap: HTTP Command: %s", thisLine.c_str());
  }

  // if it wasn't an http thing, let it go, do nothing and release our
  // hold on it, let it go
  if (!transfering()) {
    bz_removeNonPlayerConnectionHandler(conID, this);
    conID = -1;
  }
}

void FastMapClient::disconnect(int connectionID)
{
  if (connectionID != conID)
    return;

  bz_removeNonPlayerConnectionHandler(conID, this);
  bz_disconnectNonPlayerConnection(conID);
  conID = -1;

}
void FastMapClient::startTransfer(unsigned char *d, unsigned int s)
{
  if (data)
    free(data);
  data = NULL;
  size = 0;
  currerntPos = 0;

  if (!s)
    return;

  bz_debugMessagef(2, "FastMap: File Transfer for connection %d started", conID);

  data = (unsigned char *) malloc(s);
  memcpy(data, d, s);
  size = s;

  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 200 OK\n";
  httpHeaders += format("Content-Length: %d\n", size);
  httpHeaders += "Connection: close\n";
  httpHeaders += "Content-Type: application/octet-stream\n";
  httpHeaders += "\n";

  dataSent += (unsigned int) httpHeaders.size();
  if (!bz_sendNonPlayerData(conID, httpHeaders.c_str(), (unsigned int) httpHeaders.size()) || !updateTransfer())
    disconnect(conID);
}

bool FastMapClient::updateTransfer(void)
{
  int chunkToSend = 1000;

  if (!transfering())
    return false;

  updatTimes.push_back(bz_getCurrentTime());
  // wait till the current data is sent
  if (bz_getNonPlayerConnectionOutboundPacketCount(conID) != 0)
    return true;

  if (currerntPos + chunkToSend > size)
    chunkToSend = size - currerntPos;

  bool worked = bz_sendNonPlayerData(conID, data + currerntPos, chunkToSend);

  currerntPos += chunkToSend;
  dataSent += chunkToSend;

  return worked;
}

bool FastMapClient::transfering(void)
{
  return currerntPos < size;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
