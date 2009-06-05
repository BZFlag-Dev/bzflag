/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// loops.cpp : Defines the entry point for the DLL application.
//

#include "loops.h"
#include "plugin_files.h"

//-----------LogLoop
LogLoop::LogLoop(Templateiser &ts) : LoopHandler()
{
  displayLimit = 40;
  addNewPageCallback(this);
  ts.addLoop("loglines",this);
  ts.addKey("loglinetime",this);
  ts.addKey("loglinetext",this);

  bz_registerEvent(bz_eRawChatMessageEvent,this);
  bz_registerEvent(bz_ePlayerJoinEvent,this);
  bz_registerEvent(bz_ePlayerPartEvent,this);
  bz_registerEvent(bz_ePlayerDieEvent,this);
  bz_registerEvent(bz_ePlayerSpawnEvent,this);
  bz_registerEvent(bz_eGetWorldEvent,this);
  bz_registerEvent(bz_eWorldFinalized,this);

  bz_registerEvent(bz_eNewRabbitEvent,this);
  bz_registerEvent(bz_eCaptureEvent,this);
  bz_registerEvent(bz_eCaptureEvent,this);

  bz_registerEvent(bz_eListServerUpdateEvent,this);
  bz_registerEvent(bz_eBanEvent,this);
  bz_registerEvent(bz_eHostBanNotifyEvent,this);
  bz_registerEvent(bz_eHostBanModifyEvent,this);
  bz_registerEvent(bz_eIdBanEvent,this);
  bz_registerEvent(bz_eKickEvent,this);
  bz_registerEvent(bz_eKillEvent,this);
  bz_registerEvent(bz_ePlayerPausedEvent,this);
  bz_registerEvent(bz_eMessageFilteredEvent,this);

  bz_registerEvent(bz_eGameStartEvent,this);
  bz_registerEvent(bz_eGameEndEvent,this);
  bz_registerEvent(bz_eSlashCommandEvent,this);
  bz_registerEvent(bz_ePlayerAuthEvent,this);
  bz_registerEvent(bz_eReportFiledEvent,this);


  // debug
#ifdef _DEBUG
  LogMessage message;

  bz_Time now;

  bz_getUTCtime(&now);
  message.time = printTime(&now);

  message.message = "Log Startup";

  messages.push_back(message);
#endif
}

LogLoop::~LogLoop()
{
  removeNewPageCallback(this);
  bz_removeEvent(bz_eRawChatMessageEvent,this);
  bz_removeEvent(bz_ePlayerJoinEvent,this);
  bz_removeEvent(bz_ePlayerPartEvent,this);
  bz_removeEvent(bz_ePlayerDieEvent,this);
  bz_removeEvent(bz_ePlayerSpawnEvent,this);
  bz_removeEvent(bz_eGetWorldEvent,this);
  bz_removeEvent(bz_eWorldFinalized,this);

  bz_removeEvent(bz_eListServerUpdateEvent,this);
  bz_removeEvent(bz_eBanEvent,this);
  bz_removeEvent(bz_eHostBanNotifyEvent,this);
  bz_removeEvent(bz_eHostBanModifyEvent,this);
  bz_removeEvent(bz_eIdBanEvent,this);
  bz_removeEvent(bz_eKickEvent,this);
  bz_removeEvent(bz_eKillEvent,this);
  bz_removeEvent(bz_ePlayerPausedEvent,this);
  bz_removeEvent(bz_eMessageFilteredEvent,this);

  bz_removeEvent(bz_eGameStartEvent,this);
  bz_removeEvent(bz_eGameEndEvent,this);
  bz_removeEvent(bz_eSlashCommandEvent,this);
  bz_removeEvent(bz_ePlayerAuthEvent,this);
  bz_removeEvent(bz_eReportFiledEvent,this);
}

// check for any filter params
void LogLoop::newPage ( const std::string &pagename, const HTTPRequest &request )
{
  displayLimit = 20;
  if (!pagename.size() || compare_nocase(pagename,"main"))
    displayLimit = 5;

  std::string val;
  if (request.getParam("loglimit",val))
    displayLimit = (size_t)atoi(val.c_str());
}

void LogLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  LogMessage &message = messages[item];

  if (key == "loglinetime")
    data += message.time;
  else if (key == "loglinetext")
    data += message.message;
}

void LogLoop::getLogAsFile ( std::string &file )
{
  for (size_t i = 0; i < messages.size(); i++)
  {
    file += messages[i].time + ":";
    file += messages[i].message + "\r\n";
  }
}

void LogLoop::clearLog ( void )
{
  messages.clear();

  bz_Time now;

  LogMessage message;
  bz_getUTCtime(&now);
  message.time = printTime(&now);

  message.message = "Log cleared";
  messages.push_back(message);
}

void LogLoop::process(bz_EventData *eventData)
{
  if (!eventData)
    return;

  LogMessage message;

  bz_Time now;

  bz_getUTCtime(&now);
  message.time = printTime(&now);

  switch(eventData->eventType)
  {
  case bz_eRawChatMessageEvent:
    logChatMessage((bz_ChatEventData_V1*)eventData,message);
    break;

  case bz_ePlayerJoinEvent:
  case bz_ePlayerPartEvent:
    logJoinPartMessage((bz_PlayerJoinPartEventData_V1*)eventData,message,eventData->eventType==bz_ePlayerJoinEvent);
    break;

  case bz_ePlayerSpawnEvent:
    logSpawnMessage ( (bz_PlayerSpawnEventData_V1*)eventData, message );
    break;

  case bz_ePlayerDieEvent:
    logDieMessage ( (bz_PlayerDieEventData_V1*)eventData, message );
    break;

  case bz_eGetWorldEvent:
    logGetWorldMessage((bz_GetWorldEventData_V1*)eventData,message);
    break;

  case bz_eWorldFinalized:
    logWorldDoneMessage(message);
    break;

  case bz_eListServerUpdateEvent:
    message.message = "List server update";
    break;

  case bz_eBanEvent:
    logBanMessage ((bz_BanEventData_V1*)eventData, message);
    break;

  case bz_eHostBanNotifyEvent:
    logHostBanMessage ((bz_HostBanEventData_V1*)eventData, message);
    break;

  case bz_eIdBanEvent:
    logIDBanMessage ((bz_IdBanEventData_V1*)eventData, message);
    break;

  case bz_eKickEvent:
    logKickMessage ((bz_KickEventData_V1*)eventData, message);
    break;

  case bz_eKillEvent:
    logKillMessage ((bz_KillEventData_V1*)eventData, message);
    break;

  case bz_ePlayerPausedEvent:
    logPausedMessage ((bz_PlayerPausedEventData_V1*)eventData, message);
    break;

  case bz_eMessageFilteredEvent:
    message.message = "Message Filtered Event";
    break;

  case bz_eGameStartEvent:
  case bz_eGameEndEvent:
    logGameStartEndMessage ((bz_GameStartEndEventData_V1*)eventData, message, eventData->eventType==bz_eGameStartEvent);
    break;

  case bz_eSlashCommandEvent:
    logSlashMessage ((bz_SlashCommandEventData_V1*)eventData, message);
    break;

  case bz_ePlayerAuthEvent:
    logAuthMessage ((bz_PlayerAuthEventData_V1*)eventData, message);
    break;

  case bz_eReportFiledEvent:
    logReportMessage ( (bz_ReportFiledEventData_V1*)eventData, message);
    break;

  default:
    break;
  }

  if (message.message.size())
    messages.push_back(message);
}

void LogLoop::logReportMessage ( bz_ReportFiledEventData_V1 *data, LogMessage &message )
{
  message.message = format("Report Event: %s: report %s",data->from.c_str(),data->message.c_str());
}

void LogLoop::logAuthMessage ( bz_PlayerAuthEventData_V1 *data, LogMessage &message )
{
  std::string player;
  if (data->playerID >= 0)
    player = bz_getPlayerCallsign(data->playerID);

  std::string auth;
  if (data->password)
    auth = "entered a password";
  else if (data->globalAuth)
    auth = "globally authenticated";
  else
    auth = "failed to authenticate";

  message.message = format("Authentication Event: %s(%d) has %s",player.c_str(),data->playerID,auth.c_str());
}

void LogLoop::logSlashMessage ( bz_SlashCommandEventData_V1 *data, LogMessage &message )
{
  message.message = format("Slash Command: %s(%d): %s",bz_getPlayerCallsign(data->from),data->from,data->message.c_str());
}

void LogLoop::logGameStartEndMessage ( bz_GameStartEndEventData_V1 *data, LogMessage &message, bool start )
{
  if (start)
    message.message = "Game Start Event: ";
  else
    message.message = "Game End Event: ";

  message.message += format("%f",data->duration);
}

void LogLoop::logPausedMessage ( bz_PlayerPausedEventData_V1 *data, LogMessage &message )
{
  std::string player;
  if (data->playerID >= 0)
    player = bz_getPlayerCallsign(data->playerID);

  message.message = format("Pause Event: %s(%d) %spaused",player.c_str(),data->playerID,data->pause ? "" : "un");
}

void LogLoop::logKillMessage ( bz_KillEventData_V1 *data, LogMessage &message )
{
  std::string killer = "Server";
  if (data->killerID != BZ_SERVER)
    killer = bz_getPlayerCallsign(data->killerID);

  std::string killie;
  if (data->killedID >= 0)
    killie = bz_getPlayerCallsign(data->killedID);

  message.message = format("Kill Event: %s killed %d",killer.c_str(),data->killedID);
  if (killie.size())
    message.message += "(" + killie + ") ";

  if (data->reason.size())
    message.message += data->reason.c_str();
}

void LogLoop::logBanMessage ( bz_BanEventData_V1 *data, LogMessage &message )
{
  std::string baner = "Server";
  if (data->bannerID != BZ_SERVER)
    baner = bz_getPlayerCallsign(data->bannerID);

  std::string bannie;
  if (data->banneeID >= 0)
    bannie = bz_getPlayerCallsign(data->banneeID);

  message.message = format("Ban Event: %s banned %s",baner.c_str(),data->ipAddress.c_str());
  if (bannie.size())
    message.message += "(" + bannie + ")";

  message.message += " for " + format("%d ",data->duration);
  if (data->reason.size())
    message.message += data->reason.c_str();
}

void LogLoop::logHostBanMessage ( bz_HostBanEventData_V1 *data, LogMessage &message )
{
  std::string baner = "Server";
  if (data->bannerID != BZ_SERVER)
    baner = bz_getPlayerCallsign(data->bannerID);

  message.message = format("Host Ban Event: %s banned %s",baner.c_str(),data->hostPattern.c_str());

  message.message += " for " + format("%d ",data->duration);
  if (data->reason.size())
    message.message += data->reason.c_str();
}

void LogLoop::logIDBanMessage ( bz_IdBanEventData_V1 *data, LogMessage &message )
{
  std::string baner = "Server";
  if (data->bannerID != BZ_SERVER)
    baner = bz_getPlayerCallsign(data->bannerID);

  std::string bannie;
  if (data->banneeID >= 0)
    bannie = bz_getPlayerCallsign(data->banneeID);

  message.message = format("ID Ban Event: %s banned %s",baner.c_str(),data->bzId.c_str());
  if (bannie.size())
    message.message += "(" + bannie + ")";

  message.message += " for " + format("%d ",data->duration);
  if (data->reason.size())
    message.message += data->reason.c_str();
}

void LogLoop::logKickMessage ( bz_KickEventData_V1 *data, LogMessage &message )
{
  std::string kicker = "Server";
  if (data->kickerID != BZ_SERVER)
    kicker = bz_getPlayerCallsign(data->kickerID);

  std::string kickie;
  if (data->kickedID >= 0)
    kickie = bz_getPlayerCallsign(data->kickedID);

  message.message = format("Host Ban Event: %s banned %d",kicker.c_str(),data->kickerID);
  if (kickie.size())
    message.message += "(" + kickie + ") ";

  if (data->reason.size())
    message.message += data->reason.c_str();
}

void LogLoop::logChatMessage ( bz_ChatEventData_V1 *data, LogMessage &message )
{
  std::string from;
  std::string to;

  if (data->from != BZ_SERVER)
    from = bz_getPlayerCallsign(data->from);
  else
    from = "server";

  if (data->to == BZ_NULLUSER)
    to = bzu_GetTeamName(data->team);
  else if ( data->to == BZ_ALLUSERS)
    to = "all";
  else
    to = bz_getPlayerCallsign(data->to);

  message.message = format("Chat from %s to %s : %s",from.c_str(),to.c_str(),data->message.c_str());
}

void LogLoop::logJoinPartMessage ( bz_PlayerJoinPartEventData_V1 *data, LogMessage &message, bool join )
{
  message.message = format("Player %s(%d)",data->record->callsign.c_str(),data->playerID);
  if (join)
  {
    message.message += format("[%s] ",data->record->ipAddress.c_str());
    message.message += " joined";
  }
  else
    message.message += " parted";
  if (data->reason.size())
  {
    message.message += " reason: ";
    message.message += data->reason.c_str();
  }
}

void LogLoop::logSpawnMessage ( bz_PlayerSpawnEventData_V1 *data, LogMessage &message )
{
  message.message = format("Player %s(%d) spawned at %f %f %f (%f)",bz_getPlayerCallsign(data->playerID),data->playerID,data->state.pos[0],data->state.pos[1],data->state.pos[2],data->state.rotation);
}

void LogLoop::logDieMessage ( bz_PlayerDieEventData_V1 *data, LogMessage &message )
{
  message.message = format("Player %s(%d) died at %f %f %f (%f)",bz_getPlayerCallsign(data->playerID),data->playerID,data->state.pos[0],data->state.pos[1],data->state.pos[2],data->state.rotation);

  if (data->killerID != -1)
    message.message += format(" by %s(%d) with %s",bz_getPlayerCallsign(data->killerID),data->killerID,data->flagKilledWith.c_str());
}

void LogLoop::logGetWorldMessage ( bz_GetWorldEventData_V1 *data, LogMessage &message )
{
  std::string world = "random";
  if (data->worldFile.size())
    world = data->worldFile.c_str();
  if (data->worldBlob)
    world = "Custom";

  message.message = format("World %s loading",world.c_str());
}

void LogLoop::logWorldDoneMessage ( LogMessage &message )
{
  message.message = "World loaded";
}

void LogLoop::setSize ( void )
{
  size = messages.size();
}

size_t LogLoop::getStart ( void )
{
  if (displayLimit <= 0)
    return 0;
  if ( messages.size() < displayLimit)
    return 0;
  return messages.size() - displayLimit; // always start the limit up from the bottom
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
