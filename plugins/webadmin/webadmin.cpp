// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"

class WebAdmin : public BZFSHTTPVDirAuth, TemplateCallbackClass
{
public:
	WebAdmin():BZFSHTTPVDirAuth(){registerVDir();}

	virtual const char * getVDir ( void ){return "WebAdmin";}
	virtual const char * getDescription ( void ){return "Server Administration(Login Required)";}

  void init (const char *tDir);

	virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );
 
  // from TemplateCallbackClass
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool loopCallback (const std::string &key);
  virtual bool ifCallback (const std::string &key);
  
private:
};

WebAdmin *webAdmin = NULL;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
	if(webAdmin)
		delete(webAdmin);
	webAdmin = new WebAdmin;

  if (commandLine && strlen(commandLine))
    webAdmin->init(commandLine);
  else
    webAdmin->init("./");

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

void WebAdmin::init(const char* tDir)
{
  if (tDir)
    templateSystem.addSearchPath(tDir);

	// level one has admin perms
	addPermToLevel(1,"ADMIN");

  /* template symbols go here */

  templateSystem.setPluginName("WebReport", getBaseURL().c_str());
}

// event hook for [$Something] in templates
void WebAdmin::keyCallback (std::string &data, const std::string &key)
{
}

// condition check for [*START] in templates
bool WebAdmin::loopCallback (const std::string &key)
{
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

	if (level == 1)
		page = format("authenticated sessionID %d",request.sessionID);
	else if ( level == VERIFIED)	// valid login but does not have admin groups
		page = format("Not authenticated(Verified) sessionID %d",request.sessionID);
	else
		page = format("Not authenticated sessionID %d",request.sessionID);

	reply.docType = HTTPReply::eHTML;

	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=2 expandtab
