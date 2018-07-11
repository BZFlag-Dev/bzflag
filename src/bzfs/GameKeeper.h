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

#ifndef __GAMEKEEPER_H__
#define __GAMEKEEPER_H__

// bzflag global header
#include "common.h"

// system headers
#include <vector>
#include <string>

// common interface headers
#include "PlayerInfo.h"
#include "PlayerState.h"
#include "TimeKeeper.h"

// implementation-specific bzfs-specific headers
#include "CmdLineOptions.h"
#include "FlagHistory.h"
#include "Permissions.h"
#include "LagInfo.h"
#include "Score.h"
#include "RecordReplay.h"
#include "NetHandler.h"
#include "Authentication.h"
#include "messages.h"
#include "bzfsAPI.h"
#include "FlagInfo.h"
#include "ShotUpdate.h"
#include "ShotManager.h"
#include "bzfsAPIServerSidePlayers.h"
#include "Obstacle.h"

using namespace Shots;

class ShotSlotInfo
{
public:
    ShotSlotInfo() {};

    int         slotID = -1;
    Shot::Ptr   activeShot = nullptr;
    double      expireTime = -1.0f;
    bool        reloading = false;
};

const int PlayerSlot = MaxPlayers + ReplayObservers;

typedef void (*tcpCallback)(NetHandler &netPlayer, int i, const RxStatus e);

/** This class is meant to be the container of all the global entity that lives
    into the game and methods to act globally on those.
    Up to now it contain players. Flag class is only there as a TODO
*/
class GameKeeper
{
public:
    class Player
    {
    public:
        Player(int _playerIndex, const struct sockaddr_in &clientAddr, int fd, tcpCallback _clientCallback);
        Player(int _playerIndex, NetHandler *handler, tcpCallback _clientCallback);
        Player(int _playerIndex, bz_ServerSidePlayerHandler *handler);
        ~Player();

        int             getIndex();
        static int      getFreeIndex(int min, int max);
        static Player*  getPlayerByIndex(int _playerIndex);
        static int      count();
        static void     updateLatency(float &waitTime);
        static void     dumpScore();
        static int      anointRabbit(int oldRabbit);
        static std::vector<int> allowed(PlayerAccessInfo::AccessPerm right, int targetPlayer = -1);
        static int      getPlayerIDByName(const std::string &name);
        static void     reloadAccessDatabase();

        bool            loadEnterData(uint16_t& rejectCode, char* rejectMsg);
        void            packAdminInfo(MessageBuffer::Ptr msg);
        void            packPlayerInfo(MessageBuffer::Ptr msg);
        void            packPlayerUpdate(MessageBuffer::Ptr msg);

        void            setPlayerAddMessage(PlayerAddMessage &msg);

        void            signingOn(bool ctf);
        void            close();
        static bool     clean();
        void            handleTcpPacket(fd_set *set);

        // For hostban checking, to avoid check and check again
        static void     setAllNeedHostbanChecked(bool set);
        void            setNeedThisHostbanChecked(bool set);
        bool            needsHostbanChecked();

        // To handle player State
        void            setPlayerState(float pos[3], float azimuth);
        void            getPlayerState(float pos[3], float &azimuth);
        void            setPlayerState(PlayerState state, float timestamp);

        void            setBzIdentifier(const std::string& id);
        const std::string& getBzIdentifier() const;

        // When is the player's next GameTime?
        const TimeKeeper&   getNextGameTime() const;
        void                updateNextGameTime();

        // To handle Identify
        void            setLastIdFlag(int _idFlag);
        int             getLastIdFlag();

        // To handle shot
        static void         setMaxShots(unsigned int _maxShots);
        bool                isValidShotToShoot(FiringInfo &firingInfo);
        bool                canShoot();
        bool                addShot(Shot::Ptr);
        bool                removeShot(int gid);
        void                updateShotSlots();
        std::vector<float>  getSlotReloads();
        void                update();

        void grantFlag(int _flag);

        FlagType::Ptr       getFlagType() const;
        FlagEffect          getFlagEffect() const;

        const Obstacle*  getHitBuilding(const float* p, float a, bool phased, bool& expelled) const;
        const Obstacle*  getHitBuilding(const float* oldP, float oldA, const float* p, float a, bool phased, bool& expelled);
        bool             getHitNormal(const Obstacle* o, const float* pos1, float azimuth1, const float* pos2, float azimuth2, float* normal) const;

        inline const float* getDimensions() const
        {
            return current_dimensions;
        }

        inline float getRadius() const
        {
            // NOTE: this encompasses everything but Narrow
            //       the Obese, Tiny, and Thief flags adjust
            //       the radius, but Narrow does not.
            return current_dimensions[0] * BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
        }


        inline float        getMuzzleHeight() const 
        {
            return/* dimensionsScale[2] * */BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
        }

        bool validTeamTarget(const Player *possibleTarget) const;

        enum LSAState
        {
            start,
            notRequired,
            required,
            requesting,
            checking,
            timedOut,
            failed,
            verified,
            done
        } _LSAState;

        // players
        PlayerInfo        player;
        // Net Handler
        std::shared_ptr<NetHandler> netHandler;
        // player lag info
        LagInfo       lagInfo;
        // player access
        PlayerAccessInfo  accessInfo;
        // Last known position, vel, etc
        PlayerState       lastState;
        float         stateTimeStamp;
        float         serverTimeStamp;
        // GameTime update
        float         gameTimeRate;
        TimeKeeper        gameTimeNext;
        // FlagHistory
        FlagHistory       flagHistory;
        // Score
        Score         score;
        // Authentication
        Authentication    authentication;

        std::map<std::string, std::string> extraData;
        class AttributeModifiers
        {
        public:
            float reloadMultiplyer = 1.0f;
        };

        AttributeModifiers Attributes;

        // flag to let us know the player is on it's way out
        bool  isParting;

        bool hasEntered;

        // logic class for server side players
        bz_ServerSidePlayerHandler* playerHandler;

        bool addWasDelayed;
        bool hadEnter;
        double addDelayStartTime;

        int lastHeldFlagID;

        bool isTransferingWorld = false;
        std::list<int> chunksLeft;

        inline bool isFlagActive() const
        {
            return (lastState.status & short(PlayerState::FlagActive)) != 0;
        }

        inline bool isTeleporting() const
        {
            return (lastState.status & short(PlayerState::Teleporting)) != 0;
        }

        inline bool isExploding() const
        {
            return (lastState.status & short(PlayerState::Exploding)) != 0;
        }

        bool isPhantomZoned() const
        {
            if (!player.haveFlag())
                return false;

            auto flag =  FlagInfo::get(player.getFlag())->flag.type;
            return (isFlagActive() && (flag->flagEffect == FlagEffect::PhantomZone));
        }


        // data use for server side physics
        float           desiredSpeed = 0;
        float           desiredAngVel = 0;
        int             deathPhyDrv;    // physics driver that caused death

        TimeKeeper      lastUpdateSent;
        TimeKeeper      agilityTime;
        TimeKeeper      bounceTime;
        TimeKeeper      teleportTime;     // time I started teleporting
        short           fromTeleporter;       // teleporter I entered
        short           toTeleporter;         // teleporter I exited
        float           flagShakingTime;
        int             flagShakingWins;
        float           flagAntidotePos[3];
        bool            hasAntidoteFlag = false;

        void            setDesiredSpeed(float fracOfMaxSpeed);
        void            setDesiredAngVel(float fracOfMaxAngVel);

        void            setTeleport(const TimeKeeper& t, short from, short to);

        float           getHandicapFactor();

        void            setPhysicsDriver(int driver);

        void            move(const float* _pos, float _azimuth);
        void            collectInsideBuildings();

        bool            isDeadReckoningWrong() const;

        class UpdateInfo
        {
        public:
            float pos[3];
            float vec[3];       // forwad vector * speed
            float forward[3];   // direction the tank if facing
            float rot = 0;          // radians like azimuth
            double time = 0;

            float speed = 0;
            float angVel = 0;

            int phydrv = -1;

            bz_eTankStatus Status = bz_eTankStatus::Dead;

            short pStatus = PlayerState::DeadStatus;

            UpdateInfo()
            {
                for (int i = 0; i < 3; i++)
                    pos[i] = vec[0] = 0;
            }

            UpdateInfo& operator=(const UpdateInfo& u)
            {
                memcpy(pos, u.pos, sizeof(float) * 3);
                memcpy(vec, u.vec, sizeof(float) * 3);
                rot = u.rot;
                angVel = u.angVel;
                time = u.time;
                pStatus = u.pStatus;
                Status = u.Status;
                speed = u.speed;
                phydrv = u.phydrv;

                return *this;
            }

            inline short getPStatus() const
            {
                return pStatus;
            }

            inline void setPStatus(short _status)
            {
                pStatus = _status;
            }

            float getDelta(const UpdateInfo & state)
            {
                // plot where we think we are now based on the current time
                double dt = state.time - time;

                float newPos[3];
                newPos[0] = pos[0] + (float)(vec[0] * dt);
                newPos[1] = pos[1] + (float)(vec[1] * dt);
                newPos[2] = pos[2] + (float)(vec[2] * dt);

                // that's where we thing we'll be based on movement

                float dx = newPos[0] - state.pos[0];
                float dy = newPos[1] - state.pos[1];
                float dz = newPos[1] - state.pos[2];

                // return the distance between where our projection is, and where state is
                return sqrt(dx*dx + dy * dy + dz * dz);
            }

        };

        UpdateInfo lastUpdate;
        UpdateInfo currentState;

        std::vector<const Obstacle*> insideBuildings;

    private:
        static Player*      playerList[PlayerSlot];
        int                 playerIndex;
        bool                closed;
        tcpCallback         clientCallback;
        std::string         bzIdentifier;
        bool                needThisHostbanChecked;
        // In case you want recheck all condition on all players
        static bool         allNeedHostbanChecked;

        static unsigned int       maxShots;
        std::vector<ShotSlotInfo> shotSlots;

        int                 idFlag;

        double              lastShotUpdateTime = -1;

        float               base_dimensions[3];     // unmodified tank dimenstions    
        float               current_dimensions[3];  // current tank dimensions

        void setupPhysicsData();
        void updateDimensions();
        void getDeadReckoning(float* predictedPos, float* predictedAzimuth, float* predictedVel, float time) const;
        void calcRelativeMotion(float vel[2], float& speed, float& angVel) const;
    };
};

inline int GameKeeper::Player::getIndex()
{
    return playerIndex;
}

inline GameKeeper::Player* GameKeeper::Player::getPlayerByIndex(int
        _playerIndex)
{
    if (_playerIndex < 0 || _playerIndex >= PlayerSlot)
        return 0;
    if (!playerList[_playerIndex])
        return 0;
    if (playerList[_playerIndex]->closed)
        return 0;
    return playerList[_playerIndex];
}

void PackPlayerInfo(MessageBuffer::Ptr msg, int playerIndex, uint8_t properties );

// For hostban checking, to avoid check and check again
inline void GameKeeper::Player::setAllNeedHostbanChecked(bool set)
{
    allNeedHostbanChecked = set;
}

inline void GameKeeper::Player::setNeedThisHostbanChecked(bool set)
{
    needThisHostbanChecked = set;
}

inline bool GameKeeper::Player::needsHostbanChecked()
{
    return (allNeedHostbanChecked || needThisHostbanChecked);
}


inline void GameKeeper::Player::setBzIdentifier(const std::string& id)
{
    bzIdentifier = id;
}

inline const std::string& GameKeeper::Player::getBzIdentifier() const
{
    return bzIdentifier;
}


inline const TimeKeeper& GameKeeper::Player::getNextGameTime() const
{
    return gameTimeNext;
}


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
