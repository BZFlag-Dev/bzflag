// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include <fstream>
#include <cstring>
#include <algorithm>

class WebAdmin : public BZFSHTTPAuth, TemplateCallbackClass
{
public:
  WebAdmin();
  virtual const char * getVDir ( void ){return "webadmin";}
  virtual const char * getDescription ( void ){return "Server Administration (Login Required)";}

  void init (const char *tDir);

  virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );
 
  // from TemplateCallbackClass
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool loopCallback (const std::string &key);
  virtual bool ifCallback (const std::string &key);

private:
  unsigned int loopPos;
  std::map<std::string,std::string> templateVars;
  
  std::vector<std::string> pagenames;
  
  void pageCallback (const std::string &pagename, const HTTPRequest &request);
  
  void mainPageCallback (const HTTPRequest &request);
  void banlistPageCallback (const HTTPRequest &request);
  void groupPageCallback (const HTTPRequest &request);
  
  bz_APIIntList players;
  bz_APIStringList *stringList;
  int listSize;
  
  bool editing, checked;
};

WebAdmin *webAdmin = NULL;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  if(webAdmin)
    delete(webAdmin);
  webAdmin = new WebAdmin;
  webAdmin->init(commandLine);

  bz_debugMessage(4,"webadmin plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  if(webAdmin)
    delete(webAdmin);
  webAdmin = NULL;

  bz_debugMessage(4,"webadmin plugin unloaded");
  return 0;
}

WebAdmin::WebAdmin():BZFSHTTPAuth(),loopPos(0)
{  
	registerVDir();
	
	// add new pages here
  pagenames.push_back("main");
  pagenames.push_back("banlist");
  pagenames.push_back("helpmsg");
  pagenames.push_back("group");
}

void WebAdmin::pageCallback(const std::string &pagename, const HTTPRequest &request)
{
       if (pagename == "main")        mainPageCallback(request);
  else if (pagename == "banlist")  banlistPageCallback(request);
  else if (pagename == "helpmsg");
  else if (pagename == "group")      groupPageCallback(request);
}

void WebAdmin::init(const char* cmdln)
{
  templateSystem.addSearchPath(cmdln ? cmdln : "./");

  // level one has admin perms
  addPermToLevel(1,"ADMIN");
  
  templateSystem.addIF("IsCurrentPage",this);
  templateSystem.addIF("Error",this);
  templateSystem.addIF("Editing",this);
  templateSystem.addIF("Checked",this);

  templateSystem.addKey("Error",this);
  templateSystem.addKey("UserName",this);
  templateSystem.addKey("Callsign",this);
  templateSystem.addKey("BannedUser",this);
  templateSystem.addKey("PageName",this);
  templateSystem.addKey("CurrentPage",this);
  templateSystem.addKey("HelpMsgName",this);
  templateSystem.addKey("HelpMsgBody",this);
  templateSystem.addKey("GroupName",this);
  templateSystem.addKey("Permission",this);
  templateSystem.addKey("ServerVarName",this);
  templateSystem.addKey("ServerVarValue",this);
  
  templateSystem.addLoop("Navigation",this);
  templateSystem.addLoop("Players",this);
  templateSystem.addLoop("ServerVars",this);
  templateSystem.addLoop("IPBanList",this);
  templateSystem.addLoop("IDBanList",this);
  templateSystem.addLoop("HelpMsgs",this);
  templateSystem.addLoop("Groups",this);
  templateSystem.addLoop("Permissions", this);

  templateSystem.setPluginName("webadmin", getBaseURL().c_str());

  setupAuth();
}

// event hook for [$Something] in templates
void WebAdmin::keyCallback (std::string &data, const std::string &key)
{
  const std::map<std::string,std::string>::iterator &pair = templateVars.find(key);
  if (pair != templateVars.end())
		data = pair->second;
}

// condition check for [*START] in templates
bool WebAdmin::loopCallback (const std::string &key)
{
  if (key == "players") {
    if (!loopPos) {
      bz_getPlayerIndexList(&players);
      listSize = players.size();
    }
    if (loopPos < listSize) {
      templateVars["playerid"] = players[loopPos];
      templateVars["callsign"] = bz_getPlayerCallsign(players[loopPos++]);
      return true;
    } else players.clear();
  } else if (key == "navigation") {
    if (!loopPos) listSize = pagenames.size();
    if (loopPos < pagenames.size()) {
      templateVars["pagename"] = pagenames[loopPos++];
      return true;
    }
  } else if (key == "permissions") {
    if (!loopPos) listSize = bzu_standardPerms().size();
    if (loopPos < listSize) {
      const std::string &perm = bzu_standardPerms()[loopPos++];
      if (stringList) checked = stringList->contains(perm);
      templateVars["permission"] = perm;
      return true;
    } else delete(stringList);
  } else if (key == "helpmsgs") {
    if (!loopPos) {
      stringList = bz_getHelpTopics();
      listSize = stringList->size();
    }
    if (loopPos < listSize) {
      templateVars["helpmsgname"] = (*stringList)[loopPos++].c_str();
      return true;
    } else delete(stringList);
  } else if (key == "groups") {
    if (!loopPos) {
      stringList = bz_getGroupList();
      listSize = stringList->size();
    }
    if (loopPos < listSize) {
      templateVars["groupname"] = (*stringList)[loopPos++].c_str();
      return true;
    } else delete(stringList);
  } else if (key == "servervars") {
		if (!loopPos) {
			stringList = bz_newStringList();
			listSize = bz_getBZDBVarList(stringList);
		}
    if (loopPos < listSize) {
      const char *varname = (*stringList)[loopPos++].c_str();
      templateVars["servervarname"] = varname;
      templateVars["servervarvalue"] = bz_getBZDBString(varname).c_str();
			return true;
    } else bz_deleteStringList(stringList);
  } else return false;
  return loopPos = 0;
}

// condition check for [?IF] in templates
bool WebAdmin::ifCallback (const std::string &key)
{
  if (key == "iscurrentpage")
    return templateVars["pagename"] == templateVars["currentpage"];
  if (key == "editing")
    return editing;
  if (key == "checked")
    return checked;
  if (key == "error")
    return templateVars.find("error") != templateVars.end();
  return false;
}

bool WebAdmin::handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply )
{
  size_t size;
  std::string pagename = request.resource;
  
  reply.returnCode = HTTPReply::e200OK;
  reply.docType = HTTPReply::eHTML;
  
  switch(level) {
  case 1:
  case VERIFIED:
    templateVars["username"] = getSessionUser(request.sessionID);
    if (pagename.empty()) {
      pagename = "main";
      std::string dummy;
      if (request.getParam("logout",dummy))
        reply.cookies["SessionID"] = "null";
    } else {
      size = pagename.size();
      if (size > 0 && pagename[size-1] == '/') pagename.erase(size-1);
    }
    if (find(pagenames.begin(), pagenames.end(), pagename) != pagenames.end()) {
      templateVars["currentpage"] = pagename;
      pageCallback(pagename, request);
      if (!templateSystem.processTemplateFile(reply.body, (pagename + ".tmpl").c_str())) {
        reply.returnCode = HTTPReply::e500ServerError;
        if (!templateSystem.processTemplateFile(reply.body, "500.tmpl"))
          reply.body = format("Missing template: %s.tmpl", pagename.c_str());
      }
    } else {
      reply.returnCode = HTTPReply::e404NotFound;
      if (!templateSystem.processTemplateFile(reply.body, "404.tmpl"))
        reply.body = format("No such resource: %s", pagename.c_str());
    }
    break;
  //reply.body = format("Not authenticated(Verified) sessionID %d",request.sessionID);
  default:
    reply.body = format("Not authenticated sessionID %d, access level %d",request.sessionID,level);
  }
  
  templateVars.clear();
  editing = false;
  return true;
}

void WebAdmin::mainPageCallback (const HTTPRequest &request)
{
  std::string s1, s2;
  if (request.request != ePost) return;
  std::vector<std::string> v;
  // kick/ban players
  if (request.getParam("players", v)) {
    bool notify = request.getParam("notify", s1);
    request.getParam("reason", s2);
    std::vector<std::string>::iterator i;
    if (request.getParam("kick", s1))
      for (i = v.begin(); i != v.end(); i++)
        bz_kickUser(atoi(i->c_str()), s2.c_str(), notify);
    else if (request.getParam("ipban", v)) {
      request.getParam("duration", s1);
      int duration = atoi(s1.c_str());
      for (i = v.begin(); i != v.end(); i++) {
        int playerID = atoi(i->c_str());
        bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
        bz_IPBanUser(playerID, bz_getPlayerIPAddress(playerID), duration, s2.c_str());
      }
    }
    else if (request.getParam("idban", v)) {
      request.getParam("duration", s1);
      int duration = atoi(s1.c_str());
      for (i = v.begin(); i != v.end(); i++) {
        int playerID = atoi(i->c_str());
        bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
        bz_IPBanUser(playerID, bz_getPlayerCallsign(playerID), duration, s2.c_str());
      }
    }
  }
  // update server vars
  stringList = new bz_APIStringList;
  listSize = bz_getBZDBVarList(stringList);
  for (loopPos = 0; loopPos < listSize; loopPos++) {
    s1 = "var";
    s1 += (*stringList)[loopPos].c_str();
    if (request.getParam(s1, s2))
      bz_setBZDBString((*stringList)[loopPos].c_str(), s2.c_str());
  }
}

void WebAdmin::banlistPageCallback (const HTTPRequest &request)
{
  if (request.request != ePost) return;
  std::vector<std::string> banRemovals;
  std::vector<std::string>::iterator i;
  if (request.getParam("delip", banRemovals))
    for(i = banRemovals.begin(); i != banRemovals.end(); i++)
      bz_IPUnbanUser(i->c_str());
  if (request.getParam("delid", banRemovals))
    for(i = banRemovals.begin(); i != banRemovals.end(); i++)
      bz_IDUnbanUser(i->c_str()); 
}

void WebAdmin::groupPageCallback (const HTTPRequest &request)
{  
  std::string name;
  if (request.getParam("name", name) && request.request == eGet) {
    stringList = bz_getGroupPerms(name.c_str());
    if (!stringList) { // TODO: make a new group instead
      templateVars["error"] = std::string("No such group: ") + name;
    } else {
      templateVars["groupname"] = name;
      editing = true;
    }
    return;
  } else if (request.request == ePost) {
    delete(stringList);
    listSize = bzu_standardPerms().size();
    std::string dummy;
    for (loopPos = 0; loopPos < listSize; loopPos++) {
      if (request.getParam(std::string("perm") + bzu_standardPerms()[loopPos], dummy));
        bz_groupAllowPerm(name.c_str(), bzu_standardPerms()[loopPos].c_str());
      // TODO: else remove permission
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=2 expandtab
