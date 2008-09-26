// actions.cpp : Defines the entry point for the DLL application.
//

#include "actions.h"
#include <fstream>
#include <cstring>
#include <algorithm>

#include <map>
#include <vector>
#include <string>

std::string UpdateBZDBVars::process ( const std::string &inputPage, const HTTPRequest &request )
{
  std::map<std::string, std::vector<std::string> >::const_iterator itr = request.parameters.begin();

  while (itr != request.parameters.end())
  {
    const std::string &key = itr->first;
    if ( itr->second.size())
    { 
      // vars only use the first param with the name.
      const std::string val = url_decode(itr->second[0]);
      if (strncmp(key.c_str(),"var",3) == 0)
      {
	// it's a var, 
	if ( varChanged(key.c_str()+3,val.c_str()))
	  bz_updateBZDBString(key.c_str()+3,val.c_str());
      }
    }
    itr++;
  }
  return inputPage;
}

bool UpdateBZDBVars::varChanged ( const char * key , const char * val)
{
  if (!bz_BZDBItemExists(key))
    return false;
  return bz_getBZDBString(key) != val;
}

std::string SendChatMessage::process ( const std::string &inputPage, const HTTPRequest &request )
{
  std::string message;

  if (request.getParam("message",message) && message.size())
    bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,message.c_str());
  return inputPage;
}
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
