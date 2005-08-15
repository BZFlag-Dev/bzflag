// vocaliser.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "TextUtils.h"
#include <string>
#include <map>
#include <vector>

BZ_GET_PLUGIN_VERSION

typedef struct 
{
	bool		team;
	std::string comand;
	std::string url;
	std::string sound;
	std::string text;
}trVoiceItem;

typedef struct 
{
	std::string prefix;
	std::string name;
	std::string description;
	std::map<std::string,trVoiceItem> items;
}trVoiceSet;

std::map<std::string,trVoiceSet> mVoices;

typedef struct 
{
	int	playerID;
	std::string callsign;
	std::string voice;

	int lastVoiceTime;
}trPlayerVoiceRecord;

std::map<int,trPlayerVoiceRecord> playerVoices;

class PlaysndCommand : public bz_CustomSlashCommandHandler
{
public:
	virtual ~PlaysndCommand(){};
	virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *param );
};

PlaysndCommand	playsndCommand;

// event handler callback
class VocaliserEvents : public bz_EventHandler
{
public:
	virtual ~VocaliserEvents(){};
	virtual void process ( bz_EventData *eventData );
};

VocaliserEvents vocEvents;

std::vector<std::string> resourceList;

double minVoiceTime = 45.0;

void loadVoiceProfiles ( std::string configFile )
{
	FILE *fp = fopen(configFile.c_str(),"rt");
	if (!fp)
	{
		bz_debugMessage(0,"vocaliser plugin confg file load failed");
		return;
	}
	
	fseek(fp,0,SEEK_END);
	unsigned int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char *p = (char*)malloc(size+1);
	fread(p,size,1,fp);
	fclose(fp);
	p[size] = 0;

	std::string file = p;
	free(p);

	std::vector<std::string> lines = TextUtils::tokenize(file,std::string("\n"));

	playerVoices.clear();
	resourceList.clear();

	trVoiceSet theVoiceSet;
	std::string URLBase;
	std::string URLExtension;

	for ( unsigned int i = 0; i < lines.size(); i++ )
	{
		if (lines[i].size())
		{
			std::vector<std::string> commands = TextUtils::tokenize(file,std::string(" "),1,true);
			if ( commands.size() > 1)
			{
				std::string command = TextUtils::tolower(commands[0]);

				if ( command == "voice")
				{
					theVoiceSet.items.clear();
					theVoiceSet.name = commands[1];
					theVoiceSet.prefix = "";
					theVoiceSet.description = "";

					URLBase = "";
					URLExtension = "";
				}

				if ( command == "description")
					theVoiceSet.description = commands[1];

				if ( command == "urlbase")
					URLBase = commands[1];

				if ( command == "urlextension")
					URLExtension = commands[1];

				if ( command == "prefix")
					theVoiceSet.prefix = commands[1];

				if ( command == "team" || command == "all")
				{
					trVoiceItem	item;
					item.team = command == "team";
					std::vector<std::string> args = TextUtils::tokenize(commands[1],std::string(" "),0,true);
					if (args.size() >2)
					{
						item.comand = args[0];
						item.sound = theVoiceSet.prefix+args[0];
						item.text = args[1];
						item.url = URLBase + item.sound + "." + URLExtension;

						resourceList.push_back(item.url);
						theVoiceSet.items[TextUtils::tolower(item.comand)] = item;
					}
				}

				if ( command == "endvoice")
					mVoices[theVoiceSet.name] = theVoiceSet;
			}
		}
	}

}

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"vocaliser plugin loaded");

	bz_registerCustomSlashCommand("playsnd",&playsndCommand);
	bz_registerCustomSlashCommand("setvoice",&playsndCommand);
	bz_registerCustomSlashCommand("listvoices",&playsndCommand);
	bz_registerCustomSlashCommand("listvoiceitems",&playsndCommand);

	bz_registerEvent(bz_ePlayerJoinEvent,&vocEvents);
	bz_registerEvent(bz_ePlayerPartEvent,&vocEvents);

	loadVoiceProfiles(std::string(commandLine));

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeCustomSlashCommand("playsnd");
	bz_removeCustomSlashCommand("setvoice");

	bz_removeEvent(bz_ePlayerJoinEvent,&vocEvents);
	bz_removeEvent(bz_ePlayerPartEvent,&vocEvents);

	bz_debugMessage(4,"vocaliser plugin unloaded");
	return 0;
}

trPlayerVoiceRecord& getPlayerVoiceRecord ( int playerID )
{
	if (playerVoices.find(playerID) == playerVoices.end())
	{
		trPlayerVoiceRecord	rec;
		rec.playerID = playerID;
		rec.lastVoiceTime = -1;
		rec.voice = mVoices.begin()->first;
		playerVoices[playerID] = rec;
	}

	return playerVoices[playerID];
}

void clearPlayerVoiceRecord ( int playerID )
{
	if (playerVoices.find(playerID) != playerVoices.end())
		playerVoices.erase(playerVoices.find(playerID));
}

bool PlaysndCommand::handle ( int playerID, bzApiString _command, bzApiString _message, bzAPIStringList *_param )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();

	double time = bz_getCurrentTime();
	if (!mVoices.size())
	{
		bz_sendTextMessage (BZ_SERVER, playerID, "There are no voices loaded");
		return true;
	}

	if ( command == "listvoices")
	{
		std::map<std::string,trVoiceSet>::iterator itr = mVoices.begin();
		bz_sendTextMessage (BZ_SERVER, playerID, "Available voices;");
		while (itr != mVoices.end())
		{
			bz_sendTextMessage (BZ_SERVER, playerID, itr->first.c_str());
			itr++;
		}
		return true;
	}

	if ( command == "listvoiceitems")
	{
		trPlayerVoiceRecord &voice = getPlayerVoiceRecord (playerID);

		// lets find the command in the voice

		trVoiceSet &voiceSet = mVoices[voice.voice];
		
		std::map<std::string,trVoiceItem>::iterator itr = voiceSet.items.begin();
		bz_sendTextMessage (BZ_SERVER, playerID, "Available voice items;");
		while (itr != voiceSet.items.end())
		{
			bz_sendTextMessage (BZ_SERVER, playerID, itr->first.c_str());
			itr++;
		}
		return true;
	}

	if ( command == "setvoice")
	{
		trPlayerVoiceRecord &voice = getPlayerVoiceRecord (playerID);

		if (mVoices.find(message) == mVoices.end())
		{
			bz_sendTextMessage (BZ_SERVER, playerID, "The requested voice profile does not exist");
			return true;
		}
		else
		{
			voice.voice = message;
			bz_sendTextMessage (BZ_SERVER, playerID, "Your voice profile has been set");
			return true;
		}
	}

	if ( command == "playsnd" )
	{
		trPlayerVoiceRecord &voice = getPlayerVoiceRecord (playerID);
		
		// lets find the command in the voice
		if ( voice.lastVoiceTime != -1 )
		{
			if (time - voice.lastVoiceTime < minVoiceTime)
			{
				bz_sendTextMessage (BZ_SERVER, playerID, "You just said something, wait a bit");
				return true;
			}
		}

		trVoiceSet &voiceSet = mVoices[voice.voice];
		
		if (voiceSet.items.find(TextUtils::tolower(message)) == voiceSet.items.end())
		{
			bz_sendTextMessage (BZ_SERVER, playerID, "That voice message is not part of your voice set");
			return true;
		}

		trVoiceItem &item = voiceSet.items[TextUtils::tolower(message)];

		bz_PlayerRecord	*playerInfo = bz_getPlayerByIndex(playerID);
		if (!playerInfo)
		{
			bz_debugMessage(1,"vocaliser plugin: bz_getPlayerByIndex failed");
			return true;
		}
		int target = BZ_ALLUSERS;
		if (item.team)
			target = playerInfo->team;

		voice.lastVoiceTime = (int)time;

		bz_sendTextMessage(playerID,target,item.text.c_str());
		bz_sendPlayCustomLocalSound (target, item.sound.c_str());

		return true;
	}

	return false;
}

void VocaliserEvents::process ( bz_EventData *eventData )
{
	bz_PlayerJoinPartEventData	*joinPartData = (bz_PlayerJoinPartEventData*)eventData;

	switch( eventData->eventType)
	{
	case bz_ePlayerJoinEvent:
		// send em out a resource list
		for ( unsigned int i = 0; i < resourceList.size(); i++)
			bz_sentFetchResMessage(joinPartData->playerID,resourceList[i].c_str());
		break;

	case bz_ePlayerPartEvent:
		// remove them from the "list"
		clearPlayerVoiceRecord(joinPartData->playerID);
		break;
	default:
		break;
	}
}

