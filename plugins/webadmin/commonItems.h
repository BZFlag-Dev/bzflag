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

// commonItems.h : Defines the entry point for the DLL application.
//

#ifndef _COMMON_ITEMS_H_
#define _COMMON_ITEMS_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

void initCommonItems(Templateiser& ts);
void freeCommonItems(void);

class UserInfo : public TemplateCallbackClass {
  public:

    UserInfo(Templateiser& ts);
    virtual void keyCallback(std::string& data, const std::string& key);
    virtual bool ifCallback(const std::string& key);

    bool hasPerm(const char* perm) {
      if (!sessionAuth) {
        return false;
      }

      return sessionAuth->getSessionPermision(sessionID, perm);
    }

    bool hasPerm(const std::string& perm) {
      if (!sessionAuth) {
        return false;
      }

      return sessionAuth->getSessionPermision(sessionID, perm);
    }

    BZFSHTTPAuth* sessionAuth;
    int   sessionID;
    std::string userName;
};

class ServerError : public TemplateCallbackClass {
  public:

    ServerError(Templateiser& ts);
    virtual void keyCallback(std::string& data, const std::string& key);
    virtual bool ifCallback(const std::string& key);

    std::string errorMessage;
};

class GameInfo : public TemplateCallbackClass, bz_EventHandler {
  public:

    GameInfo(Templateiser& ts);
    virtual ~GameInfo(void);
    virtual void keyCallback(std::string& data, const std::string& key);

    virtual void process(bz_EventData* eventData);

  protected:
    std::string mapFile;

    double startTime;
    size_t bytesIn;
    size_t bytesOut;
};

// cheap singletons
extern ServerError* serverError;
extern UserInfo* userInfo;

// newpage callbacks

class NewPageCallback {
  public:
    virtual ~NewPageCallback() {};
    virtual void newPage(const std::string& pagename, const HTTPRequest& request) = 0;
};

void addNewPageCallback(NewPageCallback* callback);
void removeNewPageCallback(NewPageCallback* callback);
void callNewPageCallbacks(const std::string& pagename, const HTTPRequest& request);


#endif //_COMMON_ITEMS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
