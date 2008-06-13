// SAMPLE_PLUGIN.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

BZ_GET_PLUGIN_VERSION

class ChatLogger : public bz_EventHandler
{
  virtual void process ( bz_EventData *eventData );
};

std::string filePath;
ChatLogger chatLogger;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
  if (commandLine && strlen(commandLine))
    filePath = commandLine;
  else
    filePath = "chatlog.txt";

  bz_registerEvent(bz_eRawChatMessageEvent,&chatLogger);

  bz_debugMessage(4,"ChatLog plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_eRawChatMessageEvent,&chatLogger);

  bz_debugMessage(4,"ChatLog plugin unloaded");
  return 0;
}


void ChatLogger::process( bz_EventData *eventData )
{
  if (eventData->eventType != bz_eRawChatMessageEvent)
    return;

  FILE *fp = fopen(filePath.c_str(),"a+");
  if (!fp)
    return;

  bz_ChatEventData_V1* data = (bz_ChatEventData_V1*)eventData;

  bz_localTime now;
  bz_getLocaltime(&now);

  const char* from = bz_getPlayerCallsign(data->from);
  const char* to = bz_getPlayerCallsign(data->to);

  const char* team = bzu_GetTeamName(data->team);

  fprintf(fp,"%d:%d:%d %d.%d.%d",now.year,now.month,now.day,now.hour,now.minute,now.second);
  fprintf(fp,"%s to %s :%s\n",from ? from : "Unknown", to ? to : (team ? team : "General"), data->message.c_str());
  fclose(fp);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
