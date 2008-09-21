// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include <fstream>
#include <cstring>
#include <algorithm>

#include "pages.h"

class LoopStatus
{
public:
  size_t pos;
  size_t end;

  bool isEnded ( void )
  {
    return pos >= end;
  }

  bool isSet ( void )
  {
    return end == 0;
  }

  void clear ( void )
  {
    pos = end = 0;
  }
};

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
  void clearLoops ( void );
 
//   unsigned int loopPos;
//   std::map<std::string,std::string> templateVars;

//   std::vector<std::string> pagenames;

//   bool editing, checked;

  typedef std::map<std::string,BasePageHandler*> PageHandlerMap;
  PageHandlerMap  pageHandlers;

//    std::string generatedPage;
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

WebAdmin::WebAdmin():BZFSHTTPAuth()
{
  registerVDir();

  pageHandlers["main"] = new Mainpage(&templateSystem);
  pageHandlers["banlist"] = new Banlistpage(&templateSystem);
  pageHandlers["helpmsg"] = new HelpMsgpage(&templateSystem);
  pageHandlers["group"] = new GroupPage(&templateSystem);
}

void WebAdmin::init(const char* cmdln)
{
  templateSystem.addSearchPath(cmdln ? cmdln : "./");

//   level one has admin perms
//     addPermToLevel(1,"ADMIN");
//   
//     templateSystem.addIF("Error",this);
//     templateSystem.addIF("Editing",this);
//     templateSystem.addIF("Checked",this);
//   
//     templateSystem.addKey("Error",this);
//     templateSystem.addKey("UserName",this);
//     templateSystem.addKey("Callsign",this);
//     templateSystem.addKey("BannedUser",this);
//     templateSystem.addKey("PageName",this);
//     templateSystem.addKey("CurrentPage",this);
//     templateSystem.addKey("HelpMsgName",this);
//     templateSystem.addKey("HelpMsgBody",this);
//     templateSystem.addKey("GroupName",this);
//     templateSystem.addKey("Permission",this);
//     templateSystem.addKey("ServerVarName",this);
//     templateSystem.addKey("ServerVarValue",this);
//   
//     templateSystem.addLoop("Navigation",this);
//     templateSystem.addLoop("Players",this);
//     templateSystem.addLoop("ServerVars",this);
//     templateSystem.addLoop("IPBanList",this);
//     templateSystem.addLoop("IDBanList",this);
//     templateSystem.addLoop("HelpMsgs",this);
//     templateSystem.addLoop("Groups",this);
//     templateSystem.addLoop("Permissions", this);
//   
//     templateSystem.setPluginName("webadmin", getBaseURL().c_str());

  setupAuth();
}

// event hook for [$Something] in templates
void WebAdmin::keyCallback (std::string &data, const std::string &key)
{
//   const std::map<std::string,std::string>::iterator &pair = templateVars.find(key);
//   if (pair != templateVars.end())
//     data = pair->second;
}

// condition check for [*START] in templates
bool WebAdmin::loopCallback (const std::string &key)
 {
//   if (key == "players") {
//     if (!loopPos) {
//       bz_getPlayerIndexList(&players);
//       listSize = players.size();
//     }
//     if (loopPos < listSize) {
//       templateVars["playerid"] = players[loopPos];
//       templateVars["callsign"] = bz_getPlayerCallsign(players[loopPos++]);
//       return true;
//     } else {
//       players.clear();
//     }
//   } else if (key == "navigation") {
//     if (!loopPos)
//       listSize = pagenames.size();
//     if (loopPos < pagenames.size()) {
//       templateVars["pagename"] = pagenames[loopPos++];
//       return true;
//     }
//   } else if (key == "permissions") {
//     if (!loopPos)
//       listSize = bzu_standardPerms().size();
//     if (loopPos < listSize) {
//       const std::string perm = bzu_standardPerms()[loopPos++];
//       if (stringList)
// 	checked = stringList->contains(perm);
//       templateVars["permission"] = perm;
//       return true;
//     } else {
//       bz_deleteStringList(stringList);
//     }
//   } else if (key == "helpmsgs") {
//     if (!loopPos) {
//       stringList = bz_getHelpTopics();
//       listSize = stringList->size();
//     }
//     if (loopPos < listSize) {
//       templateVars["helpmsgname"] = (*stringList)[loopPos++].c_str();
//       return true;
//     } else {
//       bz_deleteStringList(stringList);
//     }
//   } else if (key == "groups") {
//     if (!loopPos) {
//       stringList = bz_getGroupList();
//       listSize = stringList->size();
//     }
//     if (loopPos < listSize) {
//       templateVars["groupname"] = (*stringList)[loopPos++].c_str();
//       return true;
//     } else {
//       bz_deleteStringList(stringList);
//     }
//   } else if (key == "servervars") {
//     if (!loopPos) {
//       stringList = bz_newStringList();
//       listSize = bz_getBZDBVarList(stringList);
//     }
//     if (loopPos < listSize) {
//       const char *varname = (*stringList)[loopPos++].c_str();
//       templateVars["servervarname"] = varname;
//       templateVars["servervarvalue"] = bz_getBZDBString(varname).c_str();
//       return true;
//     } else {
//       bz_deleteStringList(stringList);
//     }
//   } else {
    return false;
//   }
//   return loopPos = 0;
}

// condition check for [?IF] in templates
bool WebAdmin::ifCallback (const std::string &key)
{
//   if (key == "iscurrentpage")
//     return templateVars["pagename"] == templateVars["currentpage"];
//   if (key == "editing")
//     return editing;
//   if (key == "checked")
//     return checked;
//   if (key == "error")
//     return templateVars.find("error") != templateVars.end();
  return false;
}

bool WebAdmin::handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply )
{
  size_t size;
  std::string pagename = request.resource;
  std::string filename;
//  const char *username;

  reply.returnCode = HTTPReply::e200OK;
  reply.docType = HTTPReply::eHTML;

  switch(level) {
  case 1:
  case VERIFIED:
    {
      if (pagename.empty())
      {
	pagename = "main";
      } 
      else 
      {
	size = pagename.size();
	if (size > 0 && pagename[size-1] == '/')
	  pagename.erase(size-1);
      }

      if (pagename == "logout")
      {
	invalidateSession(request.sessionID);
	reply.returnCode = HTTPReply::e301Redirect;
	reply.redirectLoc = std::string("/") + getVDir();
	return true;
      }

     // if (username = getSessionUser(request.sessionID))
     //   templateVars["username"] = username;

      PageHandlerMap::iterator itr = pageHandlers.find(pagename);
      if (itr != pageHandlers.end())
      {
	itr->second->process(pagename,request,reply); // add some auth crap in a struct here
      }
      else
      {
	reply.returnCode = HTTPReply::e404NotFound;
	if (!templateSystem.processTemplateFile(reply.body, "404.tmpl"))
	  reply.body = format("No such resource: %s", pagename.c_str());
      }
    }
   break;

  default:
    reply.body = format("Not authenticated sessionID %d", request.sessionID);
  }

  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
