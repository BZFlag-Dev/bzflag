// pages.h : Defines the entry point for the DLL application.
//

#ifndef _PAGES_H_
#define _PAGES_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

class CommonTemplateCallbacks : public TemplateCallbackClass
{
public:
  CommonTemplateCallbacks(Templateiser *ts) : baseTemplateSystem(ts)
  {
    baseTemplateSystem.addIF("IsCurrentPage",this);
  }

  virtual ~CommonTemplateCallbacks(void){};

  // do the current page and common page data here
  virtual void keyCallback(std::string &data , const std::string & key ) {};
  virtual bool loopCallback(const std::string & key ) { return false; }
  virtual bool ifCallback(const std::string &key) { return false; }

protected:
  Templateiser baseTemplateSystem;
};

class BasePageHandler : CommonTemplateCallbacks
{
public:
  BasePageHandler(Templateiser *ts): CommonTemplateCallbacks(ts), templateSystem(baseTemplateSystem){init();}
  virtual ~BasePageHandler(void){};

  virtual void init ( void ) {};
  virtual bool requireAuth ( void ){return true;}

  virtual void process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply ) = 0;

  virtual void keyCallback(std::string &data , const std::string & key ) { return CommonTemplateCallbacks::keyCallback(data,key);}
  virtual bool loopCallback(const std::string & key ) { return CommonTemplateCallbacks::loopCallback(key); }
  virtual bool ifCallback(const std::string &key) { return CommonTemplateCallbacks::ifCallback(key); }
protected:

  Templateiser templateSystem;
};

class Mainpage : public BasePageHandler
{
public:
  Mainpage(Templateiser *ts): BasePageHandler(ts){};

  virtual void process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply );
};

class Banlistpage : public BasePageHandler
{
public:
  Banlistpage(Templateiser *ts): BasePageHandler(ts){};
  virtual void process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply );
};

class HelpMsgpage : public BasePageHandler
{
public:
  HelpMsgpage(Templateiser *ts): BasePageHandler(ts){};
 virtual void process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply );
};

class GroupPage : public BasePageHandler
{
public:
  GroupPage(Templateiser *ts): BasePageHandler(ts){};
 virtual void process ( const std::string &pagename, const HTTPRequest &request, HTTPReply &reply );
};



#endif //_PAGES_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
