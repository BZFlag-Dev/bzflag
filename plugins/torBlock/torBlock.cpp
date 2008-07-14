// torBlock.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <stdarg.h>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <map>
#include <vector>
#include "plugin_utils.h"

#define _CSVPARSE false

BZ_GET_PLUGIN_VERSION

std::string torMasterList("http://moria.seul.org:9032/tor/status/authority");

double lastUpdateTime = -99999.0;
double updateInterval = 3600.0;

std::vector<std::string>	exitNodes;

class Handler : public bz_EventHandler
{
public:
  virtual void process ( bz_EventData *eventData );
};

Handler handler;

class MyURLHandler: public bz_BaseURLHandler
{
public:
  std::string page;

  void csvParse ( void )
  {
    exitNodes = tokenize(page,",",0,false);
  }

  void parse ( void )
  {
    std::vector<std::string> tokes = tokenize(page,std::string("\n"),0,false);

    bool gotKey = false;
    for (unsigned int i = 0; i < tokes.size(); i++ )
    {
      if (!gotKey)
      {
	if ( tokes[i] == "-----END RSA PUBLIC KEY-----")
	{
	  gotKey = true;
	  exitNodes.clear(); // only clear when we have a list
	}
      }
      else
      {
	if ( tokes[i].size() )
	{
	  std::vector<std::string> chunks = tokenize(tokes[i],std::string(" "),0,false);

	  if ( chunks.size() > 1 )
	  {
	    if ( chunks[0] == "r" && chunks.size() > 7 )
	      exitNodes.push_back(chunks[6]);
	  }
	}
      }
    }
  }

  virtual void done ( const char* /* URL */, void * data, unsigned int size, bool complete )
  {
    char *str = (char*)malloc(size+1);
    memcpy(str,data,size);
    str[size] = 0;

    page += str;
    free(str);
    if (!complete)
      return;

   if (_CSVPARSE)
     csvParse();
   else
     parse();
  }
};


class mySlashCommand : public bz_CustomSlashCommandHandler
{
public:
  virtual bool handle ( int playerID, bz_ApiString /* command */, bz_ApiString /* message */, bz_APIStringList * /* params */ )
  {
    bz_sendTextMessage(BZ_SERVER,playerID,"torBlock List");
    for ( unsigned int i = 0; i < exitNodes.size(); i++ )
      bz_sendTextMessage(BZ_SERVER,playerID,exitNodes[i].c_str());

    return true;
  }
};

mySlashCommand mySlash;
MyURLHandler myURL;

void updateTorList ( void )
{
  if ( bz_getCurrentTime() - lastUpdateTime >= updateInterval)
  {
    lastUpdateTime = bz_getCurrentTime();
    myURL.page.clear();
    bz_addURLJob(torMasterList.c_str(),&myURL);
  }
}

bool isTorAddress ( const char* addy )
{
  for ( unsigned int i = 0; i < exitNodes.size(); i++ )
  {
    if (exitNodes[i] == addy)
      return true;
  }
  return false;
}

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"torBlock plugin loaded");
  bz_registerEvent(bz_eAllowPlayer,&handler);
  bz_registerEvent(bz_eTickEvent,&handler);
  bz_registerCustomSlashCommand("torlist",&mySlash);
  lastUpdateTime = -updateInterval * 2;
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeCustomSlashCommand("torlist");
  bz_removeEvent(bz_eTickEvent,&handler);
  bz_removeEvent(bz_eAllowPlayer,&handler);
  bz_debugMessage(4,"torBlock plugin unloaded");
  return 0;
}

void Handler::process ( bz_EventData *eventData )
{
  if (!eventData)
    return;

  switch (eventData->eventType)
  {
  case bz_eAllowPlayer:
    {
      bz_AllowPlayerEventData_V1 *data = (bz_AllowPlayerEventData_V1*)eventData;

      if (isTorAddress(data->ipAddress.c_str()))
      {
	data->allow = false;
	data->reason = "Proxy Network Ban";
	bz_debugMessage(0, "Proxy Network Ban: Rejected");
      }
    }
    break;

  case bz_eTickEvent:
    updateTorList();
    break;

  default:
    break;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
