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
  WebAdmin::WebAdmin();
  virtual const char * getVDir ( void ){return "webadmin";}
  virtual const char * getDescription ( void ){return "Server Administration (Login Required)";}

  void init (const char *tDir);

  virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );
 
  // from TemplateCallbackClass
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool loopCallback (const std::string &key);
  virtual bool ifCallback (const std::string &key);

private:
  typedef std::vector<std::string>::const_iterator loop_pos;
  loop_pos *loopItr;
  std::map<std::string,std::string> templateVars;

	std::vector<std::string> pages
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

WebAdmin::WebAdmin():BZFSHTTPAuth(),loopItr(NULL)
{
	pages.push_back("main");
	pages.push_back("banlist");
	pages.push_back("helpmsg");
	pages.push_back("group");
	registerVDir();
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
    if (!loopItr) loopItr = new loop_pos(bzu_standardPerms.begin());
    else if (*loopItr != bzu_standardPerms.end()) {
      templateVars["callsign"] = **loopItr++;
      return true;
    }
  } else if (key == "navigation") {
    if (!loopItr) loopItr = new loop_pos(pages.begin());
    else if (*loopItr != pages.end()) {
      templateVars["pagename"] = **loopItr++;
      return true;
    }
  } else return false;
  delete(loopItr);
  return loopItr == NULL;
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
  loop_pos page_iter;
  size_t last;
  std::string pagename = request.resource;

  switch(level) {
  case 1:
  case VERIFIED:
    last = pagename.size()-1;
    if (pagename[last] == '/') pagename.erase(last);
    if (pagename == "") pagename = "main";
    // std::map<std::string,controller>::iterator pair = controllers.find(pagename);
		page_iter = std::find(pages.begin(), pages.end(), pagename);
    // if (pair != controllers.end()) {
    if (page_iter != pages.end()) {
      // if (pair->second) (this->*(pair->second))(request);
      if (!templateSystem.processTemplateFile(reply.body, (pagename + ".tmpl").c_str())) {
        reply.returnCode = HTTPReply::e500ServerError;
        if (!templateSystem.processTemplateFile(reply.body, "505.tmpl"))
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


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=2 expandtab
