// CustomZoneSample.cpp : Defines the entry point for the DLL application.
//
#include <vector>

#include "bzfsAPI.h"
#include "plugin_utils.h"

// The custom zone we'll be making for this plug-in and we'll be extending a class provided by the API.
// The class provided in the API handles all of the logic handling rectangular and circular zones.
class MsgZone : public bz_CustomZoneObject
{
public:
  // Our custom constructor will call the parent constructor so we can setup default positions
  // for the zone
  MsgZone() : bz_CustomZoneObject()
  {
    message = "";
    flag = "US";
  }

  // Custom fields that are unique to our zone so we can build on top of the class we're extending
  std::string message;
  std::string flag;
};

class CustomZoneSample : public bz_Plugin, bz_CustomMapObjectHandler
{
public:
  virtual const char* Name () {return "Custom Zone Sample";}
  virtual void Init (const char* config);
  virtual void Event (bz_EventData *eventData);
  virtual void Cleanup (void);

  virtual bool MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data);

  std::vector<MsgZone> msgZones;

  bool messageSentTo[256];
};

BZ_PLUGIN(CustomZoneSample)

void CustomZoneSample::Init (const char* commandLine)
{
  Register(bz_ePlayerUpdateEvent);

  // Whenever a player enters a zone and is carrying a specified flag, they will receive the specified message
  bz_registerCustomMapObject("msgzone", this);
}

void CustomZoneSample::Cleanup (void)
{
  Flush();

  bz_removeCustomMapObject("msgzone");
}

bool CustomZoneSample::MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data)
{
  if (object != "MSGZONE" || !data)
      return false;

  // The new zone we just found and we'll be storing in our vector of zones
  MsgZone newZone;

  // This function will parse the attributes that are handled by bz_CustomZoneObject which
  // handles rectangular and circular zones
  //
  // For rectangular zones:
  //   - position
  //   - size
  //   - rotation
  //
  // For circular zones:
  //   - position
  //   - height
  //   - radius
  //
  // This also handles BBOX and CYLINDER fields but they have been deprecated and will be
  // removed in the future
  newZone.handleDefaultOptions(data);

  // Loop through the object data
  for (unsigned int i = 0; i < data->data.size(); i++)
  {
    std::string line = data->data.get(i).c_str();

    bz_APIStringList *nubs = bz_newStringList();
    nubs->tokenize(line.c_str(), " ", 0, true);

    if (nubs->size() > 0)
    {
      std::string key = bz_toupper(nubs->get(0).c_str());

      // These are our custom fields in the MsgZone class
      if (key == "MESSAGE" && nubs->size() > 1)
      {
	newZone.message = nubs->get(1).c_str();
      }
      else if (key == "FLAG" && nubs->size() > 1)
      {
	newZone.flag = nubs->get(1).c_str();
      }
    }

    bz_deleteStringList(nubs);
  }

  msgZones.push_back(newZone);

  return true;
}

void CustomZoneSample::Event (bz_EventData *eventData)
{
  switch (eventData->eventType)
  {
    case bz_ePlayerUpdateEvent: // This event is called each time a player sends an update to the server
    {
      bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;

      // Loop through all of our custom zones
      for (int i = 0; i < msgZones.size(); i++)
      {
	// Use the pointInZone(float pos[3]) function provided by the bz_CustomZoneObject to check if the position
	// of the player is inside of the zone. This function will automatically handle the logic if the zone is a
	// rectangle (even if it's rotated) or a circle
	if (msgZones[i].pointInZone(updateData->state.pos) && bz_getPlayerFlagID(updateData->playerID) >= 0)
	{
	  // If the player has the flag specified in the zone, send them a message and remove their flag
	  if (strcmp(bz_getPlayerFlag(updateData->playerID), msgZones[i].flag.c_str()) == 0)
	  {
	    bz_sendTextMessage(BZ_SERVER, updateData->playerID, msgZones[i].message.c_str());
	    bz_removePlayerFlag(updateData->playerID);
	  }
	}
      }
    }
    break;

    default: break;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
