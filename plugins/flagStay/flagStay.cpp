// flagStay.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <algorithm>
#include <map>
#include <cmath>

class FlagStayZone : public bz_CustomZoneObject
{
public:
  FlagStayZone() : bz_CustomZoneObject() {}

  std::string message;

  bool checkFlag (const char* flag)
  {
    return (std::find(flagList.begin(), flagList.end(), flag) != flagList.end());
  }

  std::vector<std::string> flagList;
};

std::vector<FlagStayZone> zoneList;

class FlagStay : public bz_Plugin, bz_CustomMapObjectHandler
{
public:
  virtual const char* Name () { return "Flag Stay Zones"; }
  virtual void Init (const char* cl);
  virtual void Cleanup ();
  virtual void Event (bz_EventData *eventData);
  virtual bool MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data);
};

BZ_PLUGIN(FlagStay)

void FlagStay::Init (const char* /*commandLine*/)
{
  bz_registerCustomMapObject("FLAGSTAYZONE", this);

  Register(bz_eFlagGrabbedEvent);
  Register(bz_ePlayerUpdateEvent);
}

void FlagStay::Cleanup (void)
{
  Flush();

  bz_removeCustomMapObject("FLAGSTAYZONE");
}

bool FlagStay::MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data)
{
  if (object != "FLAGSTAYZONE" || !data) {
    return false;
  }

  FlagStayZone newZone;
  newZone.handleDefaultOptions(data);

  // parse all the chunks
  for (unsigned int i = 0; i < data->data.size(); i++) {
    std::string line = data->data.get(i).c_str();

    bz_APIStringList *nubs = bz_newStringList();
    nubs->tokenize(line.c_str()," ",0,true);

    if (nubs->size() > 0) {
      std::string key = bz_toupper(nubs->get(0).c_str());

      if (key == "FLAG" && nubs->size() > 1) {
	std::string flag = bz_toupper(nubs->get(1).c_str());
	newZone.flagList.push_back(flag);
      }
      else if ((key == "MESSAGE" || key == "MSG") && nubs->size() > 1) {
	newZone.message = nubs->get(1).c_str();
      }
    }

    bz_deleteStringList(nubs);
  }

  zoneList.push_back(newZone);

  return true;
}

std::map<int,int> playerIDToZoneMap;

void FlagStay::Event ( bz_EventData *eventData )
{
  switch (eventData->eventType)
  {
    case bz_eFlagGrabbedEvent:
    {
      bz_FlagGrabbedEventData_V1* flagGrabData = (bz_FlagGrabbedEventData_V1*)eventData;

      for (unsigned int i = 0; i < zoneList.size(); i++) {
	if (zoneList[i].pointInZone(flagGrabData->pos) && zoneList[i].checkFlag(flagGrabData->flagType)) {
	  playerIDToZoneMap[flagGrabData->playerID] = i;
	  break;
	}
      }
    }
    break;

    case bz_ePlayerUpdateEvent:
    {
      bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
      int playerID = updateData->playerID;
      float *pos = updateData->state.pos;

      const char* flagAbrev = bz_getPlayerFlag(playerID);

      if (!flagAbrev) {
	playerIDToZoneMap[playerID] = -1;
	return;
      }

      if (playerIDToZoneMap[playerID] >= 0) {
	FlagStayZone &zone = zoneList.at(playerIDToZoneMap[playerID]);

	if (!zone.pointInZone(pos)) {
	  bz_removePlayerFlag(playerID);
	  playerIDToZoneMap[playerID] = -1;

	  if (zone.message.size())
	    bz_sendTextMessage(BZ_SERVER, playerID, zone.message.c_str());
	}
      }
    }
    break;

    default: break;
  }
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
