// fixedspawn.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "TextUtils.h"

BZ_GET_PLUGIN_VERSION

class SpawnHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

SpawnHandler	spawnHandler;

class SpawnObject : public bz_CustomMapObjectHandler
{
public:
	virtual bool handle ( bzApiString object, bz_CustomMapObjectInfo *data );
};
SpawnObject	spawnObject;

typedef struct 
{
	float x,y,z;
}trSpawnPos;

std::vector<trSpawnPos> spawnList;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"fixedspawn plugin loaded");
	
	bz_registerEvent ( bz_eGetPlayerSpawnPosEvent, &spawnHandler );
	bz_registerCustomMapObject("spawn",&spawnObject);
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent (bz_eGetPlayerSpawnPosEvent, &spawnHandler );
	bz_removeCustomMapObject("spawn");
	bz_debugMessage(4,"fixedspawn plugin unloaded");
	return 0;
}

bool SpawnObject::handle ( bzApiString object, bz_CustomMapObjectInfo *data )
{
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		std::vector<std::string> nubs = TextUtils::tokenize(line," ");

		std::string tag = nubs[0];
		if ( nubs[0] == "position" )
		{
			if ( nubs.size()>2)
			{
				trSpawnPos	pos;
				pos.x = (float)atof(nubs[1].c_str());
				pos.y = (float)atof(nubs[2].c_str());
				if ( nubs.size() > 3)
					pos.z = (float)atof(nubs[3].c_str());
				else
					pos.z = 0;

				spawnList.push_back(pos);
			}
		}
	}
	return true;
}

void SpawnHandler::process ( bz_EventData *eventData )
{
	if (eventData->eventType == bz_eGetPlayerSpawnPosEvent )
	{
		bz_GetPlayerSpawnPosEventData *spawnData = (bz_GetPlayerSpawnPosEventData*)eventData;

		if ( spawnList.size() )
		{
			unsigned int index = rand()%(unsigned int)spawnList.size();
			spawnData->pos[0] = spawnList[index].x;
			spawnData->pos[1] = spawnList[index].y;
			spawnData->pos[2] = spawnList[index].z;

			spawnData->rot = ((float)rand()/(float)RAND_MAX)* 360.0f;
			spawnData->handled = true;
		}
	}
}

