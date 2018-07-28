/* bzflag
* Copyright (c) 1993-2018 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once
#include "bzfsAPI.h"

typedef struct
{
    int index;
    char type[2];
    int status;
    int endurance;
    int owner;
    float position[3];
    float launchPosition[3];
    float landingPosition[3];
    float flightTime;
    float flightEnd;
    float initialVelocity;
} bz_FlagUpdateRecord;

typedef struct
{
    float rank;
    int wins;
    int losses;
    int tks;
} bz_ScoreRecord;

typedef struct
{
    int id;
    int size;
    int wins;
    int losses;
} bz_TeamInfoRecord;

typedef enum
{
    eRejectBadRequest,
    eRejectBadTeam,
    eRejectBadType,
    eRejectUNUSED,
    eRejectTeamFull,
    eRejectServerFull,
    eRejectBadCallsign,
    eRejectRepeatCallsign,
    eRejectRejoinWaitTime,
    eRejectIPBanned,
    eRejectHostBanned,
    eRejectIDBanned
} bz_eRejectCodes;

enum class bz_eTankStatus
{
    Dead,           // dead, explosion over
    Exploding,      // dead and exploding
    OnGround,       // playing on ground
    InBuilding,     // playing in building
    OnBuilding,     // playing on building
    InAir           // playing in air
};

typedef struct
{
    int player;
    int handicap;
} bz_HandicapUpdateRecord;

class BZF_API bz_ServerSidePlayerHandler
{
public:
    bz_ServerSidePlayerHandler();
    virtual ~bz_ServerSidePlayerHandler();

    int getPlayerID(void)
    {
        return playerID;
    }

    void update(void);

    // you must call setPlayerData when this is called.
    virtual void added(int player) = 0; // it is required that the bot provide this method

    // lower level events for various things that happen in the game
    virtual void removed(void) {}

    virtual void playerAdded(int player);
    virtual void playerRemoved(int player);

    virtual void playerSpawned(int player, const float pos[3], float rot);

    virtual void textMessage(int dest, int source, const char *text);
    virtual void playerKilled(int victimIndex, int killerIndex, bz_ePlayerDeathReason reason, int shotIndex,
                              const char *flagType, int phydrv);
    virtual void scoreLimitReached(int player, bz_eTeamType team);
    virtual void flagCaptured(int player, bz_eTeamType team);

    virtual void playerStateUpdate(int player, bz_PlayerUpdateState *playerState, double timestamp);

    // implement when server side scoring is in
    // virtual void playerScoreUpdate(int player, float rank, int wins, int losses, int TKs); // implement when server side scoring is in

    virtual void shotFired(int player, unsigned short shotID);
    virtual void shotEnded(int player, unsigned short shotID, bool expired);
    virtual void playerTeleported(int player, bz_PlayerUpdateState *currentState, bz_PlayerUpdateState *lastState);

    // higher level functions for events that happen to the bot
    typedef enum
    {
        eWorldDeath,
        eServerDeath,
        eCaptureDeath,
        eOtherDeath
    } SmiteReason;

    void rejected(bz_eRejectCodes, const char* /*reason*/) {}; // the bot was rejectd for some reason
    virtual void spawned(void); // the bot has spawned
    virtual void died(int killer); // the bot has died from gameplay
    virtual void smote(SmiteReason reason = eOtherDeath); // the bot has died from some other manner

    // virtual void collide ( bz_APISolidWorldObject_V1* /*object*/, float* /*pos*/ ) {} // the bot ran into an object

    // give the bot time to do it's processing
    virtual bool think(void); // return true to kill and delete the bot;

    void setPlayerID(int id)
    {
        playerID = id;
    }

    // actions to make
    void setPlayerData(const char *callsign, const char *token, const char *clientVersion, bz_eTeamType team);

    void joinGame(bool spawn = true);

    void respawn(void);
    void getCurrentState(bz_PlayerUpdateState *state);

    void sendServerCommand(const char* text);
    void sendChatMessage(const char* text, int targetPlayer = BZ_ALLUSERS, bz_eMessageType type = eChatMessage);
    void sendTeamChatMessage(const char *text, bz_eTeamType targetTeam, bz_eMessageType type = eChatMessage);

    void dropFlag(void);
    void setMovement(float speed, float turn);
    bool fireShot(void);
    void jump(void);

    // state info
    bool isAlive();
    bool canJump();
    bool canShoot();
    bool canMove();
    bool falling();

    void getPosition(float *p);
    void getVelocity(float *v);
    float getFacing();

    float getMaxLinSpeed();
    float getMaxRotSpeed();

    // state actions
    void setAutoSpawn(bool s = true)
    {
        autoSpawn = s;
    }

    // called by the event system to handle basic player funcitons
    void checkForSpawn(int player, const float pos[3], float rot);

    int playerID;

protected:
    // default autopilot based logic
    virtual  void    doAutoPilot(float &rotation, float &speed);
    void             dropHardFlags();
    virtual bool     avoidBullet(float &rotation, float &speed);
    bool             stuckOnWall(float &rotation, float &speed);
    virtual bool     chasePlayer(float &rotation, float &speed);
    virtual bool     lookForFlag(float &rotation, float &speed);
    virtual bool     navigate(float &rotation, float &speed);
    bool             avoidDeathFall(float &rotation, float &speed);
    virtual bool     fireAtTank();

    virtual bool     isFlagUseful(const char* type);

private:
    bool autoSpawn;
    class Impl;
    Impl* pImpl = nullptr;

    // implementation of physics pulled from the client
    void processUpdate(float dt);
    void doUpdateMotion(float dt);
    void doMomentum(float dt, float& speed, float& angVel);
    void doFriction(float dt, const float *oldVelocity, float *newVelocity);
    void doSlideMotion(float dt, float slideTime, float newAngVel, float* newVelocity);
    void doJump();

    void setVelocity(float newVel[3]);



    int flaps;
};

// *** NOTE *** support for server side players in incomplete.
//  there WILL be crashes if you add one.
// this message will be removed when the code is complete.
BZF_API int bz_addServerSidePlayer(bz_ServerSidePlayerHandler *handler);
BZF_API bool bz_removeServerSidePlayer(int playerID,
                                       bz_ServerSidePlayerHandler *handler); // you have to pass in the handler to ensure you "own" the player


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
