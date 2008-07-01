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
	WebAdmin():BZFSHTTPAuth(),it(bzu_standardPerms.end()){registerVDir();}

	virtual const char * getVDir ( void ){return "webadmin";}
	virtual const char * getDescription ( void ){return "Server Administration (Login Required)";}

  void init (const char *tDir);

	virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );
 
  // from TemplateCallbackClass
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool loopCallback (const std::string &key);
  virtual bool ifCallback (const std::string &key);

private:
	std::map<std::string,std::string> tmplvars;
	std::vector<std::string>::const_iterator it;
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
	std::map<std::string,std::string>::iterator pair = tmplvars.find(key);
	if (pair != tmplvars.end()) data = pair->second;
}

// condition check for [*START] in templates
bool WebAdmin::loopCallback (const std::string &key)
{
	if (key == "players") {
		if (it == bzu_standardPerms.end()) return false;
		tmplvars["callsign"] = *it++;
		return true;
	}
  return false;
}

// condition check for [?IF] in templates
bool WebAdmin::ifCallback (const std::string &key)
{
  return false;
}

bool WebAdmin::handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply )
{
  std::string &page = reply.body;

	if (level == 1) {
		page = format("vdir=%s, resource=%s, baseURL=%s", request.vdir.c_str(), request.resource.c_str(), request.baseURL.c_str());
	} else if ( level == VERIFIED)	// valid login but does not have admin groups
		page = format("Not authenticated(Verified) sessionID %d",request.sessionID);
	else
		page = format("Not authenticated sessionID %d",request.sessionID);

	reply.docType = HTTPReply::eHTML;
	//templateSystem.processTemplate(reply.body, )

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
