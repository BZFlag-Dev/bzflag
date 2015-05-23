// wwzones.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"
#include <map>
#include <cmath>

class WWINFO
{
public:
	WWINFO()
	{
		pi = 3.14159265358979323846;
		tickTime = 0.5;
	}
	double pi;
	double tickTime;
};

WWINFO wwinfo;

class WWZEventHandler : public bz_Plugin, bz_CustomMapObjectHandler
{
public:

	virtual const char* Name (){return "World Weapon Zones";}
	virtual void Init ( const char* config);
	virtual void Cleanup ();
	virtual void Event ( bz_EventData *eventData );

	virtual bool MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data );

};

BZ_PLUGIN(WWZEventHandler)

void WWZEventHandler::Init (const char* /*commandLineParameter*/){

	bz_registerCustomMapObject("WWZONE",this);
	Register(bz_eTickEvent);

	MaxWaitTime = (float)wwinfo.tickTime;
}

void WWZEventHandler::Cleanup (void){

	Flush();
	bz_removeCustomMapObject("WWZONE");
}

class WWZPlyrInfo
{
public:
	WWZPlyrInfo()
	{
		wwzplyrID = -1;
		wwzPlyrInTime = 0;
	}

	int wwzplyrID;
	double wwzPlyrInTime;
};

WWZPlyrInfo wwzNewPlyr;

class WWZone : public bz_CustomZoneObject
{
public:
	WWZone() : bz_CustomZoneObject()
	{
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
		wwzPlyrList.clear();
	}

	std::vector <WWZPlyrInfo> wwzPlyrList;

	bz_ApiString zoneWeapon;
	float zoneWeaponLifetime, zoneWeaponPosition[3], zoneWeaponTilt, zoneWeaponDirection, zoneWeaponDT;
	double zoneWeaponMinFireTime, zoneWeaponTimeDelay, zoneWeaponLastFired;
	bool zoneWeaponRepeat, zoneWeaponInfoMessage, zoneWeaponFired, zoneWeaponSentMessage;
	int zoneWeaponShotID;

	std::string playermessage;
	std::string servermessage;
};

std::vector <WWZone> zoneList;

bool WWZEventHandler::MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "WWZONE" || !data)
		return false;

	WWZone newZone;
	newZone.handleDefaultOptions(data);

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);

		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if ( key == "ZONEWEAPON" && nubs->size() > 9)
			{
				newZone.zoneWeapon = nubs->get(1);
				newZone.zoneWeaponLifetime = (float)atof(nubs->get(2).c_str());
				newZone.zoneWeaponPosition[0] = (float)atof(nubs->get(3).c_str());
				newZone.zoneWeaponPosition[1] = (float)atof(nubs->get(4).c_str());
				newZone.zoneWeaponPosition[2] = (float)atof(nubs->get(5).c_str());
				newZone.zoneWeaponTilt = (float)atof(nubs->get(6).c_str());
				newZone.zoneWeaponTilt = (newZone.zoneWeaponTilt / 360) * (2 * (float)wwinfo.pi);
				newZone.zoneWeaponDirection = (float)atof(nubs->get(7).c_str());
				newZone.zoneWeaponDirection = (newZone.zoneWeaponDirection / 360) * (2 * (float)wwinfo.pi);
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
				if (newZone.zoneWeaponMinFireTime < wwinfo.tickTime && newZone.zoneWeaponMinFireTime >= 0.1)
					wwinfo.tickTime = newZone.zoneWeaponMinFireTime - .05;  //tick faster than ww calls
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

	MaxWaitTime = (float)wwinfo.tickTime;
	return true;
}

inline bool wasHere(int zoneNum, int plyrNum)
{
	for (unsigned int j = 0; j < zoneList[zoneNum].wwzPlyrList.size(); j++)
	{
		if (plyrNum == zoneList[zoneNum].wwzPlyrList[j].wwzplyrID)
			return true;
	}

	wwzNewPlyr.wwzplyrID = plyrNum;
	wwzNewPlyr.wwzPlyrInTime = bz_getCurrentTime();

	zoneList[zoneNum].wwzPlyrList.push_back(wwzNewPlyr);
	zoneList[zoneNum].zoneWeaponSentMessage = false;
	zoneList[zoneNum].zoneWeaponFired = false;

	return false;
}

inline void notHere(int zoneNum, int plyrNum)
{
	for (unsigned int j = 0; j < zoneList[zoneNum].wwzPlyrList.size(); j++)
	{
		if (plyrNum == zoneList[zoneNum].wwzPlyrList[j].wwzplyrID)
		{
			zoneList[zoneNum].wwzPlyrList.erase(zoneList[zoneNum].wwzPlyrList.begin() + j);
			zoneList[zoneNum].zoneWeaponFired = false;
			zoneList[zoneNum].zoneWeaponSentMessage = false;
			return;
		}
	}

	return;
}

inline bool OKToFire(int zoneNum, int plyrNum)
{
	for (unsigned int j = 0; j < zoneList[zoneNum].wwzPlyrList.size(); j++)
	{
		if (zoneList[zoneNum].wwzPlyrList[j].wwzplyrID == plyrNum)
		{
			if ((bz_getCurrentTime() - zoneList[zoneNum].wwzPlyrList[j].wwzPlyrInTime) > zoneList[zoneNum].zoneWeaponTimeDelay && !zoneList[zoneNum].zoneWeaponFired)
			{
				zoneList[zoneNum].wwzPlyrList[j].wwzPlyrInTime = bz_getCurrentTime();
				return true;
			}
		}
	}

	return false;
}

void WWZEventHandler::Event ( bz_EventData *eventData )
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
					if (zoneList[i].pointInZone(player->lastKnownState.pos) && player->spawned)
					{
						if (wasHere(i, player->playerID) && OKToFire(i, player->playerID) && !zoneList[i].zoneWeaponFired)
						{
							bz_fireWorldWep (zoneList[i].zoneWeapon.c_str(),zoneList[i].zoneWeaponLifetime,BZ_SERVER,zoneList[i].zoneWeaponPosition,zoneList[i].zoneWeaponTilt,zoneList[i].zoneWeaponDirection,zoneList[i].zoneWeaponShotID,zoneList[i].zoneWeaponDT);
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
				bz_freePlayerRecord(player);
			}
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

