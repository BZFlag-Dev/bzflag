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
#include "BZDBCache.h"
#include "playing.h"
#include "LocalPlayer.h"
#include "Roster.h"
#include "TargetingUtils.h"
#include "World.h"

/**
 * Plan
 */
Plan::Plan(float planDuration)
{
	planExpiration = TimeKeeper::getCurrent();
	planExpiration += planDuration;
}


Plan::~Plan()
{
}


bool Plan::isValid()
{
	TimeKeeper now = TimeKeeper();
	float delta = now - planExpiration;
	return (delta < 0.0f);
}

void Plan::execute()
{
	float pos[3];
	LocalPlayer *myTank = LocalPlayer::getMyTank();
	memcpy(pos, myTank->getPosition(), sizeof(pos));
	if (pos[2] < 0.0f)
		pos[2] = 0.01f;
	float myAzimuth = myTank->getAngle();

	float dir[3] = {cosf(myAzimuth), sinf(myAzimuth), 0.0f};
	pos[2] += myTank->getMuzzleHeight();
	Ray tankRay(pos, dir);
	pos[2] -= myTank->getMuzzleHeight();

	if (myTank->getFlag() == Flags::ShockWave) {
		TimeKeeper now = TimeKeeper::getTick();
		if (now - lastShot >= (1.0f / World::getWorld()->getMaxShots())) {
			bool hasSWTarget = false;
			for (int t = 0; t < curMaxPlayers; t++) {
				if (t != myTank->getId() && player[t] &&
					player[t]->isAlive() && !player[t]->isPaused() &&
					!player[t]->isNotResponding()) {

					const float *tp = player[t]->getPosition();
					float enemyPos[3];

					//toss in some lag adjustment/future prediction - 300 millis
					memcpy(enemyPos,tp,sizeof(enemyPos));
					const float *tv = player[t]->getVelocity();
					enemyPos[0] += 0.3f * tv[0];
					enemyPos[1] += 0.3f * tv[1];
					enemyPos[2] += 0.3f * tv[2];

					if (enemyPos[2] < 0.0f)
						enemyPos[2] = 0.0f;
					float dist = TargetingUtils::getTargetDistance( pos, enemyPos );
					if (dist <= BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS)) {
						if (!myTank->validTeamTarget(player[t])) {
							hasSWTarget = false;
							t = curMaxPlayers;
						} else {
							hasSWTarget = true;
						}
					}
				}
			}
			if (hasSWTarget) {
				myTank->fireShot();
				lastShot = TimeKeeper::getTick();
			}
		}
	} else {
		TimeKeeper now = TimeKeeper::getTick();
		if (now - lastShot >= (1.0f / World::getWorld()->getMaxShots())) {

			float errorLimit = World::getWorld()->getMaxShots() * BZDB.eval(StateDatabase::BZDB_LOCKONANGLE) / 8.0f;
			float closeErrorLimit = errorLimit * 2.0f;

			for (int t = 0; t < curMaxPlayers; t++) {
				if (t != myTank->getId() && player[t] &&
					player[t]->isAlive() && !player[t]->isPaused() &&
					!player[t]->isNotResponding() &&
					myTank->validTeamTarget(player[t])) {

					if (player[t]->isPhantomZoned() && !myTank->isPhantomZoned()
					&& (myTank->getFlag() != Flags::SuperBullet))
						continue;

					const float *tp = player[t]->getPosition();
					float enemyPos[3];
					//toss in some lag adjustment/future prediction - 300 millis
					memcpy(enemyPos,tp,sizeof(enemyPos));
					const float *tv = player[t]->getVelocity();
					enemyPos[0] += 0.3f * tv[0];
					enemyPos[1] += 0.3f * tv[1];
					enemyPos[2] += 0.3f * tv[2];
					if (enemyPos[2] < 0.0f)
						enemyPos[2] = 0.0f;

					float dist = TargetingUtils::getTargetDistance( pos, enemyPos );

					if ((myTank->getFlag() == Flags::GuidedMissile) || (fabs(pos[2] - enemyPos[2]) < 2.0f * BZDBCache::tankHeight)) {

						float targetDiff = TargetingUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos );
						if ((targetDiff < errorLimit)
						||  ((dist < (2.0f * BZDB.eval(StateDatabase::BZDB_SHOTSPEED))) && (targetDiff < closeErrorLimit))) {
							bool isTargetObscured;
							if (myTank->getFlag() != Flags::SuperBullet)
								isTargetObscured = TargetingUtils::isLocationObscured( pos, enemyPos );
							else
								isTargetObscured = false;

							if (!isTargetObscured) {
								myTank->fireShot();
								lastShot = now;
								t = curMaxPlayers;
							}
						}
					}
				}
			}
		}
	}
}

/**
 * PlanStack
 */

PlanStack::PlanStack()
{
	Plan *pPlan = new TopLevelPlan();
	plans.push(pPlan);
}

void PlanStack::execute()
{
	Plan *pPlan = NULL;
	
	while (plans.size() > 0) {
		pPlan = plans.top();
		if (!pPlan->isValid()) {
			delete pPlan;
			plans.pop();
		}
	}

	while (pPlan->usesSubPlan()) {
		pPlan = pPlan->createSubPlan();
		plans.push(pPlan);
	}

	pPlan->execute();
}

/**
 * TopLevelPlan
 */

TopLevelPlan::TopLevelPlan()
: Plan(0)
{
}

bool TopLevelPlan::isValid()
{	//always valid
	return true;
}

bool TopLevelPlan::usesSubPlan()
{
	return true;
}

Plan *TopLevelPlan::createSubPlan()
{	//TODO: Pick a plan
	return NULL;
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

void GotoPointPlan::execute()
{
	//TODO: goto point, then

	Plan::execute();
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

void WeavePlan::execute()
{
	//TODO: weave, then

	Plan::execute();
}

/**
 * HuntPlayerPlan
 */

HuntPlayerPlan::HuntPlayerPlan()
	:Plan(300.0f)
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


/**
 * HuntTeamFlagPlan
 */

HuntTeamFlagPlan::HuntTeamFlagPlan()
:Plan(300.0f)
{
}

bool HuntTeamFlagPlan::isValid()
{
	return false;
}

bool HuntTeamFlagPlan::usesSubPlan()
{
	return true;
}

Plan *HuntTeamFlagPlan::createSubPlan()
{
	return NULL;
}

/**
 * CaptureFlagPlan
 */

CaptureFlagPlan::CaptureFlagPlan()
	:Plan(1200.0f)
{
}

bool CaptureFlagPlan::isValid()
{
	return false;
}

bool CaptureFlagPlan::usesSubPlan()
{
	return true;
}

Plan *CaptureFlagPlan::createSubPlan()
{
	return NULL;
}

