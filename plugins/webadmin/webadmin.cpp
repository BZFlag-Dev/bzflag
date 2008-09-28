// webadmin.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_files.h"
#include <fstream>
#include <cstring>
#include <algorithm>

#include "loops.h"
#include "actions.h"
#include "commonItems.h"

class WebAdmin : public BZFSHTTPAuth
{
public:
  WebAdmin();
  ~WebAdmin();
  virtual const char * getVDir ( void ){return "webadmin";}
  virtual const char * getDescription ( void ){return "Server Administration (Login Required)";}

  void init (const char *tDir);

  virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );

protected:
  void addAction ( Action *action );

private:
  std::map<std::string,Action*> actions;
};

WebAdmin *webAdmin = NULL;

std::string binRoot;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  // save off where we are
  binRoot = bz_pluginBinPath();

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
}

void WebAdmin::addAction ( Action *action )
{
  if (action && action->name())
  {
    std::string name;
    toupper(action->name(),name);
    actions[name] = action;
  }
  else if (action)
    delete(action);
}

WebAdmin::~WebAdmin()
{
  freeCommonItems();
  freeLoops();
  std::map<std::string,Action*>::iterator itr = actions.begin();
  while (itr != actions.end())
  {
    if (itr->second)
      delete(itr->second);

    itr++;
  }
}


void WebAdmin::init(const char* cmdln)
{
  actions.clear();

  if (cmdln && strlen(cmdln))
  {
    templateSystem.addSearchPath(cmdln);
    serviceMimeResources = true;
    resourceRootPath = cmdln;
  }
  else if (binRoot.size())
  {
    // ok make a heroic effort to find the 404 item, cus if we can find that we got us a template dir
    std::string searchpath = concatPaths(binRoot,"templates/");
    if (!fileExists(searchpath+"404.tmpl"))
    {
      searchpath = concatPaths(binRoot,"webadmin/templates/");
      if (!fileExists(searchpath+"404.tmpl"))
      {
	searchpath = concatPaths(binRoot,"plugins/webadmin/templates/");
	if (!fileExists(searchpath+"404.tmpl"))
	  searchpath = "";
      }
    }

    if (searchpath.size())
    {
      bz_debugMessagef(1,"webadmin: using template path %s",searchpath.c_str());

      templateSystem.addSearchPath(searchpath.c_str());
      serviceMimeResources = true;
      resourceRootPath = searchpath;
    }
  }
  else
  {
    bz_debugMessage(0,"No paths found, unable to run, BLAAAAGGGGG!");
  }

  initLoops(templateSystem);
  initCommonItems(templateSystem);

  addAction(new UpdateBZDBVars());
  addAction(new SendChatMessage());
  addAction(new SaveLogFile());

//   level one has admin perms
  addPermToLevel(1,"ADMIN");

  templateSystem.setPluginName("webadmin", getBaseURL().c_str());

  setupAuth();
}

bool WebAdmin::handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply )
{
  std::string pagename = request.resource;
  std::string filename;
//  const char *username;

  if(userInfo)
  {
    userInfo->userName = "";
    if (getSessionUser(request.sessionID))
      userInfo->userName = getSessionUser(request.sessionID);
  }

  if(serverError) // clear out the error message
    serverError->errorMessage ="";

  reply.returnCode = HTTPReply::e200OK;
  reply.docType = HTTPReply::eHTML;

  switch(level) {
  case 1:
  case VERIFIED:
    {
      if (pagename.size())
      {
	size_t size = pagename.size();
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

      // check for actions, fill out an auth thingy or something
      std::string action;
      if (request.getParam("action",action))
      {
	// find an action handler, call it, and get the pagename back
	makeupper(action);
	if (actions.find(action) != actions.end())
	{
	  if(actions[action]->process(pagename,request,reply))
	    return true;
	}
      }

      if (!pagename.size())
	pagename = "main";

      callNewPageCallbacks(pagename,request);

      if (navLoop)
	navLoop->currentTemplate = pagename;

      pagename += ".page";
      // if it's regular page then go for it
      if (!templateSystem.processTemplateFile(reply.body, pagename.c_str()))
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
