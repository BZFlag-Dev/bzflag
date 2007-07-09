#include "TestRobot.h"

#include "TimeKeeper.h"

void TestRobot::run()
{
    while (true)
    {
        setFire();
        execute();
        TimeKeeper::sleep(getGunHeat());
        setFire();
        setAhead(100);
        while (getDistanceRemaining() > 0)
        {
            if (getGunHeat() <= 0)
                setFire();
            execute();
        }
        setTurnLeft(90);
        while (getTurnRemaining() > 0)
            execute();
    }
}
