#include "TestRobot.h"
#include "TimeKeeper.h"
#include "Logger.h"

extern "C" {
    BZAdvancedRobot *create() { return new TestRobot(); }
    void destroy(BZAdvancedRobot *robot) { delete robot; }
}

void TestRobot::initialize()
{
    setCompatability(false);
}

void TestRobot::update()
{
    setFire();
    setAhead(20);
    do {
        if (getGunHeat() <= 0)
            setFire();
        execute();
    } while (getDistanceRemaining() > 0);

    setTurnLeft(90);
    do {
        execute();
    } while (getTurnRemaining() > 0);
}

void TestRobot::onHitWall(const HitWallEvent &/*hwe*/)
{
    FRONTENDLOGGER << "Got a hitWallEvent, stopping!" << std::endl;
    setAhead(0);
}
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
