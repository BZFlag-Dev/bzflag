/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "Team.h"
#include "Pack.h"
#include "NetMessage.h"


fvec4 Team::tankColor[NumTeams] = {
  fvec4(0.0f, 0.0f, 0.0f, 1.0f), // rogue    - black
  fvec4(1.0f, 0.0f, 0.0f, 1.0f), // red
  fvec4(0.0f, 1.0f, 0.0f, 1.0f), // green
  fvec4(0.2f, 0.2f, 1.0f, 1.0f), // blue
  fvec4(1.0f, 0.0f, 1.0f, 1.0f), // purple
  fvec4(0.0f, 1.0f, 1.0f, 1.0f), // observer - cyan
  fvec4(1.0f, 1.0f, 1.0f, 1.0f), // rabbit   - white
  fvec4(1.0f, 0.5f, 0.0f, 1.0f)  // hunter   - orange
};

fvec4 Team::radarColor[NumTeams] = {
  fvec4(1.0f,  1.0f,  0.0f,  1.0f), // rogue    - yellow
  fvec4(1.0f,  0.15f, 0.15f, 1.0f), // red
  fvec4(0.2f,  0.9f,  0.2f,  1.0f), // green
  fvec4(0.08f, 0.25,  1.0f,  1.0f), // blue
  fvec4(1.0f,  0.4f,  1.0f,  1.0f), // purple
  fvec4(0.0f,  1.0f,  1.0f,  1.0f), // observer - cyan
  fvec4(1.0f,  1.0f,  1.0f,  1.0f), // rabbit   - white
  fvec4(1.0f,  0.5f,  0.0f,  1.0f)  // hunter   - orange
};


Team::Team() {
  size = 0;
  won  = 0;
  lost = 0;
}


void* Team::pack(void* buf) const {
  buf = nboPackUInt16(buf, uint16_t(size));
  buf = nboPackUInt16(buf, uint16_t(won));
  buf = nboPackUInt16(buf, uint16_t(lost));
  return buf;
}


void Team::pack(NetMessage& netMsg) const {
  netMsg.packUInt16(uint16_t(size));
  netMsg.packUInt16(uint16_t(won));
  netMsg.packUInt16(uint16_t(lost));
}


void* Team::unpack(void* buf) {
  uint16_t inSize, inWon, inLost;
  buf = nboUnpackUInt16(buf, inSize);
  buf = nboUnpackUInt16(buf, inWon);
  buf = nboUnpackUInt16(buf, inLost);
  size = (unsigned short)inSize;
  won = (unsigned short)inWon;
  lost = (unsigned short)inLost;
  return buf;
}


const std::string Team::getImagePrefix(TeamColor team) {
  switch (team) {
    case RedTeam:      { return BZDB.get("redTeamPrefix");      }
    case GreenTeam:    { return BZDB.get("greenTeamPrefix");    }
    case BlueTeam:     { return BZDB.get("blueTeamPrefix");     }
    case PurpleTeam:   { return BZDB.get("purpleTeamPrefix");   }
    case RabbitTeam:   { return BZDB.get("rabbitTeamPrefix");   }
    case HunterTeam:   { return BZDB.get("hunterTeamPrefix");   }
    case ObserverTeam: { return BZDB.get("observerTeamPrefix"); }
    default:           { return BZDB.get("rogueTeamPrefix");    }
  }
}


const char* Team::getName(TeamColor team) { // const
  switch (team) {
    case AutomaticTeam: { return "Automatic";    }
    case RogueTeam:     { return "Rogue";        }
    case RedTeam:       { return "Red Team";     }
    case GreenTeam:     { return "Green Team";   }
    case BlueTeam:      { return "Blue Team";    }
    case PurpleTeam:    { return "Purple Team";  }
    case ObserverTeam:  { return "Observer";     }
    case RabbitTeam:    { return "Rabbit";       }
    case HunterTeam:    { return "Hunter";       }
    case NoTeam:        { return "No Team??";    }
    default:            { return "Invalid team"; }
  }
}


const char* Team::getShortName(TeamColor team) {
  switch (team) {
    case RogueTeam:    { return "rogue";    }
    case RedTeam:      { return "red";      }
    case GreenTeam:    { return "green";    }
    case BlueTeam:     { return "blue";     }
    case PurpleTeam:   { return "purple";   }
    case ObserverTeam: { return "observer"; }
    case RabbitTeam:   { return "rabbit";   }
    case HunterTeam:   { return "hunter";   }
    default:           { return "none";     }
  }
}


TeamColor Team::getTeam(const std::string name) { // const
  if (name == Team::getName(AutomaticTeam)) {
    return AutomaticTeam;
  }
  for (int i = 0; i < NumTeams; i++) {
    if (name == Team::getName((TeamColor)i)) {
      return (TeamColor)i;
    }
  }
  return NoTeam;
}


const fvec4& Team::getTankColor(TeamColor team) { // const
  if (int(team) < 0) {
    return tankColor[0];
  }
  return tankColor[int(team)];
}


const fvec4& Team::getRadarColor(TeamColor team) { // const
  if (int(team) < 0) {
    return radarColor[0];
  }
  return radarColor[int(team)];
}


bool Team::isColorTeam(TeamColor team) { // const
  return team >= RedTeam  && team <= PurpleTeam;
}


void Team::setColors(TeamColor team, const fvec4& tank, const fvec4& radar) {
  const int teamIndex = int(team);
  // ignore bogus team color
  if (teamIndex < 0) {
    return;
  }

  // leave alpha at 1.0f
  tankColor[teamIndex].rgb()  = tank.rgb();
  radarColor[teamIndex].rgb() = radar.rgb();
}


// are the two teams foes with the current game style?
bool Team::areFoes(TeamColor team1, TeamColor team2, GameType style) {
  if (style == OpenFFA) {
    return true;
  }
  return (team1 != team2) || (team1 == RogueTeam) || (team2 == RogueTeam);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
