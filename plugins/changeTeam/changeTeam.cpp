// changeTeam.cpp : Defines the entry point for the DLL application.
//
// Licensed under BSD so plug-ins can re-license as needed
/*
Copyright (c) 1993-2017 Tim Riker
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

*/

#include "bzfsAPI.h"
#include "plugin_utils.h"

class changeTeam : public bz_Plugin, bz_CustomSlashCommandHandler
{
public:
  virtual const char* Name () {return "Change Team";}
  virtual void Init ( const char* /*config*/ )
  {
    bz_debugMessage(4,"changeTeam plugin loaded");
    bz_registerCustomSlashCommand("team", this);
  }

  virtual void Cleanup ( void )
  {
    bz_removeCustomSlashCommand("team");
  }

  virtual bool SlashCommand ( int playerID, bz_ApiString /*command*/, bz_ApiString /*message*/, bz_APIStringList* params) {
    /*bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
    if (!player)
      return true;*/

    if (bz_getPlayerTeam(playerID) == eNoTeam)
      return true;

    bz_eTeamType newTeam = eNoTeam;

    std::string newTeamString = params->get(0);
    if (newTeamString ==  "observer") {
      newTeam = eObservers;
    }
    else if (bz_getGameType() == eRabbitGame) {
      if (newTeamString ==  "hunter") newTeam = eHunterTeam;
      else if (newTeamString ==  "rabbit") newTeam = eRabbitTeam;
    }
    else {
      if (newTeamString == "rogue") newTeam = eRogueTeam;
      else if (newTeamString ==  "red") newTeam = eRedTeam;
      else if (newTeamString ==  "green") newTeam = eGreenTeam;
      else if (newTeamString ==  "blue") newTeam = eBlueTeam;
      else if (newTeamString ==  "purple") newTeam = ePurpleTeam;
    }

    if (newTeam != eNoTeam)
      bz_changePlayerTeam(playerID, newTeam);

    return true;
  }
};

BZ_PLUGIN(changeTeam)


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
