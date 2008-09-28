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
       logBanMessage ( (bz_BanEventData_V1*)eventData, message);
	break;

     case bz_eHostBanNotifyEvent:
     case bz_eHostBanModifyEvent:
      logHostBanMessage ( (bz_HostBanEventData_V1*)eventData, message );
      break;

     case bz_eIdBanEvent:
       logIDBanMessage ( (bz_IdBanEventData_V1*)eventData, message );
       break;
    case bz_eKickEvent:
     case bz_eKillEvent:
     case bz_ePlayerPausedEvent:
     case bz_eMessageFilteredEvent:

     case bz_eGameStartEvent:
     case bz_eGameEndEvent:
     case bz_eSlashCommandEvent:
     case bz_ePlayerAuthEvent:
     case bz_eReportFiledEvent:
       break;

     default:
       break;
   }

   if (message.message.size())
     messages.push_back(message);
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
    message.message = format("Player %s(%d) ",data->record->callsign.c_str(),data->playerID);
    if (join)
      message.message += "joined";
    else
      message.message += "parted";
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
