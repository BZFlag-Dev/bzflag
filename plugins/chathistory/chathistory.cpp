// chathistory.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "TextUtils.h"
#include <string>
#include <map>
#include <vector>

BZ_GET_PLUGIN_VERSION

class LastChatCommand : public bz_CustomSlashCommandHandler
{
public:
	virtual ~LastChatCommand(){};
	virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *param );
};

LastChatCommand	lastChatCommand;

// event handler callback
class ChatEvents : public bz_EventHandler
{
public:
	virtual ~ChatEvents(){};
	virtual void process ( bz_EventData *eventData );
};

ChatEvents chatEvents;

typedef std::vector<std::string>	tvChatHistory;

std::map<std::string,tvChatHistory>	chatHistories;

unsigned int		maxChatLines;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"ChatEvents plugin loaded");

	maxChatLines = 1000;
	if (commandLine)
	{
		int realLines = atoi(commandLine);
		maxChatLines  = realLines;
	}

	bz_registerCustomSlashCommand("last",&lastChatCommand);
	bz_registerCustomSlashCommand("flushchat",&lastChatCommand);

	bz_registerEvent(bz_eChatMessageEvent,&chatEvents);

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeCustomSlashCommand("last");
	bz_removeCustomSlashCommand("flushchat");

	bz_removeEvent(bz_eChatMessageEvent,&chatEvents);

	bz_debugMessage(4,"ChatEvents plugin unloaded");
	return 0;
}


bool LastChatCommand::handle ( int playerID, bzApiString _command, bzApiString _message, bzAPIStringList */*_param*/ )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();


	bz_PlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

	if ( !fromPlayer->admin )
	{
		bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the ChatHistory plugin");
		return true;
	}

	if ( command == "list")
	{
		std::vector<std::string> params = TextUtils::tokenize(message,std::string(" "),1,false);
		if ( params.size() <2)
		{
			bz_sendTextMessage(BZ_SERVER,playerID,"Usage: /last <NUMBER OF LINES> <CALLSIGN>");
			return true;
		}

		unsigned int numLines = (unsigned int)atoi(params[0].c_str());
		if ( numLines == 0 )
			numLines = 5;

		std::map<std::string,tvChatHistory>::iterator itr = chatHistories.find(TextUtils::tolower(params[1]));

		if ( itr == chatHistories.end() || !itr->second.size())
		{
			bz_sendTextMessage(BZ_SERVER,playerID,"That player has no chat history.");
			return true;
		}

		tvChatHistory &history = itr->second;
		
		if ( history.size() < numLines ) 
			numLines = (unsigned int )history.size();

		bz_sendTextMessage(BZ_SERVER,playerID,TextUtils::format("Last %d message for %s",numLines,params[1].c_str()).c_str());

		for ( unsigned int i = 0; i < numLines-1; i++ )
		{
			std::string chatItem = history[history.size()-i];
			bz_sendTextMessage(BZ_SERVER,playerID,TextUtils::format("%d<%s> %s",i,params[1].c_str(),chatItem.c_str()).c_str());
		}

		return true;
	}

	if ( command == "flushchat")
	{
		chatHistories.clear();
		bz_sendTextMessage(BZ_SERVER,playerID,"Chat History has been flushed");
		return true;
	}

	return false;
}

void ChatEvents::process ( bz_EventData *eventData )
{
	bz_ChatEventData	*chatEventData = (bz_ChatEventData*)eventData;

	bz_PlayerRecord *fromPlayer = bz_getPlayerByIndex(chatEventData->from);

	std::string message = chatEventData->message.c_str();

	std::string callsign = fromPlayer->callsign.c_str();
	callsign = TextUtils::tolower(callsign);

	switch( eventData->eventType)
	{
	default:
		break;

	case bz_eChatMessageEvent:
		std::map<std::string,tvChatHistory>::iterator itr = chatHistories.find(callsign);
		if (itr == chatHistories.end())
		{
			tvChatHistory h;
			chatHistories[callsign] = h;
		}

		tvChatHistory &history = chatHistories[callsign];

		history.push_back(message);
		if (history.size() > maxChatLines) 
			history.erase(history.begin());
		break;

	}

	bz_freePlayerRecord(fromPlayer);

}

