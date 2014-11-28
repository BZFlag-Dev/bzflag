// flagStay.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <map>
#include <cmath>

class FlagStayZoneHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data );
};

FlagStayZoneHandler	flagStayZoneHandler;

class EventHandler : public bz_Plugin
{
public:
	virtual const char* Name (){return "Flag Stay Zones";}
	virtual void Init ( const char* cl );
	virtual void Cleanup ();
	virtual void Event ( bz_EventData *eventData );
};

BZ_PLUGIN(EventHandler)

void EventHandler::Init ( const char* /*commandLine*/ )
{
  bz_registerCustomMapObject("FLAGSTAYZONE",&flagStayZoneHandler);

  Register(bz_ePlayerUpdateEvent);
}

void EventHandler::Cleanup ( void )
{
  Flush();
  bz_removeCustomMapObject("FLAGSTAYZONE");
}

class FlagStayZone
{
public:
	FlagStayZone()
	{
		box = false;
		xMax = xMin = yMax = yMin = zMax = zMin = rad = rot = 0;
	}

	bool box;
	float xMax,xMin,yMax,yMin,zMax,zMin;
	float rad, rot;

	std::string message;
	bool pointIn ( float pos[3] )
	{
		if ( box )
		{
            if (rot == 0 || rot == 180)
            {
                if ( pos[0] > xMax || pos[0] < xMin ) return false;
                if ( pos[1] > yMax || pos[1] < yMin ) return false;
            }
            else if (rot == 90 || rot == 270)
            {
                if ( pos[1] > xMax || pos[1] < xMin ) return false;
                if ( pos[0] > yMax || pos[0] < yMin ) return false;
            }
            else // the box is rotated, maths is needed
            {
                float pi = (float)(atan(1) * 4);
                float rotRad = (rot * pi) / 180;
                float height  = (yMax - yMin);
                float width   = (xMax - xMin);

                // Center of the rectangle, we can treat this as the origin
                float cX = (xMax + xMin) / 2;
                float cY = (yMax + yMin) / 2;

                // Coordinates of original and rotated shape
                float oX[4], oY[4], rX[4], rY[4];

                // Coordinates for the original rectangle
                oX[0] = xMin - cX; oY[0] = yMax - cY;
                oX[1] = xMax - cX; oY[1] = yMax - cY;
                oX[2] = xMax - cX; oY[2] = yMin - cY;
                oX[3] = xMin - cX; oY[3] = yMin - cY;

                // Coordinates for the rotated rectangle
                rX[0] = (float)(oX[0] * cos(rotRad) - oY[0] * sin(rotRad)); rY[0] = (float)(oX[0] * sin(rotRad) + oY[0] * cos(rotRad));
                rX[1] = (float)(oX[1] * cos(rotRad) - oY[1] * sin(rotRad)); rY[1] = (float)(oX[1] * sin(rotRad) + oY[1] * cos(rotRad));
                rX[2] = (float)(oX[2] * cos(rotRad) - oY[2] * sin(rotRad)); rY[2] = (float)(oX[2] * sin(rotRad) + oY[2] * cos(rotRad));
                rX[3] = (float)(oX[3] * cos(rotRad) - oY[3] * sin(rotRad)); rY[3] = (float)(oX[3] * sin(rotRad) + oY[3] * cos(rotRad));

                float pX = pos[0] - cX;
                float pY = pos[1] - cY;

                float apd = triangleSum(rX[0], pX, rX[3], rY[0], pY, rY[3]);
                float apb = triangleSum(rX[0], pX, rX[1], rY[0], pY, rY[1]);
                float dpc = triangleSum(rX[3], pX, rX[2], rY[3], pY, rY[2]);
                float bpc = triangleSum(rX[2], pX, rX[1], rY[2], pY, rY[1]);

                if (apd + dpc + bpc + apb > (width * height)) return false;
            }
		}
		else
		{
			float vec[3];
			vec[0] = pos[0]-xMax;
			vec[1] = pos[1]-yMax;
			vec[2] = pos[2]-zMax;

			float dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]);

			if ( dist > rad) return false;
		}

        return !(pos[2] > zMax || pos[2] < zMin);

    }

	bool checkFlag ( const char* flag )
	{
		for ( unsigned int i = 0; i < flagList.size(); i++ )
		{
			if ( flagList[i] == flag )
				return true;
		}
		return false;
	}

	std::vector<std::string> flagList;

private:
    float triangleSum(float x1, float x2, float x3, float y1, float y2, float y3)
    {
        return abs(((x1 * y2) + (x2 * y3) + (x3 * y1) - (y1 * x2) - (y2 * x3) - (y3 * x1))/2);
    }
};

std::vector <FlagStayZone> zoneList;

bool FlagStayZoneHandler::MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "FLAGSTAYZONE" || !data)
		return false;

	FlagStayZone newZone;

    // Temporary placeholders for information with default values just in case
    float pos[3], size[3], radius = 5, height = 5, rot = 0;

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);

		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

            // @TODO Possibly deprecate 'BBOX' and 'Cylinder' in favor of having the code do the calculations and in favor of staying consistent with other map objects
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
            else if ((key == "POSITION" || key == "POS") && nubs->size() > 3)
            {
                pos[0] = {(float)atof(nubs->get(1).c_str())};
                pos[1] = {(float)atof(nubs->get(2).c_str())};
                pos[2] = {(float)atof(nubs->get(3).c_str())};
            }
            else if (key == "SIZE" && nubs->size() > 3)
            {
                newZone.box = true;
                size[0] = {(float)atof(nubs->get(1).c_str())};
                size[1] = {(float)atof(nubs->get(2).c_str())};
                size[2] = {(float)atof(nubs->get(3).c_str())};
            }
            else if ((key == "ROTATION" || key == "ROT") && nubs->size() > 1)
            {
                rot = (float)atof(nubs->get(1).c_str());
            }
            else if (key == "RADIUS" && nubs->size() > 1)
            {
                newZone.box = false;
                radius = (float)atof(nubs->get(1).c_str());
            }
            else if (key == "HEIGHT" && nubs->size() > 1)
            {
                height = (float)atof(nubs->get(1).c_str());
            }
			else if ( key == "FLAG" && nubs->size() > 1)
			{
				std::string flag = bz_toupper(nubs->get(1).c_str());
				newZone.flagList.push_back(flag);
			}
			else if ( key == "MESSAGE" && nubs->size() > 1 )
			{
				newZone.message = nubs->get(1).c_str();
			}
		}
		bz_deleteStringList(nubs);
	}

    if (newZone.box)
    {
        newZone.xMin = pos[0] - size[0];
        newZone.xMax = pos[0] + size[0];
        newZone.yMin = pos[1] - size[1];
        newZone.yMax = pos[1] + size[1];
        newZone.zMin = pos[2];
        newZone.zMax = pos[2] + size[2];
        newZone.rot  = (rot > 0 && rot < 360) ? rot : 0;
    }
    else
    {
        newZone.rad = radius;
        newZone.xMax = pos[0];
        newZone.yMax = pos[1];
        newZone.zMin = pos[2];
        newZone.zMax = pos[2] + height;
    }

	zoneList.push_back(newZone);
	return true;
}

std::map<int,int>	playerIDToZoneMap;

void EventHandler::Event ( bz_EventData *eventData )
{
	float pos[3] = {0};

	int playerID = -1;

	switch (eventData->eventType)
	{
	case bz_ePlayerUpdateEvent:
		pos[0] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[0];
		pos[1] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[1];
		pos[2] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[2];
		playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
		break;

	case bz_eShotFiredEvent:
		pos[0] = ((bz_ShotFiredEventData_V1*)eventData)->pos[0];
		pos[1] = ((bz_ShotFiredEventData_V1*)eventData)->pos[1];
		pos[2] = ((bz_ShotFiredEventData_V1*)eventData)->pos[2];
		playerID = ((bz_ShotFiredEventData_V1*)eventData)->playerID;
		break;

	default:
		return;
	}

	const char* flagAbrev = bz_getPlayerFlag(playerID);
	if (!flagAbrev)
		return;

	std::vector<FlagStayZone*> validZones;

	// check and see if a zone cares about the current flag
	for ( unsigned int i = 0; i < zoneList.size(); i++ )
	{
		if ( zoneList[i].checkFlag(flagAbrev) )
			validZones.push_back(&zoneList[i]);
	}

	// Check each zone for this flag to see if we are in one
	bool insideOne = false;
	for ( unsigned int i = 0; i < validZones.size(); i++ )
	{
		if ( validZones[i]->pointIn(pos) )
		{
			insideOne = true;
			playerIDToZoneMap[playerID] = i;
		}
	}

	// if they have taken the flag out of a zone, pop it.
	if (!insideOne && validZones.size() > 0)
	{
		int lastZone = -1;
		if ( playerIDToZoneMap.find(playerID) != playerIDToZoneMap.end() )
			lastZone = playerIDToZoneMap[playerID];
		bz_removePlayerFlag(playerID);
		if (lastZone != -1 && zoneList[lastZone].message.size())
			bz_sendTextMessage(BZ_SERVER,playerID,zoneList[lastZone].message.c_str());
	}
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

