#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <map>

#include "Address.h"


/** This struct stores information about a player that is relevant to 
    bzadmin. */
struct PlayerInfo {
  std::string name;
  std::string ip;
};


typedef std::map<PlayerId, PlayerInfo> PlayerIdMap;


#endif
