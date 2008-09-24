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

  virtual std::string process ( const HTTPRequest &request ) = 0;

  virtual const char* name ( void ) = 0;
};



#endif //_ACTIONS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
