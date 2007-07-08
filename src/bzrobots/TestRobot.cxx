#include "TestRobot.h"

#include "TimeKeeper.h"

void TestRobot::run()
{
    while (true)
    {
        setFire();
        execute();
        TimeKeeper::sleep(0.5);
        std::cout << "Heat: " << getGunHeat() << std::endl;
    }
}
