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

// interface header
#include "ShotManager.h"
#include "TimeKeeper.h"
#include "vectors.h"

namespace Shots
{
	//----------------Manager

	double Manager::DeadShotCacheTime = 10.0;

	Manager::Manager()
	{
		Logics[std::string("")] = new FlightLogic();
		LastGUID = INVALID_SHOT_GUID;
	}

	Manager::~Manager()
	{
		for (FlightLogicMap::iterator itr = Logics.begin(); itr != Logics.end(); itr++)
			delete((itr++)->second);

		Logics.clear();

		for (ShotList::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
			delete(*itr);

		LiveShots.clear();

		for (ShotList::iterator itr = RecentlyDeadShots.begin(); itr != RecentlyDeadShots.end(); itr++)
			delete(*itr);

		RecentlyDeadShots.clear();
	}

	void Manager::Init()
	{
		Logics[Flags::GuidedMissile->flagAbbv] = new GuidedMissileLogic();
		Logics[Flags::SuperBullet->flagAbbv] = new SuperBulletLogic();
	}

	void Manager::SetFlightLogic(const char* flagCode, FlightLogic* logic)
	{
		std::string code;
		if (flagCode)
			code = flagCode;

		if (Logics.find(code) != Logics.end())
			delete(Logics.find(code)->second);

		Logics[code] = logic;
	}

	uint32_t Manager::AddShot( const FiringInfo &info, PlayerId shooter )
	{
		FlightLogic* logic = NULL;
		if(Logics.find(info.flagType->flagAbbv) != Logics.end())
			logic = Logics[info.flagType->flagAbbv];

		if (!logic)
			logic = Logics[""];

		Shot *shot = new Shot(NewGUID(),info,*logic);
		LiveShots.push_back(shot);
		return shot->GetGUID();
	}

	void Manager::RemoveShot (uint32_t shotID)
	{
		ShotList::iterator itr = LiveShots.begin();
		while ( itr != LiveShots.end())
		{
			if ((*itr)->GetGUID() == shotID)
			{
				(*itr)->End();
				RecentlyDeadShots.push_back((*itr));
				itr = LiveShots.erase(itr);
				return;
			}
			else
				itr++;
		}
	}

	void Manager::RemovePlayer( PlayerId player )
	{
		ShotList::iterator itr = LiveShots.begin();
		while ( itr != LiveShots.end())
		{
			if ((*itr)->GetPlayerID() == player)
			{
				(*itr)->End();
				itr = LiveShots.erase(itr);
			}
			else
				itr++;
		}
	}

	void Manager::SetShotTarget( uint32_t shotID, PlayerId target )
	{
		Shot* shot = FindByID(shotID);
		if (shot)
			shot->Retarget(target);
	}

	uint32_t Manager::FindShot (PlayerId shooter, uint16_t localShotID)
	{
		for (ShotList::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
		{
			if ((*itr)->GetPlayerID() == shooter && (*itr)->Info.shot.id == localShotID)
				return (*itr)->GetGUID();
		}
		return 0;
	}

	uint32_t Manager::NewGUID()
	{
		LastGUID++;

		if (LastGUID == INVALID_SHOT_GUID) // handle the rollover
			LastGUID = INVALID_SHOT_GUID + 1;

		return LastGUID;
	}

	Shot* Manager::FindByID (uint32_t shotID)
	{
		for (ShotList::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
		{
			if ((*itr)->GetGUID() == shotID)
				return *itr;
		}
		return NULL;
	}

	void Manager::Update()
	{
		ShotList::iterator itr = LiveShots.begin();
		while ( itr != LiveShots.end())
		{
			if ((*itr)->Update())
			{
				Shot* shot = *itr;
				itr = LiveShots.erase(itr);
				shot->End();
				RecentlyDeadShots.push_back(shot);
			}
			else
				itr++;
		}

		double now = TimeKeeper::getCurrent().getSeconds();

		itr = RecentlyDeadShots.begin();

		while ( itr != RecentlyDeadShots.end())
		{
			if (now - (*itr)->GetLastUpdateTime() >= Manager::DeadShotCacheTime)
				itr = RecentlyDeadShots.erase(itr);
			else
				itr++;
		}
	}

	ShotList Manager::LiveShotsForPlayer( PlayerId player )
	{
		ShotList list;

		for (ShotList::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
		{
			if ((*itr)->GetPlayerID() == player)
				list.push_back(*itr);
		}

		return list;
	}

	//----------------Shot

	Shot::Shot(uint32_t guid, const FiringInfo &info, FlightLogic& logic): GUID(guid), Logic(logic), Info(info), LastUpdateTime(-1.0)
	{
		StartTime = TimeKeeper::getCurrent().getSeconds();
		LifeTime = info.lifetime;
		Target = NoPlayer;
	}

	bool Shot::Update()
	{
		if (TimeKeeper::getCurrent().getSeconds() - StartTime >= LifeTime || Logic.Update(*this))
			return true;

		LastUpdateTime = TimeKeeper::getCurrent().getSeconds();
		return false;
	}

	void Shot::End()
	{
		Logic.End(*this);
		LastUpdateTime = TimeKeeper::getCurrent().getSeconds();
	}

	void Shot::Retarget(PlayerId target)
	{
		Logic.Retarget(*this,target);
		Target = target;
	}

	fvec3 FlightLogic::ProjectShotLocation( Shot &shot, double deltaT )
	{
		fvec3 vec;
		vec.x = shot.LastUpdatePosition.x + (shot.Info.shot.vel[0] * (float)deltaT);
		vec.y = shot.LastUpdatePosition.y + (shot.Info.shot.vel[1] * (float)deltaT);
		vec.z = shot.LastUpdatePosition.z + (shot.Info.shot.vel[2] * (float)deltaT);

		return vec;
	}
	//----------------GuidedMissileLogic

	bool GuidedMissileLogic::Update ( Shot& /*shot*/ )
	{
		// compute the distance to target and knock that sucker down
		return false;
	}

	void GuidedMissileLogic::End (  Shot& /*shot*/ )
	{

	}

//----------------SuperBulletLogic
	bool SuperBulletLogic::Update ( Shot& shot )
	{
		shot.LastUpdatePosition = ProjectShotLocation(shot,shot.GetLastUpdateTime() - shot.GetStartTime());
		return false;
	}

}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
