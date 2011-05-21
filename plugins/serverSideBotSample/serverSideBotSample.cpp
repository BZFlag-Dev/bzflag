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

// serverSideBotSample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <math.h>
#include <list>

BZ_GET_PLUGIN_VERSION

#define deg2Rad 0.017453292519943295769236907684886f
#define rad2Deg 57.295779513082320876798154814105f

class Waypoint {
  public:
    Waypoint(bz_ServerSidePlayerHandler& b, float* t = NULL) : bot(b) {
      if (t) {
        memcpy(target, t, sizeof(float) * 3);
      }
      else {
        memset(target, 0, sizeof(float) * 3);
      }
    }

    Waypoint(const Waypoint& v) : bot(v.bot) {
      memcpy(target, v.target, sizeof(float) * 3);
    }

    bool process(void) {
      // get the current vec to the target
      float vec[3];
      bot.getPosition(vec);

      for (int i = 0; i < 3; i++) {
        vec[i] = target[i] - vec[i];
      }

      // see if we are there
      if (vec[0]*vec[0] + vec[1]*vec[1] < 1) {
        return true;
      }

      float angle = atan2(vec[1], vec[0]) * rad2Deg;
      float angDelta = angle - bot.getFacing();

      float input[2] = {0, 0};
      if (fabs(angDelta) > 1.0f) {
        input[0] = angDelta;
      }

      if (fabs(angDelta) > 25.0f) {
        input[1] = 1.0f;
      }

      bot.setMovement(input[1], input[0]);

      return false;
    }

    float target[3];
  private:
    bz_ServerSidePlayerHandler& bot;
};

class SimpleBotHandler : public bz_ServerSidePlayerHandler {
  public:
    SimpleBotHandler();
    virtual void added(int playerIndex);
    virtual void removed(void);

    virtual bool think(void);

    virtual void playerSpawned(int player, float pos[3], float rot);

  protected:
    std::list<Waypoint> waypoints;
};

class EventHandler : public bz_EventHandler {
  public:
    virtual void process(bz_EventData* eventData);
};

std::vector<SimpleBotHandler*> bots;
EventHandler events;
int numBots = 1;

BZF_PLUGIN_CALL int bz_Load(const char* commandLine) {
  bz_debugMessage(4, "serverSideBotSample plugin loaded");
  bz_debugMessage(0, "******* WARNING. THE SERVER SIDE BOT PLUGIN IS UNSTABLE******");
  bz_debugMessage(0, "******* IT CAN AND WILL CRASH YOUR SYSTEM ******");
  bz_debugMessage(0, "******* THE CODE IS UNDER DEVELOPMENT ******");
  bz_debugMessage(0, "******* DO NOT USE IT UNLESS YOU ARE SURE YOU KNOW WHAT YOU ARE DOING ******");

  if (commandLine) {
    numBots = atoi(commandLine);
  }
  if (numBots < 1) {
    numBots = 1;
  }

  bz_registerEvent(bz_eTickEvent, &events);
  bz_registerEvent(bz_eWorldFinalized, &events);

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void) {
  bz_debugMessagef(2, "removing %d simple bot", bots.size());
  for (size_t i = 0; i < bots.size(); i++) {
    bz_removeServerSidePlayer(bots[i]->getPlayerID(), bots[i]);
  }

  bots.clear();

  bz_removeEvent(bz_eTickEvent, &events);
  bz_removeEvent(bz_eWorldFinalized, &events);

  bz_debugMessage(4, "serverSideBotSample plugin unloaded");
  return 0;
}

SimpleBotHandler::SimpleBotHandler() {
}

void SimpleBotHandler::added(int playerIndex) {
  bz_debugMessage(3, "SimpleBotHandler::added");
  std::string name = "dante_";
  name += bz_format("%d", playerID);
  setPlayerData(name.c_str(), NULL, "bot sample", eAutomaticTeam);
  joinGame();

  float vals[3] = {0};
  waypoints.push_back(Waypoint(*this, vals));
}

void SimpleBotHandler::removed(void) {
  bz_debugMessage(3, "SimpleBotHandler::removed");
}

bool SimpleBotHandler::think(void) {
  // see if we have a target, if so drive to it.
  if (waypoints.size()) {
    if (waypoints.begin()->process()) {
      waypoints.erase(waypoints.begin());
    }
  }

  if (!waypoints.size() && bz_anyPlayers()) {
    // find a random player that is spawned.
    bz_APIIntList* playerList = bz_getPlayerIndexList();
    if (playerList) {
      if (playerList->size()) { // if it's just one, it's just me
        int dude = rand()&playerList->size();
        dude = playerList->get(dude);

        if (dude != getPlayerID()) { // chasing myself is stupid
          bz_BasePlayerRecord* player = bz_getPlayerByIndex(dude);
          if (player && player->spawned) { // only chase the live ones
            std::string message;
            message = bz_format("I see you %s!", player->callsign.c_str());
            bz_sendTextMessage(playerID, BZ_ALLUSERS, message.c_str());

            // add the point as a waypoint
            waypoints.push_back(Waypoint(*this, player->currentState.pos));
          }

          bz_freePlayerRecord(player);
        }
      }

      bz_deleteIntList(playerList);
    }
  }
  return false;
}

void SimpleBotHandler::playerSpawned(int player, float pos[3], float rot) {
  bz_ServerSidePlayerHandler::playerSpawned(player, pos, rot);

  bz_debugMessage(3, "SimpleBotHandler::process bz_ePlayerSpawnEvent");

  if (player == playerID) {
    return;
  }

  bz_BasePlayerRecord* playerInfo = bz_getPlayerByIndex(player);
  std::string message;
  message = bz_format("well, look who dropped in...%s", playerInfo->callsign.c_str());
  bz_sendTextMessage(playerID, BZ_ALLUSERS, message.c_str());

  // add the point as a waypoint
  waypoints.push_back(Waypoint(*this, pos));

  bz_freePlayerRecord(playerInfo);
}

void EventHandler::process(bz_EventData* eventData) {
  if (eventData->eventType == bz_eTickEvent) {
    for (size_t i = 0; i < bots.size(); i++) {
      bots[i]->update();
    }
  }
  else {
    bz_debugMessagef(2, "adding %d simple bot(s), may the gods have mercy on your soul", numBots);
    for (int i = 0; i < numBots;  i++) {
      SimpleBotHandler* bot = new SimpleBotHandler;
      bz_addServerSidePlayer(bot);
      bots.push_back(bot);
    }

    bz_setMaxWaitTime(0.01f, "StupidSampleBot");
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
