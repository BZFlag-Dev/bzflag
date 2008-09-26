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

class Error : public TemplateCallbackClass
{
public:

  Error(Templateiser &ts);
  virtual void keyCallback (std::string &data, const std::string &key);
  virtual bool ifCallback (const std::string &key);

  std::string errorMessage;
};

// cheap singletons
extern Error *error;
extern UserInfo *userInfo;


// newpage callbacks

class NewPageCallback
{
public:
  virtual ~NewPageCallback(){};
  virtual void newPage ( const HTTPRequest &request ) =0;
};

void addNewPageCallback ( NewPageCallback *callback );
void removeNewPageCallback ( NewPageCallback *callback );
void callNewPageCallbacks ( const HTTPRequest &request );


#endif //_COMMON_ITEMS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
