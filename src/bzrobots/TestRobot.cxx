/* Compile using:
 * g++ -shared -fPIC -O2 -g -I. -I. -I../../include  -I../../include -I../bzflag -Wall -W TestRobot.cxx -o TestRobot.so
 */

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

void TestRobot::onHitWall(const HitWallEvent &hwe)
{
    FRONTENDLOGGER << "Got a hitWallEvent, stopping!" << std::endl;
    setAhead(0);
}
