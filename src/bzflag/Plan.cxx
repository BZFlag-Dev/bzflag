/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "Plan.h"
#include "playing.h"
#include "LocalPlayer.h"
#include "TargetingUtils.h"

/**
 * Plan
 */
Plan::Plan(float planDuration)
{
	planExpiration = TimeKeeper::getCurrent();
	planExpiration += planDuration;
}


bool Plan::isValid()
{
	TimeKeeper now = TimeKeeper();
	float delta = now - planExpiration;
	return (delta < 0.0f);
}

/**
 * GotoPointPlan
 */

GotoPointPlan::GotoPointPlan(float *pt)
	: Plan(20.0f)
{
	memcpy( gotoPt, pt, sizeof( gotoPt ));
}
	
bool GotoPointPlan::usesSubPlan()
{
	return false;
}

Plan *GotoPointPlan::createSubPlan() 
{
	return NULL;
}

void GotoPointPlan::execute( float &rotation, float &velocity )
{
}

/**
 * WeavePlan
 */

WeavePlan::WeavePlan(int pID, bool right )
	: Plan(10.0)
{
	playerID = pID;
	weaveRight = right;
}

bool WeavePlan::isValid()
{
	LocalPlayer *myTank = LocalPlayer::getMyTank();
	const float *pVel = myTank->getVelocity();
	return (pVel[0] > 0.0f) || (pVel[1] > 0.0f) || (pVel[2] > 0.0f);
}

bool WeavePlan::usesSubPlan()
{
	return false;
}

Plan* WeavePlan::createSubPlan()
{
	return NULL;
}

void WeavePlan::execute( float &rotation, float &velocity )
{
}

/**
 * HuntPlayerPlan
 */

HuntPlayerPlan::HuntPlayerPlan()
	:Plan(MAXFLOAT)
{
	//Pick a player ID to hunt
	playerID = 0;
}
bool HuntPlayerPlan::isValid()
{
	if (!Plan::isValid())
		return false;

	Player *pPlayer = lookupPlayer(playerID);
	if (pPlayer == NULL)
		return false;

	if (!pPlayer->isAlive())
		return false;

	LocalPlayer *myTank = LocalPlayer::getMyTank();
	if (pPlayer->getTeam() == myTank->getTeam())
		return false;

	return true;
}

bool HuntPlayerPlan::usesSubPlan()
{
	return true;
}

Plan *HuntPlayerPlan::createSubPlan()
{
	Player *pPlayer = lookupPlayer(playerID);
 	LocalPlayer *myTank = LocalPlayer::getMyTank();
    bool isObscured = TargetingUtils::isLocationObscured( myTank->getPosition(), pPlayer->getPosition());
	if (isObscured) {
		float pt[3];
		// fill in pt with a open spot to go to
		return new GotoPointPlan(pt);
	} else {
		return new WeavePlan(playerID, bzfrand() > 0.5f);
	}
}

void HuntPlayerPlan::execute( float &, float *)
{
}


