/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
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

#include <string>
#include <vector>
#include <map>

/** a ShotManager is used track shots fired by players and the server
 */

namespace Shots
{
class FlightLogic;

class Shot
{
protected:
	uint32_t GUID;
	FlightLogic &Logic;

	
	double	StartTime;
	double	LifeTime;
	double	LastUpdateTime;

public:
	fvec3		LastUpdatePosition;
	FiringInfo	Info;

	PlayerId	Target;

	Shot(uint32_t guid, const FiringInfo &info, FlightLogic& logic);

	bool Update();
	void End();
	void Retarget(PlayerId target);

	uint32_t GetGUID() {return GUID;}
	uint16_t GetLocalShotID(){return Info.shot.player;}

	PlayerId GetPlayerID(){return Info.shot.player;}

	double GetLastUpdateTime(){return LastUpdateTime;}
	double GetStartTime(){return StartTime;}
};

typedef std::vector<Shot*> ShotList;

class FlightLogic
{
public:
	virtual ~FlightLogic(){}

	virtual bool Update ( Shot& /*shot*/ ){return false;}
	virtual void End ( Shot& /*shot*/ ){}
	virtual void Retarget ( Shot& shot, PlayerId newTarget ){};

protected:
	virtual fvec3 ProjectShotLocation(  Shot& shot, double deltaT );
};

typedef std::map<std::string, FlightLogic*> FlightLogicMap;

#define INVALID_SHOT_GUID 0

class Manager
{
public:
  Manager();
  virtual ~Manager();

  void Init();

  void SetFlightLogic(const char* flagCode, FlightLogic* logic);

  uint32_t AddShot (const FiringInfo &info, PlayerId shooter);
  void RemoveShot (uint32_t shotID);

  void RemovePlayer (PlayerId player);

  void SetShotTarget( uint32_t shot, PlayerId target );

  uint32_t FindShot (PlayerId shooter, uint16_t localShotID);

  void Update();

  static double DeadShotCacheTime;

  ShotList	LiveShotsForPlayer(PlayerId player);
  ShotList	DeadShotsForPlayer(PlayerId player);

private:
	uint32_t NewGUID();
	Shot*	FindByID(uint32_t shotID);

	ShotList	LiveShots;
	ShotList	RecentlyDeadShots;

	FlightLogicMap Logics;

	uint32_t	LastGUID;
};


class GuidedMissileLogic: public FlightLogic
{
public:
	virtual ~GuidedMissileLogic(){}

	virtual bool Update ( Shot& /*shot*/ );
	virtual void End ( Shot& /*shot*/ );
};

class SuperBulletLogic: public FlightLogic
{
public:
	virtual bool Update ( Shot& /*shot*/ );

};

}
#endif  /*__SPAWNPOLICY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
