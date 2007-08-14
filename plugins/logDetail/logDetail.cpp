/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// logDetail.cpp : Plugin module for logging server events to stdout
//

#include <iostream>
#include <sstream>
#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

using namespace std;

enum action { join , auth , part };

class LogDetail : public bz_EventHandler
{
public:
  LogDetail();
  virtual ~LogDetail();
  virtual void process( bz_EventData *eventData );
private:
  void displayPlayerPrivs( int playerID );
  void displayCallsign( bz_ApiString callsign );
  void displayCallsign( int playerID );
  void displayBZid( int playerID );
  void displayTeam( bz_eTeamType team );
  virtual void listPlayers( action act, bz_PlayerJoinPartEventData_V1 *data );
};

LogDetail logDetailHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_registerEvent(bz_eSlashCommandEvent, &logDetailHandler);
  bz_registerEvent(bz_eRawChatMessageEvent, &logDetailHandler);
  bz_registerEvent(bz_eServerMsgEvent, &logDetailHandler);
  bz_registerEvent(bz_ePlayerJoinEvent, &logDetailHandler);
  bz_registerEvent(bz_ePlayerPartEvent, &logDetailHandler);
  bz_registerEvent(bz_ePlayerAuthEvent, &logDetailHandler);
  bz_registerEvent(bz_eMessageFilteredEvent, &logDetailHandler);
  bz_debugMessage(4, "logDetail plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_eSlashCommandEvent, &logDetailHandler);
  bz_removeEvent(bz_eRawChatMessageEvent, &logDetailHandler);
  bz_removeEvent(bz_eServerMsgEvent, &logDetailHandler);
  bz_removeEvent(bz_ePlayerJoinEvent, &logDetailHandler);
  bz_removeEvent(bz_ePlayerPartEvent, &logDetailHandler);
  bz_removeEvent(bz_ePlayerAuthEvent, &logDetailHandler);
  bz_removeEvent(bz_eMessageFilteredEvent, &logDetailHandler);
  bz_debugMessage(4, "logDetail plugin unloaded");
  return 0;
}

LogDetail::LogDetail()
{
  cout << "SERVER-STATUS Running" << endl;
  listPlayers( join , NULL );
}

LogDetail::~LogDetail()
{
  listPlayers( part , NULL );
  cout << "SERVER-STATUS Stopped" << endl;
}

void LogDetail::process( bz_EventData *eventData )
{
  bz_ChatEventData_V1 *chatData = (bz_ChatEventData_V1 *) eventData;
  bz_ServerMsgEventData_V1 *serverMsgData = (bz_ServerMsgEventData_V1 *) eventData;
  bz_SlashCommandEventData_V1 *cmdData = (bz_SlashCommandEventData_V1 *) eventData;
  bz_PlayerJoinPartEventData_V1 *joinPartData = (bz_PlayerJoinPartEventData_V1 *) eventData;
  bz_PlayerAuthEventData_V1 *authData = (bz_PlayerAuthEventData_V1 *) eventData;
  bz_MessageFilteredEventData_V1 *filteredData = (bz_MessageFilteredEventData_V1 *) eventData;
  char temp[9] = {0};

  if (eventData) {
    switch (eventData->eventType) {
      case bz_eSlashCommandEvent:
	// Slash commands are case insensitive
	// Tokenize the stream and check the first word
	// /report -> MSG-REPORT
	// anything -> MSG-COMMAND

	strncpy(temp, cmdData->message.c_str(), 8);

	if (strcasecmp(temp, "/REPORT ") == 0) {
	  cout << "MSG-REPORT ";
	  displayCallsign( cmdData->from );
	  cout << " " << cmdData->message.c_str()+8 << endl;
	} else {
	  cout << "MSG-COMMAND ";
	  displayCallsign( cmdData->from );
	  cout << " " << cmdData->message.c_str()+1 << endl;
	}
	break;
      case bz_eRawChatMessageEvent:
	if ((chatData->to == BZ_ALLUSERS) and (chatData->team == eNoTeam)) {
	  cout << "MSG-BROADCAST ";
	  displayCallsign( chatData->from );
	  cout << " " << chatData->message.c_str() << endl;
	} else if (chatData->to == BZ_NULLUSER) {
	  if (chatData->team == eAdministrators) {
	    cout << "MSG-ADMIN ";
	    displayCallsign( chatData->from );
	    cout << " " << chatData->message.c_str() << endl;
	  } else {
	    cout << "MSG-TEAM ";
	    displayCallsign( chatData->from );
	    displayTeam( chatData->team );
	    cout << " " << chatData->message.c_str() << endl;
	  }
	} else {
	  cout << "MSG-DIRECT ";
	  displayCallsign( chatData->from );
	  cout << " ";
	  displayCallsign( chatData->to );
	  cout << " " << chatData->message.c_str() << endl;
	}
	break;
      case bz_eMessageFilteredEvent:
	cout << "MSG-FILTERED ";
	displayCallsign( filteredData->player );
	cout << " " << filteredData->filteredMessage.c_str() << endl;
	break;
      case bz_eServerMsgEvent:
	if ((serverMsgData->to == BZ_ALLUSERS) and (serverMsgData->team == eNoTeam)) {
	  cout << "MSG-BROADCAST 6:SERVER";
	  cout << " " << serverMsgData->message.c_str() << endl;
	} else if (serverMsgData->to == BZ_NULLUSER) {
	  if (serverMsgData->team == eAdministrators) {
	    cout << "MSG-ADMIN 6:SERVER";
	    cout << " " << serverMsgData->message.c_str() << endl;
	  } else {
	    cout << "MSG-TEAM 6:SERVER";
	    displayTeam( serverMsgData->team );
	    cout << " " << chatData->message.c_str() << endl;
	  }
	} else {
	  cout << "MSG-DIRECT 6:SERVER";
	  cout << " ";
	  displayCallsign( serverMsgData->to );
	  cout << " " << serverMsgData->message.c_str() << endl;
	}
	break;
      case bz_ePlayerJoinEvent:
	{
	  if (joinPartData->record) {
	    cout << "PLAYER-JOIN ";
	    displayCallsign( joinPartData->record->callsign );
	    cout << " #" << joinPartData->playerID;
	    displayBZid( joinPartData->playerID );
	    displayTeam( joinPartData->record->team );
	    displayPlayerPrivs( joinPartData->playerID );
	    cout << endl;
	    listPlayers( join, joinPartData );
	  }
	}
	break;
      case bz_ePlayerPartEvent:
	cout << "PLAYER-PART ";
	displayCallsign( joinPartData->playerID );
	cout << " #" << joinPartData->playerID;
	displayBZid( joinPartData->playerID );
	cout << " " << joinPartData->reason.c_str();
	cout << endl;
	listPlayers( part, joinPartData );
	break;
      case bz_ePlayerAuthEvent:
	cout << "PLAYER-AUTH ";
	displayCallsign( authData->playerID );
	displayPlayerPrivs( authData->playerID );
	cout << endl;
	listPlayers( join, joinPartData );
	break;
      default :
	break;
    }
  }
}

void LogDetail::displayBZid( int playerID )
{
  bz_BasePlayerRecord *player = bz_getPlayerByIndex( playerID );
  if (player && player->globalUser)
    cout << " BZid:" << player->bzID.c_str();
}

void LogDetail::displayPlayerPrivs( int playerID )
{
  bz_BasePlayerRecord *player = bz_getPlayerByIndex( playerID );
  if (player) {
    cout << " IP:" << player->ipAddress.c_str();
    if (player->verified ) cout << " VERIFIED";
    if (player->globalUser ) cout << " GLOBALUSER";
    if (player->admin ) cout << " ADMIN";
    if (player->op ) cout << " OPERATOR";
  } else {
    cout << " IP:0.0.0.0";
  }
}

void LogDetail::displayCallsign( bz_ApiString callsign )
{
  cout << strlen( callsign.c_str() ) << ":";
  cout << callsign.c_str();
}

void LogDetail::displayCallsign( int playerID )
{
  bz_BasePlayerRecord *player = bz_getPlayerByIndex( playerID );
  if (player) {
    cout << strlen( player->callsign.c_str() ) << ":";
    cout << player->callsign.c_str();
  } else {
    cout << "7:UNKNOWN";
  }
}


void LogDetail::displayTeam( bz_eTeamType team )
{
  // Display the player team
  switch ( team ) {
    case eRogueTeam:
      cout << " ROGUE";
      break;
    case eRedTeam:
      cout << " RED";
      break;
    case eGreenTeam:
      cout << " GREEN";
      break;
    case eBlueTeam:
      cout << " BLUE";
      break;
    case ePurpleTeam:
      cout << " PURPLE";
      break;
    case eRabbitTeam:
      cout << " RABBIT";
      break;
    case eHunterTeam:
      cout << " HUNTER";
      break;
    case eObservers:
      cout << " OBSERVER";
      break;
    default :
      cout << " NOTEAM";
      break;
  }
}

void LogDetail::listPlayers( action act , bz_PlayerJoinPartEventData_V1 *data )
{
  bz_APIIntList *playerList = bz_newIntList();
  bz_BasePlayerRecord *player;
  ostringstream msg;
  string str;
  char playerStatus;
  int numPlayers;

  bz_getPlayerIndexList( playerList );

  bz_debugMessage( 4 , "Players:" );
  //
  // Count number of players
  //
  numPlayers = 0;
  for ( unsigned int i = 0; i < playerList->size(); i++ ) {
    player = bz_getPlayerByIndex( playerList->get(i));
    if (player) {
      if ((player->callsign != "") && (act == join || act == auth || (data && (player->playerID != data->playerID))))
	numPlayers++;
      bz_freePlayerRecord( player );
    }
  }

  //
  // Display number of players, callsign, and email string in the following format:
  //
  // PLAYERS (nn) [G]cc:callsign(ee:emailstring)
  // nn - number of players
  // G  - global auth identifier (+|-| |@)
  // cc - count of characters in player callsign
  // callsign - player callsign
  // ee - count of characters in email string
  // emailstring - player email string
  //
  // eg.
  // PLAYERS (2) [@]7:Thumper(16:me@somewhere.net) [ ]3:xxx()
  //
  msg.str("");
  msg << "PLAYERS (" << numPlayers << ") ";
  for ( unsigned int i = 0; i < playerList->size(); i++ ) {
    player = bz_getPlayerByIndex( playerList->get(i));
    if (player) {
      if ((player->callsign != "") && (act == join || act == auth || (data && (player->playerID != data->playerID)))) {
	playerStatus = ' ';
	if (player->globalUser) playerStatus = '+';
	if (player->verified) playerStatus = '+';
	if (player->admin and !bz_hasPerm(player->playerID, bz_perm_hideAdmin)) playerStatus = '@';
	msg << "[" << playerStatus << "]";
	msg << player->callsign.size() << ':';
	msg << player->callsign.c_str();
	msg << "(";
	if (player->email != "")
	  msg << player->email.size() << ":" << player->email.c_str();
	msg << ") ";
      }
      bz_freePlayerRecord( player );
    }
  }
  str = msg.str();
  cout << str << endl;

  bz_deleteIntList(playerList);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
