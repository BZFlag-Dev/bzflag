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

#ifndef	PLAN_H
#define	PLAN_H

#include "TimeKeeper.h"

/**
 * Roger will follow a plan. These plans are stored on a plan stack
 * allowing for short medium and long range plans to cooperate to 
 * produce a better playing bot.
 */
class Plan
{
public:
	Plan( float planDuration );
	virtual ~Plan();
	/**
	 * checks to make sure this plan is still valid. The default
	 * implementation just checks for an expiration time, to keep
	 * roger from bullheadedly trying something that just aint
	 * gonna work. You can override this method for specific plans
	 */
	virtual bool isValid();

	/**
	 * determines if this plan is a longer range plan that can
	 * only be accomplished with smaller steps. Hunting a player
	 * might consist of avoiding a building, or weaving, for instance.
	 * if this method returns true, than this plan is just a coordinator
	 * of sub plans.
     */
	virtual bool usesSubPlan() = 0;

	/**
	 * creates a sub plan to be placed on the stack to accomplish
	 * this plan's goals.
	 */
	virtual Plan *createSubPlan() = 0;

	/**
	 * execute the plan. set the rotation and speed
	 *
	 */
	virtual void execute( float &rotation, float &velocity ) = 0;

private:
	TimeKeeper planExpiration;
};

class GotoPointPlan : public Plan
{
public:
	GotoPointPlan(float *pt);
	
	virtual bool usesSubPlan();
	virtual Plan *createSubPlan() ;
	virtual void execute( float &rotation, float &velocity );

private:
	float gotoPt[3];
};

class WeavePlan : public Plan
{
public:
	WeavePlan(int pID, bool right );

	virtual bool isValid();
	virtual bool usesSubPlan();
	virtual Plan* createSubPlan();
	virtual void execute( float &rotation, float &velocity );

private:
	int playerID;
	bool weaveRight;
};

class HuntPlayerPlan : public Plan
{
public:
	HuntPlayerPlan();

	virtual bool isValid();
	virtual bool usesSubPlan();
	virtual Plan *createSubPlan();
	virtual void execute( float &, float &);

private:
	int playerID;
};

class HuntTeamFlagPlan : public Plan
{
public:
	HuntTeamFlagPlan();

	virtual bool isValid();
	virtual bool usesSubPlan();
	virtual Plan *createSubPlan();
	virtual void execute( float &, float &);
private:
	int flagID;
};

class CaptureFlagPlan : public Plan
{
public:
	CaptureFlagPlan();

	virtual bool isValid();
	virtual bool usesSubPlan();
	virtual Plan *createSubPlan();
	virtual void execute( float &, float &);
};

#endif
