#include "RCFrontend.h"

#include "TimeKeeper.h"

#include <unistd.h>

bool RCFrontend::run(const char *host, int port)
{
    pid_t pid = fork();
    if (pid < 0)
        return false;
    else if (pid > 0)
        return true;

    RCFrontend rcFrontend(host, port);

    while (true)
    {
        rcFrontend.update();
        TimeKeeper::sleep(0.01);
    }

    /* Yeah, right. */
    return true;
}

RCFrontend::RCFrontend(const char *host, int port)
{
    link = new RCLinkFrontend(host, port);
}

void RCFrontend::update()
{
    link->update();
}
