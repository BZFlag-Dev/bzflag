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

#ifndef __SHOTMANAGER_H__
#define __SHOTMANAGER_H__

#include "common.h"

/* common interface headers */
#include "global.h"  /* for TeamColor */
#include "ShotUpdate.h"
#include "vectors.h"
#include "TimeKeeper.h"

#include "Obstacle.h"

#include <string>
#include <vector>
#include <map>
#ifdef USE_TR1
#include <tr1/memory>
#include <tr1/functional>
#define shared_ptr  tr1::shared_ptr
#define function    tr1::function
#else
#include <memory>
#include <functional>
#endif

#include "Ray.h"

/** a ShotManager is used track shots fired by players and the server
*/

namespace Shots
{
    class Shot
    {
    protected:
        uint16_t GUID;

        double  LifeTime;

        class MetaDataItem
        {
        public:
            std::string Name;
            std::string DataS;
            uint32_t    DataI;
        };

        std::map<std::string, MetaDataItem> MetaData;

        virtual fvec3 ProjectShotLocation(double deltaT);

    public:
        typedef std::shared_ptr <Shot> Ptr;
        typedef std::vector<std::shared_ptr<Shot>> Vec;
        typedef std::shared_ptr<std::function <void(Shot&)> > Event;

        fvec3       StartPosition;
        fvec3       LastUpdatePosition;
        fvec3       LastUpdateVector;
        double      LastUpdateTime;
        double      StartTime;

        FiringInfo  Info;

        PlayerId    Target;

        void        *Pimple;

        Shot(uint16_t guid, const FiringInfo &info);
        virtual ~Shot();

        uint16_t GetGUID()
        {
            return GUID;
        }
        uint16_t GetLocalShotID()
        {
            return Info.shot.player;
        }

        PlayerId GetPlayerID()
        {
            return Info.shot.player;
        }

        double GetLastUpdateTime()
        {
            return LastUpdateTime;
        }

        double GetStartTime()
        {

            return StartTime;
        }
        double GetLifeTime()
        {
            return LifeTime;
        }

        double GetLifeParam()
        {
            return (LastUpdateTime - StartTime) / LifeTime;
        }

        void setPosition(const float* p)
        {
            Info.shot.pos[0] = p[0];
            Info.shot.pos[1] = p[1];
            Info.shot.pos[2] = p[2];
        }

        void setVelocity(const float* v)
        {
            Info.shot.vel[0] = v[0];
            Info.shot.vel[1] = v[1];
            Info.shot.vel[2] = v[2];
        }


        virtual void Setup() {}
        virtual bool Update(float dt); // call the base class for lifetime expire
        virtual void End();
        virtual void Retarget(PlayerId UNUSED(newTarget));

        virtual void ProcessUpdate(ShotUpdate& UNUSED(referece)) {}

        virtual bool CollideBox(fvec3& UNUSED(center), fvec3& UNUSED(size), float UNUSED(rotation))
        {
            return false;
        }
        virtual bool CollideSphere(fvec3& UNUSED(center), float UNUSED(radius))
        {
            return false;
        }
        virtual bool CollideCylinder(fvec3& UNUSED(center), float UNUSED(height), float UNUSED(radius))
        {
            return false;
        }


        // meta data API
        void SetMetaData(const std::string& name, const char* data);
        void SetMetaData(const std::string& name, uint32_t data);
        bool HasMetaData(const std::string& name);
        const char * GetMetaDataS(const std::string& name);
        uint32_t GetMetaDataI(const std::string& name);
    };

    class ShotFactory
    {
    public:
        virtual ~ShotFactory() {}
        virtual  Shot::Ptr GetShot(uint16_t /*guid*/, const FiringInfo &/*info*/) = 0;

        typedef std::shared_ptr <ShotFactory> Ptr;
        typedef std::map<FlagEffect, Ptr > Map;
    };

    const Obstacle* getFirstBuilding(const Ray& ray, float min, float& t);

#define INVALID_SHOT_GUID 0
#define MAX_SHOT_GUID 0xFFFF

    class Manager
    {
    public:
        Manager();
        virtual ~Manager();

        void Init();

        void SetShotFactory(FlagEffect effect, std::shared_ptr<ShotFactory> factory);

        uint16_t AddShot(const FiringInfo &info, PlayerId shooter);
        void RemoveShot(uint16_t shotID);

        void RemovePlayer(PlayerId player);

        void UpdateShot(uint16_t shotID, ShotUpdate& update);
        void SetShotTarget(uint16_t shotID, PlayerId target);

        uint16_t FindShotGUID(PlayerId shooter, uint16_t localShotID);
        Shot::Ptr FindShot(uint16_t shotID)
        {
            return FindByID(shotID);
        }

        bool IsValidShotID(uint16_t shotID);

        void Update();

        static double DeadShotCacheTime;

        Shot::Vec    LiveShotsForPlayer(PlayerId player);
        Shot::Vec    DeadShotsForPlayer(PlayerId player);

        std::vector<int> AllLiveShotIDs();
        std::vector<int> ShotIDsInRadius( float pos[3], float radius);

        Shot::Event ShotCreated;
        Shot::Event ShotEnded;

    private:
        uint16_t NewGUID();
        Shot::Ptr FindByID(uint16_t shotID);

        double Now();

        Shot::Vec    LiveShots;
        Shot::Vec    RecentlyDeadShots;

        ShotFactory::Map Factories;

        uint16_t    LastGUID;

    };

    enum  class SegmentReason { Initial, Through, Ricochet, Teleport, Boundary };

    class FlightSegment
    {
    public:

        FlightSegment();
        FlightSegment(const double start, const double end, const Ray& r, SegmentReason = SegmentReason::Initial);
        FlightSegment(const FlightSegment&);
        ~FlightSegment();
        FlightSegment&    operator=(const FlightSegment&);

    public:
        double      start;
        double      end;
        Ray             ray;
        SegmentReason   reason;
        float           bbox[2][3];
    };

    class ProjectileShot : public Shot
    {
    public:
        ProjectileShot(uint32_t guid, const FiringInfo &info) : Shot(guid, info) {}

        virtual ~ProjectileShot() {}

        virtual void Setup();
        virtual bool Update(float dt);

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<ProjectileShot>(guid, info); }
        };

        enum class ObstacleEffect
        {
            Stop = 0,
            Through = 1,
            Reflect = 2
        };

    protected:
        std::vector<FlightSegment> Segments;

        double      prevTime;
        double      currentTime;
        double      lastTime;
        int         segment, lastSegment;
        float       bbox[2][3];

        void makeSegments(ObstacleEffect e);

        virtual bool ForceShotRico();
    };

    class RicoShot : public ProjectileShot
    {
    public:
        RicoShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}
        virtual ~RicoShot() {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<RicoShot>(guid, info); }
        };
    };

    class RapidFireShot : public ProjectileShot
    {
    public:
        RapidFireShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}
        virtual ~RapidFireShot() {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<RapidFireShot>(guid, info); }
        };
    };

    class ThiefShot : public ProjectileShot
    {
    public:
        ThiefShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}
        virtual ~ThiefShot() {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<ThiefShot>(guid, info); }
        };
    };

    class MachineGunShot : public ProjectileShot
    {
    public:
        MachineGunShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}
        virtual ~MachineGunShot() {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<MachineGunShot>(guid, info); }
        };
    };

    class LaserShot : public ProjectileShot
    {
    public:
        LaserShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}
        virtual ~LaserShot() {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<LaserShot>(guid, info); }
        };
    };

    class PhantomShot : public ProjectileShot
    {
    public:
        PhantomShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}
        virtual ~PhantomShot() {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<PhantomShot>(guid, info); }
        };
    };


    class GuidedMissileShot : public Shot
    {
    public:
        GuidedMissileShot(uint32_t guid, const FiringInfo &info) : Shot(guid, info) {}
        virtual ~GuidedMissileShot() {}

        virtual void End();
        virtual void ProcessUpdate(ShotUpdate& update);

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<GuidedMissileShot>(guid, info); }
        };
    };

    class SuperBulletShot : public ProjectileShot
    {
    public:
        SuperBulletShot(uint32_t guid, const FiringInfo &info) : ProjectileShot(guid, info) {}

        virtual void Setup();

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<SuperBulletShot>(guid, info); }
        };
    };

    class ShockwaveShot : public Shot
    {
    public:
        ShockwaveShot(uint32_t guid, const FiringInfo &info) : Shot(guid, info) {}
        virtual void Setup();
        virtual bool Update(float dt);

        virtual bool CollideBox(fvec3& ecnter, fvec3& size, float rotation);
        virtual bool CollideSphere(fvec3& center, float radius);
        virtual bool CollideCylinder(fvec3& center, float height, float radius);

        class Factory : public ShotFactory
        {
        public:
            virtual ~Factory() {}
            virtual  Shot::Ptr GetShot(uint16_t guid, const FiringInfo &info) { return std::make_shared<ShockwaveShot>(guid, info); }
        };

    protected:
        bool PointInSphere(fvec3& point);
    };
}
#endif  /*__SHOTMANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
