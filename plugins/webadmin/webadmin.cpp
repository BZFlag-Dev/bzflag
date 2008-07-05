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
  size_t loopPos;
  std::map<std::string,std::string> templateVars;
  
  typedef void (WebAdmin::*page_callback)(const HTTPRequest &);
  std::map<std::string,page_callback> controllers;
  std::vector<std::string> pagenames;
  
  void mainPageCallback (const HTTPRequest &request);
  void WebAdmin::banlistPageCallback (const HTTPRequest &request);
  
  bz_APIIntList players;
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
	controllers["main"] = &WebAdmin::mainPageCallback;
	controllers["banlist"] = &WebAdmin::banlistPageCallback;
	controllers["helpmsg"] = NULL;
	controllers["group"] = NULL;
	
  std::map<std::string,page_callback>::iterator pair;
  for(pair = controllers.begin(); pair != controllers.end(); pair++)
    pagenames.push_back(pair->first);
}


void WebAdmin::init(const char* cmdln)
{
  templateSystem.addSearchPath(cmdln ? cmdln : "./");

  // level one has admin perms
  addPermToLevel(1,"ADMIN");
  
  templateSystem.addIF("IsCurrentPage",this);
  templateSystem.addIF("Error",this);

  templateSystem.addKey("Error",this);
  templateSystem.addKey("Callsign",this);
  templateSystem.addKey("BannedUser",this);
  templateSystem.addKey("PageName",this);
  
  templateSystem.addLoop("Navigation",this);
  templateSystem.addLoop("Players",this);
  templateSystem.addLoop("IPBanList",this);
  templateSystem.addLoop("IDBanList",this);

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
    if (!loopPos) bz_getPlayerIndexList(&players);
    else if (loopPos < players.size()) {
      templateVars["playerid"] = players[loopPos];
      templateVars["callsign"] = bz_getPlayerCallsign(players[loopPos++]);
      return true;
    } else {
      players.clear();
      return loopPos = 0;
    }
  } else if (key == "navigation") {
    if (loopPos < pagenames.size()) {
      templateVars["pagename"] = pagenames[loopPos++];
      return true;
    } else return loopPos = 0;
  } else if (key == "permissions") {
    if (loopPos < bzu_standardPerms.size()) {
      templateVars["permission"] = bzu_standardPerms[loopPos++];
      return true;
    } else return loopPos = 0;
  } else return false;
}

// condition check for [?IF] in templates
bool WebAdmin::ifCallback (const std::string &key)
{
  if (key == "iscurrentpage")
    return templateVars["pagename"] == templateVars["currentpage"];
  return false;
}

bool WebAdmin::handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply )
{
  std::map<std::string,page_callback>::iterator pair;
  size_t last;
  std::string pagename = request.resource;

  switch(level) {
  case 1:
  case VERIFIED:
    last = pagename.size()-1;
    if (pagename[last] == '/') pagename.erase(last);
    if (pagename.empty()) pagename = "main";
    pair = controllers.find(pagename);
    if (pair != controllers.end()) {
      (this->*pair->second)(request);
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
    reply.body = format("Not authenticated sessionID %d",request.sessionID);
  }

  reply.docType = HTTPReply::eHTML;

  templateVars.clear();
  return true;
}

void WebAdmin::mainPageCallback (const HTTPRequest &request)
{
  if (request.request != ePost) return;
  std::vector<std::string> players;
  if (!request.getParam("players", players)) return;
  std::string dummy, reason;
  bool notify = request.getParam("notify", dummy);
  request.getParam("reason", reason);
  std::vector<std::string>::iterator i;
  if (request.getParam("kick", dummy))
    for (i = players.begin(); i != players.end(); i++)
      bz_kickUser(atoi(i->c_str()), reason.c_str(), notify);
  else if (request.getParam("ipban", players)) {
    request.getParam("duration", dummy);
    int duration = atoi(dummy.c_str());
    for (i = players.begin(); i != players.end(); i++) {
      int playerID = atoi(i->c_str());
      bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
      bz_IPBanUser(playerID, bz_getPlayerIPAddress(playerID), duration, reason.c_str());
    }
  }
  else if (request.getParam("idban", players)) {
    request.getParam("duration", dummy);
    int duration = atoi(dummy.c_str());
    for (i = players.begin(); i != players.end(); i++) {
      int playerID = atoi(i->c_str());
      bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
      bz_IPBanUser(playerID, bz_getPlayerCallsign(playerID), duration, reason.c_str());
    }
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=2 expandtab
