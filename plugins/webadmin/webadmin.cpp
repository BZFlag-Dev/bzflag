/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

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

class WebAdmin : public BZFSHTTPAuth {
  public:
    WebAdmin();
    ~WebAdmin();
    virtual const char* getVDir(void) {return "webadmin";}
    virtual const char* getDescription(void) {return "Server Administration (Login Required)";}

    void init(const char* tDir);

    virtual bool handleAuthedRequest(int level, const HTTPRequest& request, HTTPReply& reply);

    void buildAuthPermsFromPages(void);
    void fillPageList(std::vector<std::string> &pages);

  protected:
    void addAction(Action* action);

  private:
    std::map<std::string, Action*> actions;
};

WebAdmin* webAdmin = NULL;

std::string binRoot;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine) {
  // save off where we are
  binRoot = bz_pluginBinPath();

  if (webAdmin) {
    delete(webAdmin);
  }
  webAdmin = new WebAdmin;
  webAdmin->init(commandLine);

  bz_debugMessage(4, "webadmin plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void) {
  if (webAdmin) {
    delete(webAdmin);
  }
  webAdmin = NULL;

  bz_debugMessage(4, "webadmin plugin unloaded");
  return 0;
}

WebAdmin::WebAdmin(): BZFSHTTPAuth() {
  registerVDir();
}

void WebAdmin::addAction(Action* action) {
  if (action && action->name()) {
    std::string name;
    toupper(action->name(), name);
    actions[name] = action;
  }
  else if (action) {
    delete(action);
  }
}

WebAdmin::~WebAdmin() {
  freeCommonItems();
  freeLoops();
  std::map<std::string, Action*>::iterator itr = actions.begin();
  while (itr != actions.end()) {
    if (itr->second) {
      delete(itr->second);
    }

    itr++;
  }
}

void WebAdmin::init(const char* cmdln) {
  actions.clear();

  if (cmdln && strlen(cmdln)) {
    templateSystem.addSearchPath(cmdln);
    serviceMimeResources = true;
    resourceRootPath = cmdln;
  }
  else if (binRoot.size()) {
    // ok make a heroic effort to find the 404 item, cus if we can find that we got us a template dir
    std::string searchpath = concatPaths(binRoot, "templates/");
    if (!fileExists(searchpath + "404.tmpl")) {
      searchpath = concatPaths(binRoot, "webadmin/templates/");
      if (!fileExists(searchpath + "404.tmpl")) {
        searchpath = concatPaths(binRoot, "plugins/webadmin/templates/");
        if (!fileExists(searchpath + "404.tmpl")) {
          searchpath = "";
        }
      }
    }

    if (searchpath.size()) {
      bz_debugMessagef(1, "webadmin: using template path %s", searchpath.c_str());

      templateSystem.addSearchPath(searchpath.c_str());
      serviceMimeResources = true;
      resourceRootPath = searchpath;
    }
  }
  else {
    bz_debugMessage(0, "No paths found, unable to run, BLAAAAGGGGG!");
  }

  initLoops(templateSystem);
  initCommonItems(templateSystem);

  addAction(new UpdateBZDBVars());
  addAction(new SendChatMessage());
  addAction(new SaveLogFile());
  addAction(new ClearLogFile());
  addAction(new AddBan());
  addAction(new RemoveBan());
  addAction(new RemoveReport());
  addAction(new AddReport());

  buildAuthPermsFromPages();

  templateSystem.setPluginName("webadmin", getBaseURL().c_str());

  setupAuth();
}

void WebAdmin::fillPageList(std::vector<std::string> &pages) {
  std::vector<std::string> templateDirs = templateSystem.getSearchPaths();

  pages.clear();

  for (size_t d = 0; d < templateDirs.size(); d++) {
    std::vector<std::string> files = getFilesInDir(templateDirs[d], "*.page", false);

    for (size_t f = 0; f < files.size(); f++) {
      pages.push_back(getFileTitle(files[f]) + "." + getFileExtension(files[f]));
    }
  }
}

void WebAdmin::buildAuthPermsFromPages(void) {
  std::vector<std::string> pages;
  fillPageList(pages);
  for (size_t i = 0; i < pages.size(); i++) {
    TemplateMetaData meta;
    templateSystem.getTemplateFileMetaData(meta, pages[i].c_str());
    if (meta.exists("RequirePerm")) {
      std::vector<std::string> items = meta.get("RequirePerm");
      for (size_t i = 0; i < items.size(); i++) {
        addPermToLevel(1, items[i]);
      }
    }
  }
}

bool WebAdmin::handleAuthedRequest(int level, const HTTPRequest& request, HTTPReply& reply) {
  std::string pagename = request.resource;
  std::string filename;

  if (userInfo) {
    userInfo->sessionAuth = this;
    userInfo->sessionID = request.sessionID;
    userInfo->userName = "";
    if (getSessionUser(request.sessionID)) {
      userInfo->userName = getSessionUser(request.sessionID);
    }
  }

  if (serverError) { // clear out the error message
    serverError->errorMessage = "";
  }

  reply.returnCode = HTTPReply::e200OK;
  reply.docType = HTTPReply::eHTML;

  switch (level) {
    case 1: {
      if (pagename.size()) {
        size_t size = pagename.size();
        if (size > 0 && pagename[size - 1] == '/') {
          pagename.erase(size - 1);
        }
      }

      if (pagename == "logout") {
        invalidateSession(request.sessionID);
        reply.returnCode = HTTPReply::e301Redirect;
        reply.redirectLoc = std::string("/") + getVDir();
        return true;
      }

      // check for actions, if we have a userinfo
      std::string action;
      if (userInfo && request.getParam("action", action)) {
        // find an action handler, call it, and get the pagename back
        makeupper(action);
        if (actions.find(action) != actions.end()) {
          if (actions[action]->process(pagename, request, reply)) {
            return true;
          }
        }
      }

      if (!pagename.size()) {
        pagename = "Main";
      }

      // build up the list of pages by permision
      std::vector<std::string> allPages;
      std::vector<std::string> authedPages;

      bool requestedPageIsAuthed = false;
      fillPageList(allPages);
      for (size_t i = 0; i < allPages.size(); i++) {
        TemplateMetaData meta;
        templateSystem.getTemplateFileMetaData(meta, allPages[i].c_str());
        if (meta.exists("RequirePerm")) {
          std::vector<std::string> items = meta.get("RequirePerm");
          bool hasPerms = items.size() > 0;
          for (size_t s = 0; s < items.size(); s++) {
            // the user must have all the perms for the page
            if (!getSessionPermision(request.sessionID, items[s])) {
              hasPerms = false;
            }
          }
          if (hasPerms) {
            authedPages.push_back(getFileTitle(allPages[i]));

            if (find_first_substr(allPages[i], pagename) != std::string::npos) {
              requestedPageIsAuthed = true;
            }
          }
        }
      }

      if (!authedPages.size()) {
        reply.body = format("Verified user, but no authorized pages found");
        return true;
      }

      // do something better here to get a valid default page if main isn't authed
      std::string ext = getFileExtension(pagename);

      if (ext.size()) {
        if (ext == "tmpl") { // verify the perms on the template page
          TemplateMetaData meta;
          templateSystem.getTemplateFileMetaData(meta, pagename.c_str());
          if (meta.exists("RequirePerm")) {
            std::vector<std::string> items = meta.get("RequirePerm");
            bool hasPerms = items.size() > 0;
            for (size_t s = 0; s < items.size(); s++) {
              // the user must have all the perms for the page
              if (!getSessionPermision(request.sessionID, items[s])) {
                hasPerms = false;
              }
            }
            if (!hasPerms) {
              reply.body = format("Page Not Authorized");
              return true;
            }
          }
        }
      }
      else { // it's a no name page
        if (!requestedPageIsAuthed) {
          pagename = "Main";
        }
      }

      if (navLoop) {
        navLoop->currentTemplate = pagename;
        navLoop->pages = authedPages;
        navLoop->computePageList();
      }

      callNewPageCallbacks(pagename, request);

      if (!ext.size()) {
        pagename += ".page";
      }
      // if it's regular page then go for it
      if (!templateSystem.processTemplateFile(reply.body, pagename.c_str())) {
        reply.returnCode = HTTPReply::e404NotFound;
        if (!templateSystem.processTemplateFile(reply.body, "404.tmpl")) {
          reply.body = format("No such resource: %s", pagename.c_str());
        }
      }
    }
    break;

    case VERIFIED:
      reply.body = format("Verified user, but no authorized permission groups found");
      break;

    case UNAUTHENTICATED:
    default:
      reply.body = format("Not authenticated user, global auth verify failed");
      break;
  }

  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
