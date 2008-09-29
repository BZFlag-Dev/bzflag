// loops.cpp : Defines the entry point for the DLL application.
//

#include "loops.h"
#include "plugin_files.h"

PlayerLoop *playerLoop = NULL;
NavLoop *navLoop = NULL;
VarsLoop *varsLoop = NULL;
ChatLoop *chatLoop = NULL;
LogLoop *logLoop = NULL;


size_t	max_loop = 0xFFFFFFFF;

void initLoops ( Templateiser &ts )
{
  playerLoop = new PlayerLoop(ts);
  navLoop = new NavLoop(ts);
  varsLoop = new VarsLoop(ts);
  chatLoop = new ChatLoop(ts);
  logLoop = new LogLoop(ts);
}

void freeLoops ( void )
{
  delete(playerLoop);
  delete(navLoop);
  delete(varsLoop);
  delete(chatLoop);
  delete(logLoop);

  playerLoop = NULL;
  navLoop = NULL;
  varsLoop = NULL;
  chatLoop = NULL;
  logLoop =NULL;
}

//--------------LoopHandler
LoopHandler::LoopHandler()
{
  pos = max_loop;
  size = 0;
}

bool LoopHandler::atStart ( void )
{
  if ( pos == max_loop )
  {
    pos = getStart();
    return true;
  }
  return false;
}

bool LoopHandler::increment ( void )
{
  if (pos == max_loop)
    return false;
  pos = getNext(pos);
  if ( pos >= size )
  {
    pos = max_loop;
    return false;
  }
  return true;
}

bool LoopHandler::done ( void )
{
  if ( (pos == max_loop) || (pos >= size) )
  {
    terminate();
    pos = max_loop;
    return true;
  }
  return false;
}

void LoopHandler::keyCallback (std::string &data, const std::string &key)
{
  if (!done())
    getKey(pos,data,key);
}

bool LoopHandler::ifCallback (const std::string &key)
{
  if (done())
    return false;

  return getIF(pos,key);
}

bool LoopHandler::loopCallback (const std::string &key)
{
  if (atStart())
  {
    setSize(); // let the derived class set it's size
    return !done();
  }
  else
    return increment();
}

//------------------PlayerLoop
PlayerLoop::PlayerLoop(Templateiser &ts ) : LoopHandler()
{
  ts.addLoop("players",this);
  ts.addKey("playerid",this);
  ts.addKey("callsign",this);
}

void PlayerLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  bz_BasePlayerRecord *player = bz_getPlayerByIndex(idList[item]);

  if (player)
  {
    if (key == "playerid")
      data += player->bzID.c_str();
    else if (key == "callsign")
      data += player->callsign.c_str();

    bz_freePlayerRecord(player);
  }
  else
    data += "invalid_player";
}

void PlayerLoop::setSize ( void )
{
  idList.clear();

  bz_APIIntList *players = bz_getPlayerIndexList();
  if (players)
  {
    for (unsigned int s = 0; s < players->size(); s++)
      idList.push_back(players->get(s));

    bz_deleteIntList(players);
  }

  size = idList.size();
}

//------------------NavLoop
NavLoop::NavLoop(Templateiser &ts ) : LoopHandler()
{
  size = pages.size();

  ts.addLoop("navigation",this);
  ts.addKey("pagename",this);
  ts.addKey("pagetitle",this);
  ts.addKey("currentpage",this);
  ts.addKey("currentpagetitle",this);
  ts.addIF("iscurrentpage",this);
}

void NavLoop::computePageList ( void )
{
  // scan the dirs for files with title 

  bool haveMain = false;
  for ( size_t f = 0; f < pages.size(); f++ )
  {
    std::string page = getFileTitle(pages[f]);
    if (compare_nocase(page,"main")==0)
	haveMain = true;
  }

  // ok sort that sucker so main is first

  if (haveMain)
  {
    std::vector<std::string> files = pages;
    pages.clear();
    pages.push_back("Main");
    for ( size_t f = 0; f < files.size(); f++ )
    {
      if (compare_nocase(files[f],"main")!= 0)
	pages.push_back(files[f]);
    }
  }
}

void NavLoop::setSize ( void )
{
  size = pages.size();
}


// CurrentPage dosn't use a loop, so just service it as normal
void NavLoop::keyCallback (std::string &data, const std::string &key)
{
  if (key=="curerntpage")
    data += currentTemplate;
  else if (key=="currentpagetitle")
    data += replace_all(getFileTitle(currentTemplate),std::string("_"),std::string(" "));
  else
    LoopHandler::keyCallback(data,key);
}

void NavLoop::getKey (size_t item, std::string &data, const std::string &key )
{
  if (key == "pagename")
    data += pages[item];
  if (key == "pagetitle")
    data +=   replace_all(pages[item],std::string("_"),std::string(" "));
}

bool NavLoop::getIF  (size_t item, const std::string &key)
{
  if (key == "iscurrentpage")
    return currentTemplate == pages[item];
  return false;
}

//--------------VarsLoop
VarsLoop::VarsLoop(Templateiser &ts)
{
  ts.addLoop("servervars",this);
  ts.addKey("servervarname",this);
  ts.addKey("servervarvalue",this);
}

void VarsLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  if (key == "servervarname")
    data += keys[item];
  else if (key == "servervarvalue")
    data += values[item];
}

void VarsLoop::setSize ( void )
{
  keys.clear();
  values.clear();

  bz_APIStringList *vars = bz_newStringList();

  int count = bz_getBZDBVarList(vars);

  if (vars)
  {
    for ( int i = 0; i < count; i++ )
    {
      keys.push_back(std::string(vars->get(i).c_str()));
      values.push_back(std::string(bz_getBZDBString(vars->get(i).c_str()).c_str()));
    }
  }

  size = keys.size();
}

//-------------PermsLoop

PermsLoop::PermsLoop(Templateiser &ts)
{
  bz_APIStringList *permList = bz_getStandardPermList();
  for ( unsigned int i = 0; i < permList->size(); i++ )
    perms.push_back(std::string(permList->get(i).c_str()));

  bz_deleteStringList(permList);
  size = perms.size();

  ts.addLoop("Permissions",this);
  ts.addKey("Permission",this);
}

void PermsLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  data += perms[item];
}


//-----------ChatLoop
ChatLoop::ChatLoop(Templateiser &ts) : LoopHandler()
{
  chatLimit = 20;
  addNewPageCallback(this);
  bz_registerEvent(bz_eRawChatMessageEvent,this);
  ts.addLoop("chatlines",this);
  ts.addKey("chatlinetime",this);
  ts.addKey("chatlineuser",this);
  ts.addKey("chatlineteam",this);
  ts.addKey("chatlineto",this);
  ts.addKey("chatlinetext",this);

  ts.addKey("chattotal",this);
  ts.addKey("chatitemnumber",this);

  ts.addIF("chatlineisforteam",this);
  ts.addIF("chatlimit",this);

  // debug
#ifdef _DEBUG
  ChatMessage message;

  bz_Time now;

  bz_getUTCtime(&now);
  message.time = printTime(&now);

  message.from = "DEBUG";
  message.to = "All";
  message.fromTeam = "";
  message.teamType = eNoTeam;
  message.message = "Chat Log Startup";

  messages.push_back(message);
#endif
}

ChatLoop::~ChatLoop()
{
  removeNewPageCallback(this);
  bz_removeEvent(bz_eRawChatMessageEvent,this);
}

// check for any filter params
void ChatLoop::newPage ( const std::string &pagename, const HTTPRequest &request )
{
  formChatLimit = 20;

  std::string val;
  if (request.getParam("chatlimit",val))
    formChatLimit = (size_t)atoi(val.c_str());
}

bool ChatLoop::loopCallback (const std::string &key)
{
  if (templateParam.size())
    chatLimit = atoi(templateParam.c_str());
  else
    chatLimit = formChatLimit;
  return LoopHandler::loopCallback(key);
}

// CurrentPage dosn't use a loop, so just service it as normal
void ChatLoop::keyCallback (std::string &data, const std::string &key)
{
  if (key=="chattotal")
    data += format("%d",messages.size());
  else
    LoopHandler::keyCallback(data,key);
}

bool ChatLoop::ifCallback (const std::string &key)
{
  if (key == "chatlimit" && templateParam.size())
    return chatLimit == atoi(templateParam.c_str());

  return LoopHandler::ifCallback(key);
}


void ChatLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  ChatMessage &message = messages[item];

  if (key == "chatlinetime")
    data += message.time;
  else if (key == "chatlineuser")
    data += message.from;
  else if (key == "chatlineteam")
    data += message.fromTeam;
  else if (key == "chatlineto")
    data += message.to;
  else if (key == "chatlinetext")
    data += message.message;
  else if (key == "chatlinetext")
    data += format("%d",pos);
}

bool ChatLoop::getIF  (size_t item, const std::string &key)
{
  ChatMessage &message = messages[item];
  if (key == "chatlineisforteam")
    return message.teamType != eNoTeam;

  return false;
}

 void ChatLoop::process(bz_EventData *eventData)
 {
   bz_ChatEventData_V1* data = (bz_ChatEventData_V1*)eventData;
   if (data)
   {
     ChatMessage message;

     bz_Time now;

     bz_getUTCtime(&now);
     message.time = printTime(&now);

     message.message = data->message.c_str();
     message.teamType = eNoTeam;

     if (data->from != BZ_SERVER)
     {
       message.from = bz_getPlayerCallsign(data->from);
       message.teamType = bz_getPlayerTeam(data->from);
       message.fromTeam = bzu_GetTeamName(message.teamType);
     }
     else
       message.from = "server";

     if (data->to == BZ_NULLUSER)
       message.to = bzu_GetTeamName(data->team);
     else if ( data->to == BZ_ALLUSERS)
       message.to = "all";
     else
       message.to = bz_getPlayerCallsign(data->to);

     messages.push_back(message);
   }
 }

 void ChatLoop::setSize ( void )
 {
   size = messages.size();
 }

 size_t ChatLoop::getStart ( void )
 {
   if (chatLimit <= 0)
     return 0;
   if ( messages.size() < chatLimit)
     return 0;
   return messages.size() - chatLimit; // always start the limit up from the bottom
 }

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8