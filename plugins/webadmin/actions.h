// actions.h : Defines the entry point for the DLL application.
//

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

class Action
{
public:
  virtual ~Action(){};

  virtual bool process ( std::string &page, const HTTPRequest &request, HTTPReply &reply ) = 0;

  virtual const char* name ( void ) = 0;
};

class UpdateBZDBVars : public Action
{
public:
  virtual bool process ( std::string &page, const HTTPRequest &request, HTTPReply &reply );

  virtual const char* name ( void ){ return "updatevars";}

protected:
  bool varChanged ( const char * key , const char * val);
};

class SendChatMessage : public Action
{
public:
  virtual bool process ( std::string &page, const HTTPRequest &request, HTTPReply &reply );

  virtual const char* name ( void ){ return "sendchatmessage";}
};

class SaveLogFile : public Action
{
public:
  virtual bool process ( std::string &page, const HTTPRequest &request, HTTPReply &reply );
  virtual const char* name ( void ){ return "savelog";}
};

class ClearLogFile : public Action
{
public:
  virtual bool process ( std::string &page, const HTTPRequest &request, HTTPReply &reply );
  virtual const char* name ( void ){ return "clearlog";}
};


class KickUser : public Action
{
public:
  virtual bool process ( std::string &page, const HTTPRequest &request, HTTPReply &reply );
  virtual const char* name ( void ){ return "kickuser";}
};

#endif //_ACTIONS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
