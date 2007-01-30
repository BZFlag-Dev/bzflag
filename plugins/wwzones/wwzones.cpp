// wwzones.cpp : Defines the entry point for the DLL application.
// (code modified from JeffM2501's flayStay plugin)
// Version 2.0

#include "bzfsAPI.h"
#include <string>
#include <vector>
#include <map>
#include <math.h>

double pi = 3.14159265358979323846;
double minTickTime = 0.5;

BZ_GET_PLUGIN_VERSION

class WWZoneHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool handle ( bz_ApiString object, bz_CustomMapObjectInfo *data );
};

WWZoneHandler	wwzonehandler;

class EventHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

EventHandler eventHandler;

BZF_PLUGIN_CALL int bz_Load (const char* /*commandLineParameter*/){

	bz_debugMessage(4,"wwzones plugin loaded");
	bz_registerCustomMapObject("WWZONE",&wwzonehandler);
	bz_registerEvent(bz_eTickEvent,&eventHandler);
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload (void){

	bz_removeEvent(bz_eTickEvent,&eventHandler);
	bz_debugMessage(4,"wwzones plugin unloaded");
	bz_removeCustomMapObject("WWZONE");
	return 0;
}

class PlyrInfo
{
public:
	PlyrInfo()
	{
		plyrID = -1;
		plyrInTime = 0;
	}

	int plyrID;
	double plyrInTime;
};

PlyrInfo newPlyr;

class WWZone
{
public:
	WWZone()
	{
		box = false;
		xMax = xMin = yMax = yMin = zMax = zMin = rad = 0;
		zoneWeapon = "";
		zoneWeaponLifetime = 0;
		zoneWeaponPosition[0] = 0;
		zoneWeaponPosition[1] = 0;
		zoneWeaponPosition[2] = 0;
		zoneWeaponTilt = 0;
		zoneWeaponDirection = 0;
		zoneWeaponShotID = 0;
		zoneWeaponDT = 0;
		zoneWeaponRepeat = false;
		zoneWeaponFired = false;
		zoneWeaponLastFired = 0;
		zoneWeaponMinFireTime = 0.5;
		zoneWeaponTimeDelay = 0;
		zoneWeaponInfoMessage = false;
		zoneWeaponSentMessage = false;
		plyrList.clear();
	}

	std::vector <PlyrInfo> plyrList;
	bool box;
	float xMax,xMin,yMax,yMin,zMax,zMin;
	float rad;

	bz_ApiString zoneWeapon;
	float zoneWeaponLifetime, zoneWeaponPosition[3], zoneWeaponTilt, zoneWeaponDirection, zoneWeaponDT;
	double zoneWeaponMinFireTime, zoneWeaponTimeDelay, zoneWeaponLastFired;
	bool zoneWeaponRepeat, zoneWeaponInfoMessage, zoneWeaponFired, zoneWeaponSentMessage;
	int zoneWeaponShotID;

	std::string playermessage;
	std::string servermessage;

	bool pointIn ( float pos[3] )
	{
		if ( box )
		{
			if ( pos[0] > xMax || pos[0] < xMin )
				return false;

			if ( pos[1] > yMax || pos[1] < yMin )
				return false;

			if ( pos[2] > zMax || pos[2] < zMin )
				return false;
		}
		else
		{
			float vec[3];
			vec[0] = pos[0]-xMax;
			vec[1] = pos[1]-yMax;
			vec[2] = pos[2]-zMax;

			float dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]);
			if ( dist > rad)
				return false;

			if ( pos[2] > zMax || pos[2] < zMin )
				return false;
		}
		return true;
	}
};

std::vector <WWZone> zoneList;

bool WWZoneHandler::handle ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "WWZONE" || !data)
		return false;

	WWZone newZone;

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);
		
		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if ( key == "BBOX" && nubs->size() > 6)
			{
				newZone.box = true;
				newZone.xMin = (float)atof(nubs->get(1).c_str());
				newZone.xMax = (float)atof(nubs->get(2).c_str());
				newZone.yMin = (float)atof(nubs->get(3).c_str());
				newZone.yMax = (float)atof(nubs->get(4).c_str());
				newZone.zMin = (float)atof(nubs->get(5).c_str());
				newZone.zMax = (float)atof(nubs->get(6).c_str());
			}
			else if ( key == "CYLINDER" && nubs->size() > 5)
			{
				newZone.box = false;
				newZone.rad = (float)atof(nubs->get(5).c_str());
				newZone.xMax =(float)atof(nubs->get(1).c_str());
				newZone.yMax =(float)atof(nubs->get(2).c_str());
				newZone.zMin =(float)atof(nubs->get(3).c_str());
				newZone.zMax =(float)atof(nubs->get(4).c_str());
			}
			else if ( key == "ZONEWEAPON" && nubs->size() > 9)
			{
				newZone.zoneWeapon = nubs->get(1);
				newZone.zoneWeaponLifetime = (float)atof(nubs->get(2).c_str());
				newZone.zoneWeaponPosition[0] = (float)atof(nubs->get(3).c_str());
				newZone.zoneWeaponPosition[1] = (float)atof(nubs->get(4).c_str());
				newZone.zoneWeaponPosition[2] = (float)atof(nubs->get(5).c_str());
				newZone.zoneWeaponTilt = (float)atof(nubs->get(6).c_str());
				newZone.zoneWeaponTilt = (newZone.zoneWeaponTilt / 360) * (2 * (float)pi);
				newZone.zoneWeaponDirection = (float)atof(nubs->get(7).c_str());
				newZone.zoneWeaponDirection = (newZone.zoneWeaponDirection / 360) * (2 * (float)pi);
				newZone.zoneWeaponShotID = (int)atoi(nubs->get(8).c_str());
				newZone.zoneWeaponDT = (float)atof(nubs->get(9).c_str());
			}
			else if ( key == "REPEAT" && nubs->size() == 1 )
			{
				newZone.zoneWeaponRepeat = true;
			}
			else if ( key == "REPEAT" && nubs->size() > 1 )
			{
				newZone.zoneWeaponRepeat = true;
				newZone.zoneWeaponMinFireTime = (double)atof(nubs->get(1).c_str());
				if (newZone.zoneWeaponMinFireTime < minTickTime && newZone.zoneWeaponMinFireTime >= 0.1)
					minTickTime = newZone.zoneWeaponMinFireTime - .05;  //tick faster than ww calls
			}
			else if ( key == "TIMEDELAY" && nubs->size() > 1 )
			{
				newZone.zoneWeaponTimeDelay = (double)atof(nubs->get(1).c_str());
				if (newZone.zoneWeaponTimeDelay < 0)
					newZone.zoneWeaponTimeDelay = 0;  
			}
			else if ( key == "PLAYERMESSAGE" && nubs->size() > 1 )
			{
				newZone.playermessage = nubs->get(1).c_str();
			}
			else if ( key == "SERVERMESSAGE" && nubs->size() > 1 )
			{
				newZone.servermessage = nubs->get(1).c_str();
			}
			else if ( key == "INFOMESSAGE" )
			{
				newZone.zoneWeaponInfoMessage = true;
			}
		}
		bz_deleteStringList(nubs);
	}
	zoneList.push_back(newZone);
	bz_setMaxWaitTime ( (float)minTickTime );
	return true;
}

inline bool wasHere(int zoneNum, int plyrNum)
{
	for (unsigned int j = 0; j < zoneList[zoneNum].plyrList.size(); j++)
	{
		if (plyrNum == zoneList[zoneNum].plyrList[j].plyrID)
			return true;
	}

	newPlyr.plyrID = plyrNum;
	newPlyr.plyrInTime = bz_getCurrentTime();

	zoneList[zoneNum].plyrList.push_back(newPlyr);
	zoneList[zoneNum].zoneWeaponSentMessage = false;
	zoneList[zoneNum].zoneWeaponFired = false;

	return false;
}

inline void notHere(int zoneNum, int plyrNum)
{
	for (unsigned int j = 0; j < zoneList[zoneNum].plyrList.size(); j++)
	{
		if (plyrNum == zoneList[zoneNum].plyrList[j].plyrID)
		{
			zoneList[zoneNum].plyrList.erase(zoneList[zoneNum].plyrList.begin() + j);
			zoneList[zoneNum].zoneWeaponFired = false;
			zoneList[zoneNum].zoneWeaponSentMessage = false;
			return;
		}
	}

	return;
}

inline bool OKToFire(int zoneNum, int plyrNum)
{
	for (unsigned int j = 0; j < zoneList[zoneNum].plyrList.size(); j++)
	{
		if (zoneList[zoneNum].plyrList[j].plyrID == plyrNum)
		{
			if ((bz_getCurrentTime() - zoneList[zoneNum].plyrList[j].plyrInTime) > zoneList[zoneNum].zoneWeaponTimeDelay && !zoneList[zoneNum].zoneWeaponFired)
			{
				zoneList[zoneNum].plyrList[j].plyrInTime = bz_getCurrentTime();
				return true;
			}
		}
	}

	return false;
}

void EventHandler::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_eTickEvent)
		return;

	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int h = 0; h < playerList->size(); h++ )
	{ 
    	bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](h)); 

			if (player){

				for ( unsigned int i = 0; i < zoneList.size(); i++ )
				{
					if (zoneList[i].pointIn(player->pos) && player->spawned)
					{
						if (wasHere(i, player->playerID) && OKToFire(i, player->playerID) && !zoneList[i].zoneWeaponFired)
						{
							bz_fireWorldWep (zoneList[i].zoneWeapon.c_str(),zoneList[i].zoneWeaponLifetime,zoneList[i].zoneWeaponPosition,zoneList[i].zoneWeaponTilt,zoneList[i].zoneWeaponDirection,zoneList[i].zoneWeaponShotID,zoneList[i].zoneWeaponDT);
							zoneList[i].zoneWeaponFired = true;
							zoneList[i].zoneWeaponLastFired = bz_getCurrentTime();
						}
						else
						{
							if ((bz_getCurrentTime() - zoneList[i].zoneWeaponLastFired) > zoneList[i].zoneWeaponMinFireTime && zoneList[i].zoneWeaponRepeat)
							zoneList[i].zoneWeaponFired = false;
						}

						if (!zoneList[i].zoneWeaponSentMessage && zoneList[i].zoneWeaponFired)
						{
							if (!zoneList[i].playermessage.empty())
								bz_sendTextMessage(BZ_SERVER,player->playerID,zoneList[i].playermessage.c_str());
							if (!zoneList[i].servermessage.empty())
								bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,zoneList[i].servermessage.c_str());
							if (zoneList[i].zoneWeaponInfoMessage)
								bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s triggered by %s.", zoneList[i].zoneWeapon.c_str(), player->callsign.c_str());

							zoneList[i].zoneWeaponSentMessage = true;
						}
					}
					else
						notHere(i, player->playerID);
				}
			}
			bz_freePlayerRecord(player);
	}
		
	bz_deleteIntList(playerList); 
	return;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

