#include "BZAdvancedRobot.h"

#include "RCRequests.h"

void BZAdvancedRobot::execute()
{
  link->sendAndProcess(ExecuteReq(), this);
}

double BZAdvancedRobot::getDistanceRemaining()
{
  link->sendAndProcess(GetDistanceRemainingReq(), this);
  return distanceRemaining;
}

double BZAdvancedRobot::getTurnRemaining()
{
  link->sendAndProcess(GetTurnRemainingReq(), this);
  return turnRemaining;
}

void BZAdvancedRobot::setAhead(double distance)
{
  link->sendAndProcess(SetAheadReq(distance), this);
}

void BZAdvancedRobot::setFire()
{
  link->sendAndProcess(SetFireReq(), this);
}

void BZAdvancedRobot::setTurnRate(double turnRate)
{
  link->sendAndProcess(SetTurnRateReq(turnRate), this);
}

void BZAdvancedRobot::setSpeed(double speed)
{
  link->sendAndProcess(SetSpeedReq(speed), this);
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
  link->sendAndProcess(SetTurnLeftReq(degrees), this);
}


double BZAdvancedRobot::getBattleFieldSize()
{
  link->sendAndProcess(GetBattleFieldSizeReq(), this);
  return battleFieldSize;
}

// These are normally in Robot and not AdvancedRobot, but due to
// the upside-down hierarchy we have - they're here instead ;-)
double BZAdvancedRobot::getGunHeat()
{
  link->sendAndProcess(GetGunHeatReq(), this);
  return gunHeat;
}

double BZAdvancedRobot::getHeading()
{
  link->sendAndProcess(GetHeadingReq(), this);
  return heading;
}

double BZAdvancedRobot::getHeight()
{
  link->sendAndProcess(GetHeightReq(), this);
  return tankHeight;
}

double BZAdvancedRobot::getWidth()
{
  link->sendAndProcess(GetWidthReq(), this);
  return tankWidth;
}

double BZAdvancedRobot::getLength()
{
  link->sendAndProcess(GetLengthReq(), this);
  return tankLength;
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
  link->sendAndProcess(GetXReq(), this);
  return xPosition;
}

double BZAdvancedRobot::getY()
{
  link->sendAndProcess(GetYReq(), this);
  return yPosition;
}

double BZAdvancedRobot::getZ()
{
  link->sendAndProcess(GetZReq(), this);
  return zPosition;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
