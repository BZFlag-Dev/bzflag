#include "BZAdvancedRobot.h"

#include "RCRequests.h"

void BZAdvancedRobot::execute()
{
  link->sendAndProcess(ExecuteReq(), this);
}

double BZAdvancedRobot::getDistanceRemaining()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getTurnRemaining()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::setAhead(double distance)
{
  /* TODO: Implement this. */
  return 0.0;
}

void BZAdvancedRobot::setFire()
{
  link->sendAndProcess(SetFireReq(), this);
}

// TODO: Implement 'Bullet setFireBullet()'?
void BZAdvancedRobot::setTurnRate(double turnRate)
{
}

void BZAdvancedRobot::setSpeed(double speed)
{
}

void BZAdvancedRobot::setResume()
{
}

void BZAdvancedRobot::setStop()
{
}

void BZAdvancedRobot::setStop(bool overwrite)
{
}

void BZAdvancedRobot::setTurnLeft(double degrees)
{
}


// These are normally in Robot and not AdvancedRobot, but due to
// the upside-down hierarchy we have - they're here instead ;-)
double BZAdvancedRobot::getBattleFieldHeight()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getBattleFieldWidth()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getGunHeat()
{
  link->sendAndProcess(GetGunHeatReq(), this);
  return gunHeat;
}

double BZAdvancedRobot::getHeading()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getHeight()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getWidth()
{
  /* TODO: Implement this. */
  return 0.0;
}

int BZAdvancedRobot::getOthers()
{
  /* TODO: Implement this. */
  return 0;
}

long BZAdvancedRobot::getTime()
{
  /* TODO: Implement this. */
  return 0;
}

double BZAdvancedRobot::getVelocity()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getX()
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getY()
{
  /* TODO: Implement this. */
  return 0.0;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
