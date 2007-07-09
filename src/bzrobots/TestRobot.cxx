#include "TestRobot.h"

#include "TimeKeeper.h"

void TestRobot::run()
{
    while (true)
    {
        setFire();
        execute();
        std::cout << "Heat: " << getGunHeat() << std::endl;
    }
}
