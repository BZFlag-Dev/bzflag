// loops.cpp : Defines the entry point for the DLL application.
//

#include "loops.h"
#include "plugin_files.h"

PlayerLoop *playerLoop = NULL;
NavLoop *navLoop = NULL;
VarsLoop *varsLoop = NULL;
ChatLoop *chatLoop = NULL;
LogLoop *logLoop = NULL;
IPBanLoop *ipBanLoop = NULL;
HostBanLoop *hostBanLoop = NULL;
IDBanLoop *idBanLoop = NULL;
PlayerGroupLoop *playerGroupLoop = NULL;
FlagHistoryLoop *flagHistoryLoop = NULL;
ReportsLoop *reportsLoop = NULL;

size_t	max_loop = 0xFFFFFFFF;

void initLoops ( Templateiser &ts )
{
  playerLoop = new PlayerLoop(ts);
  navLoop = new NavLoop(ts);
  varsLoop = new VarsLoop(ts);
  chatLoop = new ChatLoop(ts);
  logLoop = new LogLoop(ts);

  ipBanLoop = new IPBanLoop(ts);
  hostBanLoop = new HostBanLoop(ts);
  idBanLoop = new IDBanLoop(ts);

  playerGroupLoop = new PlayerGroupLoop(ts);
  flagHistoryLoop = new FlagHistoryLoop(ts);
  reportsLoop = new ReportsLoop(ts);
}

void freeLoops ( void )
{
  delete(playerLoop);
  delete(navLoop);
  delete(varsLoop);
  delete(chatLoop);
  delete(logLoop);
  delete(ipBanLoop);
  delete(hostBanLoop);
  delete(idBanLoop);
  delete(playerGroupLoop);
  delete(flagHistoryLoop);
  delete(reportsLoop);

  playerLoop = NULL;
  navLoop = NULL;
  varsLoop = NULL;
  chatLoop = NULL;
  logLoop =NULL;
  ipBanLoop =NULL;
  hostBanLoop =NULL;
  idBanLoop =NULL;
  playerGroupLoop =NULL;
  flagHistoryLoop =NULL;
  reportsLoop =NULL;
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

 bool LoopHandler::inLoop ( void )
 {
   return size > 0 && pos != max_loop;
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
  ts.addKey("playerbzid",this);
  ts.addKey("playercallsign",this);
  ts.addKey("playerguid",this);
  ts.addKey("playerip",this);
  ts.addKey("playerteam",this);
  ts.addKey("playerversion",this);

  ts.addKey("playerwins",this);
  ts.addKey("playerlosses",this);
  ts.addKey("playerscore",this);
  ts.addKey("playerteamkills",this);
  ts.addKey("playerrank",this);

  ts.addIF("playerbzid",this);
  ts.addIF("playerverified",this);
  ts.addIF("playeradmin",this);

  ts.addIF("playercanspawn",this);
  ts.addIF("playerspawned",this);

  ts.addKey("playerlag",this);
  ts.addKey("playerjitter",this);
  ts.addKey("playerpacketloss",this);

  ts.addKey("playerflag",this);
  ts.addKey("playerflagid",this);

  ts.addIF("playerflag",this);

  addNewPageCallback(this);

  playerID = -1;
}

PlayerLoop::~PlayerLoop()
{
  removeNewPageCallback(this);
}

void PlayerLoop::newPage ( const std::string &pagename, const HTTPRequest &request )
{
  playerID = -1;
  std::string pID;
  if (request.getParam("playerID",pID) && pID.size())
    playerID = atoi(pID.c_str());
}

int PlayerLoop::getPlayerID ( void )
{
  int id = playerID;
  if (inLoop())
    id = idList[pos];

  return id;
}

void PlayerLoop::keyCallback (std::string &data, const std::string &key)
{
  if (inLoop())
  {
    if (!done())
      getKey(pos,data,key);
  }
  else
    getKey(0,data,key);
}

bool PlayerLoop::ifCallback (const std::string &key)
{
  if (inLoop())
  {
    if (done())
      return false;

    return getIF(pos,key);
  }
  else
    return getIF(0,key);
}

void PlayerLoop::getKey (size_t /*item*/, std::string &data, const std::string &key)
{
  bz_BasePlayerRecord *player = bz_getPlayerByIndex(getPlayerID());

  if (player)
  {
    if (key == "playerbzid")
      data += player->bzID.c_str();
    else if (key == "playerid")
      data += format("%d",player->playerID);
    else if (key == "playercallsign")
      data += player->callsign.c_str();
    else if (key == "playerguid")
      data += format("%d_%s",player->playerID,player->callsign.c_str());
    else if (key == "playerip")
      data += player->ipAddress.c_str();
    else if (key == "playerteam")
      data += bzu_GetTeamName(player->team);
    else if (key == "playerversion")
      data += player->clientVersion.c_str();
    else if (key == "playerlag")
      data += format("%d",player->lag);
    else if (key == "playerjitter")
      data += format("%d",player->jitter);
    else if (key == "playerpacketloss")
      data += format("%f",player->packetLoss);
    else if (key == "playerscore")
      data += format("%d",player->wins-player->losses-player->teamKills);
    else if (key == "playerwins")
      data += format("%d",player->wins);
    else if (key == "playerlosses")
      data += format("%d",player->losses);
    else if (key == "playerteamkills")
      data += format("%d",player->teamKills);
    else if (key == "playerrank")
      data += format("%f",player->rank);
    else if (key == "playerflag")
      data += player->currentFlag.c_str();
    else if (key == "playerflagid")
      data += format("%d",player->currentFlagID);

    bz_freePlayerRecord(player);
  }
  else
    data += "invalid_player";
}

bool PlayerLoop::getIF  (size_t /*item*/, const std::string &key)
{
  bool ret = false;
 
  bz_BasePlayerRecord *player = bz_getPlayerByIndex(getPlayerID());

  if (player)
  {
    if (key == "playerbzid")
      ret = player->bzID.size() > 0;
    else if (key == "playerverified")
      ret = player->verified;
    else if (key == "playeradmin")
      ret = player->admin;
    else if (key == "playercanspawn")
      ret = player->canSpawn;
    else if (key == "playerspawned")
      ret = player->spawned;
    else if (key == "playerflag")
      ret = player->currentFlag.size() > 0;

    bz_freePlayerRecord(player);
  }

  return ret;
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

//-------------------------IPBanLoop

IPBanLoop::IPBanLoop(Templateiser &ts)
{
  ts.addLoop("IPBans",this);
  ts.addKey("IPBanMask",this);
  ts.addKey("IPBanReason",this);
  ts.addKey("IPBanSource",this);
  ts.addKey("IPBanDuration",this);
  ts.addKey("IPBanID",this);
  ts.addIF("IPBanFromMaster",this);
  ts.addIF("IPBanIsForever",this);

  filterMasterBans = false;
}

IPBanLoop::~IPBanLoop()
{
  removeNewPageCallback(this);
}

void IPBanLoop::newPage ( const std::string &pagename, const HTTPRequest &request )
{
  filterMasterBans = false;

  std::string val;
  if (request.getParam("filtermasterbans",val))
    filterMasterBans = val != "0";
}

void IPBanLoop::setSize ( void )
{
  size = bz_getBanListSize(eIPList);
}

size_t IPBanLoop::getNext ( size_t n )
{
  if (!filterMasterBans)
    return n+1;
  else
  {
    n++;
    while (bz_getBanItemIsFromMaster(eIPList,(unsigned int)n) && n < size)
      n++;
  }

  return n;
}

void IPBanLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  std::string temp;
  unsigned int i = (unsigned int )item;

  if (key == "ipbanmask")
  {
    if (bz_getBanItem(eIPList,i))
      temp = bz_getBanItem(eIPList,i);
  }
  else if (key == "ipbanreason")
  {
    if (bz_getBanItemReason(eIPList,i))
      temp = bz_getBanItemReason(eIPList,i);
  }
  else if (key == "ipbansource")
  {
    if (bz_getBanItemSource(eIPList,i))
      temp = bz_getBanItemSource(eIPList,i);
  }
  else if (key == "ipbandurration")
      temp = format("%f",bz_getBanItemDuration(eIPList,i));
  else if (key == "ipbanid")
    temp = format("%d",item);

  if (temp.size())
    data += temp;
}

bool IPBanLoop::getIF  (size_t item, const std::string &key)
{
  if (key == "ipbanfrommaster")
    return bz_getBanItemIsFromMaster(eIPList,(unsigned int)item);
  if (key == "ipbanisforever")
    return bz_getBanItemDuration(eIPList,(unsigned int)item) < 0;

  return false;
}

HostBanLoop::HostBanLoop(Templateiser &ts)
{
  ts.addLoop("HostBans",this);
  ts.addKey("HostBanMask",this);
  ts.addKey("HostBanReason",this);
  ts.addKey("HostBanSource",this);
  ts.addKey("HostBanDuration",this);
  ts.addKey("HostBanID",this);
  ts.addIF("HostBanIsForever",this);
}

void HostBanLoop::setSize ( void )
{
  size = bz_getBanListSize(eHostList);
}

void HostBanLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  std::string temp;
  unsigned int i = (unsigned int )item;

  if (key == "hostbanmask")
  {
    if (bz_getBanItem(eHostList,i))
      temp = bz_getBanItem(eHostList,i);
  }
  else if (key == "hostbanreason")
  {
    if (bz_getBanItemReason(eHostList,i))
      temp = bz_getBanItemReason(eHostList,i);
  }
  else if (key == "hostbansource")
  {
    if (bz_getBanItemSource(eHostList,i))
      temp = bz_getBanItemSource(eHostList,i);
  }
  else if (key == "hostbandurration")
    temp = format("%f",bz_getBanItemDuration(eHostList,i));
  else if (key == "hostbanid")
    temp = format("%d",item);

  if (temp.size())
    data += temp;
}

bool HostBanLoop::getIF  (size_t item, const std::string &key)
{
  if (key == "hostbanisforever")
    return bz_getBanItemDuration(eHostList,(unsigned int)item) < 0;

  return false;
}

IDBanLoop::IDBanLoop(Templateiser &ts)
{
  ts.addLoop("IDBans",this);
  ts.addKey("IDBanMask",this);
  ts.addKey("IDBanReason",this);
  ts.addKey("IDBanSource",this);
  ts.addKey("IDBanDuration",this);
  ts.addKey("IDBanID",this);
  ts.addIF("IDBanIsForever",this);
}

void IDBanLoop::setSize ( void )
{
  size = bz_getBanListSize(eIDList);
}

void IDBanLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  std::string temp;
  unsigned int i = (unsigned int )item;

  if (key == "idbanmask")
  {
    if (bz_getBanItem(eIDList,i))
      temp = bz_getBanItem(eIDList,i);
  }
  else if (key == "idbanmask")
  {
    if (bz_getBanItem(eIDList,i))
      temp = bz_getBanItem(eIDList,i);
  }
  else if (key == "idbanreason")
  {
    if (bz_getBanItemReason(eIDList,i))
      temp = bz_getBanItemReason(eIDList,i);
  }
  else if (key == "idbansource")
  {
    if (bz_getBanItemSource(eIDList,i))
      temp = bz_getBanItemSource(eIDList,i);
  }
  else if (key == "idbandurration")
    temp = format("%f",bz_getBanItemDuration(eIDList,i));
  else if (key == "idbanid")
    temp = format("%d",item);

  if (temp.size())
    data += temp;
}

bool IDBanLoop::getIF  (size_t item, const std::string &key)
{
  if (key == "idbanisforever")
    return bz_getBanItemDuration(eHostList,(unsigned int)item) < 0;

  return false;
}

PlayerGroupLoop::PlayerGroupLoop(Templateiser &ts)
{
  ts.addLoop("PlayerGroups",this);
  ts.addKey("PlayerGroup",this);
}

void PlayerGroupLoop::setSize ( void )
{
  groups.clear();
  if (!playerLoop)
    size = 0;
  else
  {
    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerLoop->getPlayerID());
    if (!player)
      size = 0;
    else
    {
      for (unsigned int i = 0; i < player->groups.size(); i++)
      {
	if (player->groups.get(i).size())
	  groups.push_back(std::string(player->groups.get(i).c_str()));
      }
      size = groups.size();
      bz_freePlayerRecord(player);
    }
  }
}

void PlayerGroupLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  if (key == "playergroup")
    data += groups[item];
}

FlagHistoryLoop::FlagHistoryLoop(Templateiser &ts)
{
  ts.addLoop("FlagHistory",this);
  ts.addKey("FlagHistoryItem",this);
}

void FlagHistoryLoop::setSize ( void )
{
  history.clear();
  if (!playerLoop)
    size = 0;
  else
  {
    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerLoop->getPlayerID());
    if (!player)
      size = 0;
    else
    {
      for (unsigned int i = 0; i < player->flagHistory.size(); i++)
      {
	if (player->flagHistory.get(i).size())
	  history.push_back(std::string(player->flagHistory.get(i).c_str()));
      }
      size = history.size();
      bz_freePlayerRecord(player);
    }
  }
}

void FlagHistoryLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  if (key == "flaghistoryitem")
    data += history[item];
}

ReportsLoop::ReportsLoop(Templateiser &ts)
{
  ts.addLoop("Reports",this);
  ts.addKey("ReportID",this);
  ts.addKey("ReportTime",this);
  ts.addKey("ReportSource",this);
  ts.addKey("ReportBody",this);
}

void ReportsLoop::setSize ( void )
{
  size = bz_getReportCount();
}

void ReportsLoop::getKey (size_t item, std::string &data, const std::string &key)
{
  unsigned int i = (unsigned int)item;
  if (key == "reportid")
    data += format("%d",i);
  else if (key == "reporttime")
    data += bz_getReportTime(i);
  else if (key == "reportsource")
    data += bz_getReportSource(i);
  else if (key == "reportbody")
    data += bz_getReportBody(i);
}




// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
