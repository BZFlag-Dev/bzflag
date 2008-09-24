// commonItems.cpp : Defines the entry point for the DLL application.
//

#include "commonItems.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <string>

UserInfo *userInfo = NULL;
Error *error = NULL;

void initCommonItems ( Templateiser &ts )
{
  userInfo = new UserInfo(ts);
  error = new Error(ts);
}

//----------------UserInfo
UserInfo::UserInfo(Templateiser &ts)
{
  ts.addKey("username",this);
}

void UserInfo::keyCallback (std::string &data, const std::string &/*key*/)
{
  data += userName;
}

//--------------Error
Error::Error(Templateiser &ts)
{
  ts.addKey("error",this);
  ts.addIF("error",this);
}

void Error::keyCallback (std::string &data, const std::string &/*key*/)
{
  data += errorMessage;
}

bool Error::ifCallback (const std::string &/*key*/)
{
  return errorMessage.size();
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
