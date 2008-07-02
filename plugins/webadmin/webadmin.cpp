// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include <fstream>
#include <cstring>

class WebAdmin : public BZFSHTTPAuth, TemplateCallbackClass
{
public:
  WebAdmin::WebAdmin():BZFSHTTPAuth(),it(NULL) {registerVDir();}

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
  loop_pos *it;
  std::map<std::string,std::string> tmplvars;
};

WebAdmin *webAdmin = NULL;

//typedef void (WebAdmin::*controller)(const HTTPRequest &);
//std::map<std::string,controller> controllers;
const std::vector<std::string> pages = tokenize(
    "main "
    "banlist "
    "helpmsg "
    "group "
  , std::string(" "), 0, false);

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
  const std::map<std::string,std::string>::iterator &pair = tmplvars.find(key);
  if (pair != tmplvars.end()) data = pair->second;
}

// condition check for [*START] in templates
bool WebAdmin::loopCallback (const std::string &key)
{
  if (key == "players") {
    if (!it) it = new loop_pos(bzu_standardPerms.begin());
    else if (*it != bzu_standardPerms.end()) {
      tmplvars["callsign"] = **it++;
      return true;
    }
  } else if (key == "navigation") {
    if (!it) it = new loop_pos(pages.begin());
    else if (*it != pages.end()) {
      tmplvars["pagename"] = **it++;
      return true;
    }
  } else return false;
  delete(it);
  return it = NULL;
}

// condition check for [?IF] in templates
bool WebAdmin::ifCallback (const std::string &key)
{
  if (key == "iscurrentpage")
    return tmplvars["pagename"] == tmplvars["currentpage"];
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
    page_iter = find(pages.begin(), pages.end(), pagename);
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

  tmplvars.clear();
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=2 expandtab
