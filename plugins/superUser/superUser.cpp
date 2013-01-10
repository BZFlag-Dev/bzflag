// SAMPLE_PLUGIN.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

#include <string>
#include <vector>

class SuperUser : public bz_Plugin
{
public:
  virtual const char* Name (){return "SuperUser";}
  virtual void Init ( const char* config);

  virtual void Event ( bz_EventData * /* eventData */ );

protected:
  std::vector<std::string> GetUserInfo(const char* bzID );

  PluginConfig Users;
};

BZ_PLUGIN(SuperUser)

void SuperUser::Init ( const char* commandLine)
{
  if (commandLine == NULL || strlen(commandLine) == 0)
    bz_debugMessage(0,"SuperUser plugin needs a user file to work from");
  else
    Users.read(commandLine);

  bz_debugMessage(4,"SAMPLE_PLUGIN plugin loaded");
  Register(bz_ePlayerJoinEvent);

  // init events here with Register();
}

std::vector<std::string> SuperUser::GetUserInfo(const char* bzID )
{
  std::vector<std::string> perms;

  std::string info = Users.item("Users",bzID);

  if (info.size() > 0)
    perms = tokenize(info,std::string(","),-1,true);

  return perms;
}

void SuperUser::Event ( bz_EventData * eventData  )
{
  bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;

  std::vector<std::string> perms = GetUserInfo(joinData->record->bzID.c_str());

  for (size_t i = 0; i < perms.size(); i++)
    bz_grantPerm(joinData->playerID,perms[i].c_str());
}
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

