#ifndef BZADMIN_H
#define BZADMIN_H

#include <string>
#include <map>

#include "BZAdminUI.h"
#include "ServerLink.h"
#include "Team.h"
#include "Address.h"

using namespace std;


// global variables
extern map<PlayerId, string> players;
extern TeamColor myTeam;

/** Checks for new packets from the server, ignores them or stores a
    text message in str. Tells ui about new or removed players. Returns
    false if no interesting packets have arrived. */
bool getServerString(ServerLink& sLink, string& str, BZAdminUI& ui);

/** Sends the message msg to the server. */
void sendMessage(ServerLink& sLink, const string& msg, PlayerId target);


#endif

