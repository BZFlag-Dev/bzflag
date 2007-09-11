// unrealCTF.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

BZ_GET_PLUGIN_VERSION

class UnrealCTFEventHandler : public bz_EventHandler
{
public:
  virtual void process ( bz_EventData *eventData );
};

typedef struct 
{
  int id;
  int caried;
  float basePos[3];
}TeamFlagStatusRecord;

TeamFlagStatusRecord  redFlag;
TeamFlagStatusRecord  greenFlag;
TeamFlagStatusRecord  blueFlag;
TeamFlagStatusRecord  purpleFlag;

bool gotFlags;
void checkFlags ( void );

UnrealCTFEventHandler unrealCTFEventHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_registerEvent ( bz_eFlagGrabbedEvent, &unrealCTFEventHandler );
  bz_registerEvent ( bz_eAllowCTFCaptureEvent, &unrealCTFEventHandler );
  bz_registerEvent ( bz_eWorldFinalized, &unrealCTFEventHandler );
  bz_registerEvent ( bz_ePlayerJoinEvent, &unrealCTFEventHandler );

  if (bz_anyPlayers())
    checkFlags();
  bz_debugMessage(4,"unrealCTF plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  gotFlags = false;
  bz_removeEvent ( bz_ePlayerJoinEvent, &unrealCTFEventHandler );
  bz_removeEvent ( bz_eFlagGrabbedEvent, &unrealCTFEventHandler );
  bz_removeEvent ( bz_eAllowCTFCaptureEvent, &unrealCTFEventHandler );
  bz_removeEvent ( bz_eWorldFinalized, &unrealCTFEventHandler );
  bz_debugMessage(4,"unrealCTF plugin unloaded");
  return 0;
}

bz_eTeamType teamFromFlag ( int i )
{
  std::string name = bz_getFlagName(i).c_str();
  if ( name == "R*" )
    return eRedTeam;

  if ( name == "G*" )
    return eGreenTeam;

  if ( name == "B*" )
    return eBlueTeam;

  if ( name == "P*" )
    return ePurpleTeam;

  return eNoTeam;
}

void zapTeamFlagToBase ( bz_eTeamType team )
{
  if ( team == eRedTeam )
  {
    redFlag.caried = -1;
    bz_moveFlag ( redFlag.id, redFlag.basePos);
  }

  if ( team == eGreenTeam  )
  {
    greenFlag.caried = -1;
    bz_moveFlag ( greenFlag.id, greenFlag.basePos);
  }

  if ( team == eBlueTeam  )
  {
    blueFlag.caried = -1;
    bz_moveFlag ( blueFlag.id, blueFlag.basePos);
  }

  if ( team == ePurpleTeam  )
  {
    purpleFlag.caried = -1;
    bz_moveFlag ( purpleFlag.id, purpleFlag.basePos);
  }
}

void checkFlags ( void )
{
  if (gotFlags)
    return;

  bz_APIWorldObjectList* worldObjects = bz_getWorldObjectList();

  for ( int i = 0; i < (int)worldObjects->size(); i++)
  {
    if ( worldObjects->get(i)->type == eSolidObject)
    {
      bz_APISolidWorldObject_V1 *solid = (bz_APISolidWorldObject_V1*)worldObjects->get(i);
      if ( solid->solidType == eBaseObject )
      {
	bz_CTFBaseWorldObject_V1 *base = (bz_CTFBaseWorldObject_V1*)solid;
	switch(base->team)
	{
	  case eRedTeam:
	    memcpy(redFlag.basePos,base->center,sizeof(float)*3);
	    redFlag.basePos[2] = base->maxAABBox[2]+0.1;
	    break;

	  case eGreenTeam:
	    memcpy(greenFlag.basePos,base->center,sizeof(float)*3);
	    greenFlag.basePos[2] = base->maxAABBox[2]+0.1;
	    break;

	  case eBlueTeam:
	    memcpy(blueFlag.basePos,base->center,sizeof(float)*3);
	    blueFlag.basePos[2] = base->maxAABBox[2]+0.1;
	    break;

	   case ePurpleTeam:
	     memcpy(purpleFlag.basePos,base->center,sizeof(float)*3);
	     purpleFlag.basePos[2] = base->maxAABBox[2]+0.1;
	     break;
	}
      }
    }
  }

  bz_releaseWorldObjectList(worldObjects);

  int flagCount = bz_getNumFlags();
  for ( int i = 0; i < flagCount; i++ )
  {
    std::string name = bz_getFlagName(i).c_str();

    bz_eTeamType team = teamFromFlag(i);

    if ( team == eRedTeam )
      redFlag.id = i;

    if ( team == eGreenTeam  )
      greenFlag.id = i;

    if ( team == eBlueTeam  )
      blueFlag.id = i;

    if ( team == ePurpleTeam  )
      purpleFlag.id = i;

    zapTeamFlagToBase(team);
  }
}


void setTeamFlagPickup ( bz_eTeamType team, int player )
{
  if ( team == eRedTeam )
    redFlag.caried = player;

  if ( team == eGreenTeam  )
    greenFlag.caried = player;

  if ( team == eBlueTeam  )
    blueFlag.caried = player;

  if ( team == ePurpleTeam  )
    purpleFlag.caried = player;
}

int getTeamFlagCarier ( bz_eTeamType team)
{
  if ( team == eRedTeam )
    return redFlag.caried;

  if ( team == eGreenTeam  )
    return greenFlag.caried;

  if ( team == eBlueTeam  )
    return blueFlag.caried;

  if ( team == ePurpleTeam  )
    return purpleFlag.caried;

  return -1;
}


void UnrealCTFEventHandler::process ( bz_EventData *eventData )
{
  if (!eventData)
    return;

  switch(eventData->eventType)
  {
    case bz_ePlayerJoinEvent: // just in case we loaded, noone was here, and we didn't check the flags
      if (!gotFlags)
	checkFlags();
      break;

    case bz_eFlagGrabbedEvent:
    {
	bz_FlagGrabbedEventData_V1 *grabbed = (bz_FlagGrabbedEventData_V1*)eventData;
	bz_eTeamType flagTeam = teamFromFlag(grabbed->flagID);
	if ( flagTeam == eNoTeam)
	  break;

	bz_eTeamType playerTeam = bz_getPlayerTeam(grabbed->playerID);
	if (playerTeam == eRogueTeam || playerTeam == flagTeam)
	  zapTeamFlagToBase(flagTeam);
	else
	  setTeamFlagPickup(flagTeam,grabbed->playerID);
    }
    break;

    case bz_eAllowCTFCaptureEvent:
      {
	bz_AllowCTFCaptureEventData_V1* CTFCap = (bz_AllowCTFCaptureEventData_V1*)eventData;

	CTFCap->killTeam = false;
	
	// only let them cap if there flag is not caried
	CTFCap->allow = getTeamFlagCarier(CTFCap->teamCapping) == -1;
      }
      break;

    case bz_eWorldFinalized:
      gotFlags = false;
      checkFlags();
      break;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
