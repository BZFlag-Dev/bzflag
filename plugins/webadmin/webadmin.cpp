// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_HTTPTemplates.h"

class WebAdmin : public BZFSHTTPServer, TemplateCallbackClass
{
public:
  WebAdmin(const char * plugInName);

  void init (std::string &tDir);

  // from BSFSHTTPServer
  virtual bool acceptURL ( const char *url ) { return true; }
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );

  Templateiser templateSystem;

  // from TemplateCallbackClass
  virtual void keyCallback ( std::string &data, const std::string &key );
  virtual bool loopCallback ( const std::string &key );
  virtual bool ifCallback ( const std::string &key );
};

WebAdmin webAdmin("webadmin");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  webAdmin.init(std::string(commandLine ? commandLine : "./"));

  bz_debugMessage(4,"webadmin plugin loaded");
  webAdmin.startupHTTP();
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  webAdmin.shutdownHTTP();
  bz_debugMessage(4,"webadmin plugin unloaded");
  return 0;
}

WebAdmin::WebAdmin(const char *plugInName)
: BZFSHTTPServer(plugInName)
{
}

void WebAdmin::init(std::string &tDir)
{
  templateSystem.addSearchPath(tDir.c_str());

  /* template symbols go here */

  templateSystem.setPluginName("Web Report", getBaseServerURL());
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

void WebAdmin::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  std::string page;

  page = "default";

  setURLDocType(eText,requestID);
  setURLDataSize(page.size(),requestID);
  setURLData(page.c_str(),requestID);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
