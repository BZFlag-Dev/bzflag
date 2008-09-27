// commonItems.h : Defines the entry point for the DLL application.
//

#ifndef _COMMON_ITEMS_H_
#define _COMMON_ITEMS_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

void initCommonItems ( Templateiser &ts );

class UserInfo : public TemplateCallbackClass
{
public:

  UserInfo(Templateiser &ts);
  virtual void keyCallback (std::string &data, const std::string &key);

  std::string userName;
};

class ServerError : public TemplateCallbackClass
{
public:

  ServerError(Templateiser &ts);
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool ifCallback (const std::string &key);

  std::string errorMessage;
};

class GameInfo : public TemplateCallbackClass, bz_EventHandler
{
public:

  GameInfo(Templateiser &ts);
  virtual void keyCallback (std::string &data, const std::string &key);

  virtual void process(bz_EventData *eventData);

protected:
  std::string mapFile;
};

// cheap singletons
extern ServerError *serverError;
extern UserInfo *userInfo;

// newpage callbacks

class NewPageCallback
{
public:
  virtual ~NewPageCallback(){};
  virtual void newPage ( const std::string &pagename, const HTTPRequest &request ) =0;
};

void addNewPageCallback ( NewPageCallback *callback );
void removeNewPageCallback ( NewPageCallback *callback );
void callNewPageCallbacks ( const std::string &pagename, const HTTPRequest &request );


#endif //_COMMON_ITEMS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
