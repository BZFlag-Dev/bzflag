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
#include "bzfsAPIServerSidePlayers.h"
#include "plugin_utils.h"
#include <map>
#include <vector>

class TeamFillPlayer : public bz_ServerSidePlayerHandler
{
public:
    TeamFillPlayer(bz_eTeamType team); // c'tor
    void added(int player);
    void playerRejected(bz_eRejectCodes code, const char* reason);

private:
    bz_eTeamType team;
};

class changeTeam : public bz_Plugin, bz_CustomSlashCommandHandler
{
private:
    std::map <bz_eTeamType, std::vector<TeamFillPlayer*>> teamFillPlayers;
public:
    virtual const char* Name ()
    {
        return "Change Team";
    }
    virtual void Init ( const char* /*config*/ )
    {
        bz_debugMessage(4,"changeTeam plugin loaded");
        bz_registerCustomSlashCommand("team", this);
        bz_registerCustomSlashCommand("fill", this);
    }

    virtual void Cleanup ( void )
    {
        bz_removeCustomSlashCommand("team");
        bz_removeCustomSlashCommand("fill");
    }

    virtual bool SlashCommand ( int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList* params)
    {
        bz_eTeamType team = eNoTeam;

        std::string teamString = "";
        if (params->size() > 0)
        {
            teamString = params->get(0);
            if (teamString == "observer")
                team = eObservers;
            else if (bz_getGameType() == eRabbitGame)
            {
                if (teamString == "hunter") team = eHunterTeam;
                else if (teamString == "rabbit") team = eRabbitTeam;
            }
            else
            {
                if (teamString == "rogue") team = eRogueTeam;
                else if (teamString == "red") team = eRedTeam;
                else if (teamString == "green") team = eGreenTeam;
                else if (teamString == "blue") team = eBlueTeam;
                else if (teamString == "purple") team = ePurpleTeam;
            }
        }

        if (command == "team")
        {
            if (params->size() == 0)
            {
                std::string yourTeam = "";
                switch(bz_getPlayerTeam(playerID))
                {
                case eRogueTeam:
                    yourTeam = "rogue";
                    break;
                case eRedTeam:
                    yourTeam = "red";
                    break;
                case eGreenTeam:
                    yourTeam = "green";
                    break;
                case eBlueTeam:
                    yourTeam = "blue";
                    break;
                case ePurpleTeam:
                    yourTeam = "purple";
                    break;
                case eRabbitTeam:
                    yourTeam = "rabbit";
                    break;
                case eHunterTeam:
                    yourTeam = "hunter";
                    break;
                case eObservers:
                    yourTeam = "observer";
                    break;
                default:
                    yourTeam = "";
                    break;
                }

                bz_sendTextMessagef(BZ_SERVER, playerID, "Your team is %s", yourTeam.c_str());
            }
            else
            {
                if (bz_getPlayerTeam(playerID) == eNoTeam)
                    return true;

                if (team != eNoTeam)
                    bz_changePlayerTeam(playerID, team);
            }
        }
        else if (command == "fill")
        {
            if (team != eNoTeam)
            {
                unsigned int limit = bz_getTeamPlayerLimit(team);
                unsigned int count = 0;
                bz_APIIntList *playerList = bz_getPlayerIndexList();
                for (unsigned int i = 0; i < playerList->size(); i++)
                {
                    if (bz_getPlayerTeam(i) == team)
                        count++;
                }
                bz_deleteIntList (playerList);

                bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Joining %d players to %s team", (limit - count), teamString.c_str());

                for (unsigned int i = 0; i < limit - count; i++)
                {
                    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Creating player %d", i);
                    auto player = new TeamFillPlayer(team);
                    bz_addServerSidePlayer(player);
                    teamFillPlayers[team].push_back(player);
                }
            }
        }

        return true;
    }
};

TeamFillPlayer::TeamFillPlayer(bz_eTeamType _team)
{
    playerID = -1;
    team = _team;
}

void TeamFillPlayer::added(int player)
{
    if (player == playerID)
    {
        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Added player %d", player);
        setPlayerData(bz_format("fillplayer%d", playerID), "", "I do my best to fill things", team);
        joinGame();
    }
}

void TeamFillPlayer::playerRejected(bz_eRejectCodes /* code */, const char* reason)
{
    bz_debugMessagef(1, "Player rejected (reason: %s)", reason);
}

BZ_PLUGIN(changeTeam)


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
