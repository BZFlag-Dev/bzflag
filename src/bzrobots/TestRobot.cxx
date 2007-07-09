#include "TestRobot.h"

#include "TimeKeeper.h"

void TestRobot::run()
{
    while (true)
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
}
