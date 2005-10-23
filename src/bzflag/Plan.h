/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	PLAN_H
#define	PLAN_H

// bzflag global header
#include "common.h"

#include <stack>
#include "TimeKeeper.h"

class ShotPath;

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
   *
   * @returns whether this plan is still valid
   */
  virtual bool isValid();

  /**
   * determines if this plan is a longer range plan that can
   * only be accomplished with smaller steps. Hunting a player
   * might consist of avoiding a building, or weaving, for instance.
   * if this method returns true, than this plan is just a coordinator
   * of sub plans.
   *
   * @returns whether this plan uses sub plans
   */
  virtual bool usesSubPlan() = 0;

  /**
   * creates a sub plan to be placed on the stack to accomplish
   * this plan's goals. This method only gets called if usesSubPlan
   * return true.
   *
   * @returns a plan that can be used to accomplish part of this task
   */
  virtual Plan *createSubPlan() = 0;

  /**
   * execute the plan. The default implementation just attempts
   * to shoot at enemies. Overridden methods should call this base
   * implementation. This method only gets called if usesSubPlan
   * return false.
   *
   * @param rotation an output reference to the desired rotation
   * @param speed an output reference to the desired speed
   */
  virtual void execute(float &rotation, float &speed);

  /**
   * checks to see if i am in imminent danger. If so move so
   * that my danger level goes down. Avoiding bullets
   * is not a plan, it operates a more subconscious level.
   *
   * returns whether or not we avoided a bullet
   */
  static bool avoidBullet(float &rotation, float &speed);

private:
  static ShotPath *findWorstBullet(float &minDistance);
  TimeKeeper planExpiration;
  TimeKeeper lastShot;
};

class PlanStack
{
public:
  PlanStack();
  ~PlanStack();
  void execute(float &rotation, float &speed);
private:
  std::stack<Plan *> plans;
};

class TopLevelPlan : public Plan
{
public:
  TopLevelPlan();

  virtual bool isValid();
  virtual bool usesSubPlan();
  virtual Plan *createSubPlan() ;
};

class GotoPointPlan : public Plan
{
public:
  GotoPointPlan(float *pt);

  virtual bool usesSubPlan();
  virtual Plan *createSubPlan() ;
  virtual void execute(float &rotation, float &speed);

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
  virtual void execute(float &rotation, float &speed);

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
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

