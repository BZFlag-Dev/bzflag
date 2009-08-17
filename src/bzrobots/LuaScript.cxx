/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

// interface header
#include "LuaScript.h"

// system headers
#include <string>
#include <stdlib.h>

// common headers
#include "TimeKeeper.h"

// local headers
#include "LuaHeader.h"
#include "AdvancedRobot.h"


using namespace BZRobots;


static const char* ThisLabel = "this";


static bool PushCallOuts(lua_State* L);

static int Execute(lua_State* L);
static int ClearEvents(lua_State* L);

static int DoNothing(lua_State* L);
static int Fire(lua_State* L);
static int Resume(lua_State* L);
static int Scan(lua_State* L);
static int Stop(lua_State* L);
static int Ahead(lua_State* L);
static int Back(lua_State* L);
static int TurnLeft(lua_State* L);
static int TurnRight(lua_State* L);

static int SetAhead(lua_State* L);
static int SetFire(lua_State* L);
static int SetMaxTurnRate(lua_State* L);
static int SetMaxVelocity(lua_State* L);
static int SetResume(lua_State* L);
static int SetStop(lua_State* L);
static int SetTurnLeft(lua_State* L);

static int GetTime(lua_State* L);
static int GetOthers(lua_State* L);
static int GetBattleFieldLength(lua_State* L);
static int GetBattleFieldWidth(lua_State* L);
static int GetGunCoolingRate(lua_State* L);
static int GetGunHeat(lua_State* L);
static int GetHeading(lua_State* L);
static int GetHeight(lua_State* L);
static int GetLength(lua_State* L);
static int GetName(lua_State* L);
static int GetWidth(lua_State* L);
static int GetVelocity(lua_State* L);
static int GetX(lua_State* L);
static int GetY(lua_State* L);
static int GetZ(lua_State* L);
static int GetDistanceRemaining(lua_State* L);
static int GetTurnRemaining(lua_State* L);

static int GetEventID(lua_State* L);
static int GetEventTime(lua_State* L);
static int GetEventPriority(lua_State* L);

static int Sleep(lua_State* L);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
static int ReadStdin(lua_State* L);
#endif


//============================================================================//
//============================================================================//

class LuaRobot : public AdvancedRobot {
  public:
    LuaRobot(const std::string& filename);
    ~LuaRobot();

    void run();

    void onBattleEnded(const BattleEndedEvent&);
    void onBulletFired(const BulletFiredEvent&);
    void onBulletHit(const BulletHitEvent&);
    void onBulletHitBullet(const BulletHitBulletEvent&);
    void onBulletMissed(const BulletMissedEvent&);
    void onDeath(const DeathEvent&);
    void onHitByBullet(const HitByBulletEvent&);
    void onHitRobot(const HitRobotEvent&);
    void onHitWall(const HitWallEvent&);
    void onRobotDeath(const RobotDeathEvent&);
    void onScannedRobot(const ScannedRobotEvent&);
    void onSpawn(const SpawnEvent&);
    void onStatus(const StatusEvent&);
    void onWin(const WinEvent&);

  public:
    const Event& getLastEvent() const { return lastEvent; }

  private:
    void setLastEvent(const Event& e) { lastEvent = e; }
    bool PushCallIn(const char* funcName, int inArgs);
    bool RunCallIn(int inArgs, int outArgs);

  private:
    lua_State* L;
    Event lastEvent;
};


//============================================================================//
//============================================================================//
//
//  LuaScript
//

LuaScript::LuaScript()
{
  _running = true;
  error = "";
}


LuaScript::~LuaScript()
{
}


bool LuaScript::load(std::string filename)
{
  scriptFile = filename;
  _loaded = true;
  return true;
}


Robot* LuaScript::create()
{
  return new LuaRobot(scriptFile);
}


void LuaScript::destroy(Robot* instance)
{
  delete instance;
}


//============================================================================//
//============================================================================//
//
//  LuaRobot
//

LuaRobot::LuaRobot(const std::string& filename)
{
  L = luaL_newstate();

  luaL_openlibs(L);

  lua_pushlightuserdata(L, (void*)this);
  lua_setfield(L, LUA_REGISTRYINDEX, ThisLabel);

  if (!PushCallOuts(L)) {
    printf("LuaRobot: failed to push call-outs\n");
    lua_close(L);
    L = NULL;
    return;
  }

  // compile the code
  if (luaL_loadfile(L, filename.c_str()) != 0) {
    printf("LuaRobot: failed to load '%s'\n", filename.c_str());
    lua_close(L);
    L = NULL;
    return;
  }

  // execute the code
  if (lua_pcall(L, 0, 0, 0) != 0) {
    printf("LuaRobot: failed to execute, %s\n", lua_tostring(L, -1));
    lua_close(L);
    L = NULL;
    return;
  }
}


LuaRobot::~LuaRobot()
{
  if (L != NULL) {
    lua_close(L);
  }
}


//============================================================================//
//============================================================================//
//
//  Call-ins
//

bool LuaRobot::PushCallIn(const char* funcName, int inArgs)
{
  if (L == NULL) {
    return false;
  }
  if (!lua_checkstack(L, inArgs + 2)) {
    return false;
  }
  lua_getglobal(L, funcName);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }
  return true;
}


bool LuaRobot::RunCallIn(int inArgs, int outArgs)
{
  if (lua_pcall(L, inArgs, outArgs, 0) != 0) {
    printf("LuaRobot: error, %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }
  return true;
}


static void PushBullet(lua_State* L, const Bullet* bullet)
{
  lua_createtable(L, 0, 8);
  if (bullet == NULL) {
    return;
  }
  lua_pushstdstring(L, bullet->getName());   lua_setfield(L, -2, "owner");
  lua_pushstdstring(L, bullet->getVictim()); lua_setfield(L, -2, "victim");
  lua_pushdouble(L, bullet->getVelocity());  lua_setfield(L, -2, "velocity");
  lua_pushdouble(L, bullet->getHeading());   lua_setfield(L, -2, "heading");
  lua_pushdouble(L, bullet->getX());         lua_setfield(L, -2, "x");
  lua_pushdouble(L, bullet->getY());         lua_setfield(L, -2, "y");
  lua_pushdouble(L, bullet->getZ());         lua_setfield(L, -2, "z");
  lua_pushboolean(L, bullet->isActive());    lua_setfield(L, -2, "active");
}


//============================================================================//

void LuaRobot::run()
{
  if (!PushCallIn("Run", 0)) {
    printf("LuaRobot: missing Run() function\n");
    return;
  }
  RunCallIn(0, 0);
}


void LuaRobot::onBattleEnded(const BattleEndedEvent& event)
{
  if (!PushCallIn("BattleEnded", 1)) {
    return;
  }
  setLastEvent(event);

  lua_pushboolean(L, event.isAborted());

  RunCallIn(1, 0);
}


void LuaRobot::onBulletFired(const BulletFiredEvent& event)
{
  if (!PushCallIn("BulletFired", 1)) {
    return;
  }
  setLastEvent(event);

  PushBullet(L, event.getBullet());

  RunCallIn(1, 0);
}


void LuaRobot::onBulletHit(const BulletHitEvent& event)
{
  if (!PushCallIn("BulletHit", 2)) {
    return;
  }
  setLastEvent(event);

  lua_pushstdstring(L, event.getName());
  PushBullet(L, event.getBullet());

  RunCallIn(2, 0);
}


void LuaRobot::onBulletHitBullet(const BulletHitBulletEvent& event)
{
  if (!PushCallIn("BulletHitBullet", 2)) {
    return;
  }
  setLastEvent(event);

  PushBullet(L, event.getBullet());
  PushBullet(L, event.getHitBullet());

  RunCallIn(2, 0);
}


void LuaRobot::onBulletMissed(const BulletMissedEvent& event)
{
  if (!PushCallIn("BulletMissed", 1)) {
    return;
  }
  setLastEvent(event);

  PushBullet(L, event.getBullet());

  RunCallIn(1, 0);
}


void LuaRobot::onDeath(const DeathEvent& event)
{
  if (!PushCallIn("Death", 0)) {
    return;
  }
  setLastEvent(event);

  RunCallIn(0, 0);
}


void LuaRobot::onHitByBullet(const HitByBulletEvent& event)
{
  if (!PushCallIn("HitByBullet", 2)) {
    return;
  }
  setLastEvent(event);

  lua_pushdouble(L, event.getBearing());
  PushBullet(L, event.getBullet());

  RunCallIn(2, 0);
}


void LuaRobot::onHitRobot(const HitRobotEvent& event)
{
  if (!PushCallIn("HitRobot", 4)) {
    return;
  }
  setLastEvent(event);

  lua_pushstdstring(L, event.getName());
  lua_pushdouble(L, event.getEnergy());
  lua_pushdouble(L, event.getBearing());
  lua_pushboolean(L, event.isMyFault());

  RunCallIn(4, 0);
}


void LuaRobot::onHitWall(const HitWallEvent& event)
{
  if (!PushCallIn("HitWall", 1)) {
    return;
  }
  setLastEvent(event);

  lua_pushdouble(L, event.getBearing());

  RunCallIn(1, 0);
}


void LuaRobot::onRobotDeath(const RobotDeathEvent& event)
{
  if (!PushCallIn("RobotDeath", 1)) {
    return;
  }
  setLastEvent(event);

  lua_pushstdstring(L, event.getName());

  RunCallIn(1, 0);
}


void LuaRobot::onScannedRobot(const ScannedRobotEvent& event)
{
  if (!PushCallIn("ScannedRobot", 1)) {
    return;
  }
  setLastEvent(event);

  lua_createtable(L, 0, 8);
  lua_pushstdstring(L, event.getName());  lua_setfield(L, -2, "name");
  lua_pushdouble(L, event.getBearing());  lua_setfield(L, -2, "bearing");
  lua_pushdouble(L, event.getDistance()); lua_setfield(L, -2, "distance");
  lua_pushdouble(L, event.getX());        lua_setfield(L, -2, "x");
  lua_pushdouble(L, event.getY());        lua_setfield(L, -2, "y");
  lua_pushdouble(L, event.getZ());        lua_setfield(L, -2, "z");
  lua_pushdouble(L, event.getHeading());  lua_setfield(L, -2, "heading");
  lua_pushdouble(L, event.getVelocity()); lua_setfield(L, -2, "velocity");

  RunCallIn(1, 0);
}


void LuaRobot::onSpawn(const SpawnEvent& event)
{
  if (!PushCallIn("Spawn", 0)) {
    return;
  }
  setLastEvent(event);

  RunCallIn(0, 0);
}


void LuaRobot::onStatus(const StatusEvent& event)
{
  if (!PushCallIn("Status", 2)) {
    return;
  }
  setLastEvent(event);

  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());

  RunCallIn(2, 0);
}


void LuaRobot::onWin(const WinEvent& event)
{
  if (!PushCallIn("Win", 0)) {
    return;
  }
  setLastEvent(event);

  RunCallIn(0, 0);
}


//============================================================================//
//============================================================================//
//
//  Call-outs
//

static bool PushCallOuts(lua_State* L)
{
  lua_newtable(L);

  PUSH_LUA_CFUNC(L, Execute);
  PUSH_LUA_CFUNC(L, ClearEvents);

  PUSH_LUA_CFUNC(L, DoNothing);
  PUSH_LUA_CFUNC(L, Fire);
  PUSH_LUA_CFUNC(L, Resume);
  PUSH_LUA_CFUNC(L, Scan);
  PUSH_LUA_CFUNC(L, Stop);
  PUSH_LUA_CFUNC(L, Ahead);
  PUSH_LUA_CFUNC(L, Back);
  PUSH_LUA_CFUNC(L, TurnLeft);
  PUSH_LUA_CFUNC(L, TurnRight);

  PUSH_LUA_CFUNC(L, SetAhead);
  PUSH_LUA_CFUNC(L, SetFire);
  PUSH_LUA_CFUNC(L, SetMaxTurnRate);
  PUSH_LUA_CFUNC(L, SetMaxVelocity);
  PUSH_LUA_CFUNC(L, SetResume);
  PUSH_LUA_CFUNC(L, SetStop);
  PUSH_LUA_CFUNC(L, SetTurnLeft);

  PUSH_LUA_CFUNC(L, GetTime);
  PUSH_LUA_CFUNC(L, GetOthers);
  PUSH_LUA_CFUNC(L, GetBattleFieldLength);
  PUSH_LUA_CFUNC(L, GetBattleFieldWidth);
  PUSH_LUA_CFUNC(L, GetGunCoolingRate);
  PUSH_LUA_CFUNC(L, GetGunHeat);
  PUSH_LUA_CFUNC(L, GetHeading);
  PUSH_LUA_CFUNC(L, GetHeight);
  PUSH_LUA_CFUNC(L, GetLength);
  PUSH_LUA_CFUNC(L, GetName);
  PUSH_LUA_CFUNC(L, GetWidth);
  PUSH_LUA_CFUNC(L, GetVelocity);
  PUSH_LUA_CFUNC(L, GetX);
  PUSH_LUA_CFUNC(L, GetY);
  PUSH_LUA_CFUNC(L, GetZ);
  PUSH_LUA_CFUNC(L, GetDistanceRemaining);
  PUSH_LUA_CFUNC(L, GetTurnRemaining);

  PUSH_LUA_CFUNC(L, GetEventID);
  PUSH_LUA_CFUNC(L, GetEventTime);
  PUSH_LUA_CFUNC(L, GetEventPriority);

  PUSH_LUA_CFUNC(L, Sleep);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  PUSH_LUA_CFUNC(L, ReadStdin);
#endif

  lua_setglobal(L, "bz");

  return true;
}


//============================================================================//

static inline LuaRobot* GetRobot(lua_State* L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, ThisLabel);
  if (!lua_isuserdata(L, -1)) {
    luaL_error(L, "Internal error -- missing '%s'", ThisLabel);
  }
  LuaRobot* robot = (LuaRobot*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  return robot;
}


//============================================================================//

static int Execute(lua_State* L) {
  GetRobot(L)->execute();
  return 0;
}

static int ClearEvents(lua_State* L) {
  GetRobot(L)->clearAllEvents();
  return 0;
}

static int DoNothing(lua_State* L) {
  GetRobot(L)->doNothing();
  return 0;
}

static int Fire(lua_State* L) {
  GetRobot(L)->fire();
  return 0;
}

static int Resume(lua_State* L) {
  GetRobot(L)->resume();
  return 0;
}

static int Scan(lua_State* L) {
  GetRobot(L)->scan();
  return 0;
}

static int Stop(lua_State* L) {
  GetRobot(L)->stop();
  return 0;
}

static int Ahead(lua_State* L) {
  GetRobot(L)->ahead(luaL_checkdouble(L, 1));
  return 0;
}

static int Back(lua_State* L) {
  GetRobot(L)->back(luaL_checkdouble(L, 1));
  return 0;
}

static int TurnLeft(lua_State* L) {
  GetRobot(L)->turnLeft(luaL_checkdouble(L, 1));
  return 0;
}

static int TurnRight(lua_State* L) {
  GetRobot(L)->turnRight(luaL_checkdouble(L, 1));
  return 0;
}


//============================================================================//

static int SetAhead(lua_State* L) {
  GetRobot(L)->setAhead(luaL_checkdouble(L, 1));
  return 0;
}

static int SetFire(lua_State* L) {
  GetRobot(L)->setFire();
  return 0;
}

static int SetMaxTurnRate(lua_State* L) {
  GetRobot(L)->setMaxTurnRate(luaL_checkdouble(L, 1));
  return 0;
}

static int SetMaxVelocity(lua_State* L) {
  GetRobot(L)->setMaxVelocity(luaL_checkdouble(L, 1));
  return 0;
}

static int SetResume(lua_State* L) {
  GetRobot(L)->setResume();
  return 0;
}

static int SetStop(lua_State* L) {
  GetRobot(L)->setStop(lua_isboolean(L, 1) && lua_toboolean(L, 1));
  return 0;
}

static int SetTurnLeft(lua_State* L) {
  GetRobot(L)->setTurnLeft(luaL_checkdouble(L, 1));
  return 0;
}


//============================================================================//

static int GetTime(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getTime());
  return 1;
}

static int GetOthers(lua_State* L) {
  lua_pushinteger(L, GetRobot(L)->getOthers());
  return 1;
}

static int GetBattleFieldLength(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getBattleFieldLength());
  return 1;
}

static int GetBattleFieldWidth(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getBattleFieldWidth());
  return 1;
}

static int GetGunCoolingRate(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getGunCoolingRate());
  return 1;
}

static int GetGunHeat(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getGunHeat());
  return 1;
}

static int GetHeading(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getHeading());
  return 1;
}

static int GetHeight(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getHeight());
  return 1;
}

static int GetLength(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getLength());
  return 1;
}

static int GetName(lua_State* L) {
  lua_pushstring(L, GetRobot(L)->getName().c_str());
  return 1;
}

static int GetWidth(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getWidth());
  return 1;
}

static int GetVelocity(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getVelocity());
  return 1;
}

static int GetX(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getX());
  return 1;
}

static int GetY(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getY());
  return 1;
}

static int GetZ(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getZ());
  return 1;
}

static int GetDistanceRemaining(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getDistanceRemaining());
  return 1;
}

static int GetTurnRemaining(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getTurnRemaining());
  return 1;
}

static int GetEventID(lua_State* L) {
  lua_pushinteger(L, GetRobot(L)->getLastEvent().getEventID());
  return 1;
}

static int GetEventTime(lua_State* L) {
  lua_pushdouble(L, GetRobot(L)->getLastEvent().getTime());
  return 1;
}

static int GetEventPriority(lua_State* L) {
  lua_pushinteger(L, GetRobot(L)->getLastEvent().getPriority());
  return 1;
}

//============================================================================//

static int Sleep(lua_State* L)
{
  TimeKeeper::sleep(luaL_checkdouble(L, 1));
  return 0;
}


// whacky bit of dev'ing fun
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  #include <unistd.h>
  #include <fcntl.h>
  static int ReadStdin(lua_State* L)
  {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    char buf[4096];
    const int r = read(STDIN_FILENO, buf, sizeof(buf));
    if (r <= 0) {
      return 0;
    }
    lua_pushlstring(L, buf, r);
    fcntl(STDIN_FILENO, F_SETFL, 0);
    return 1;
  }
#endif


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
