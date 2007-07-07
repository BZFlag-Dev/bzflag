#include "Frontend.h"

#include "TimeKeeper.h"
#include "RCMessageFactory.h"

#include <unistd.h>

bool Frontend::run(const char *host, int port)
{
    return true;
    pid_t pid = fork();
    if (pid < 0)
        return false;
    else if (pid > 0)
        return true;

    Frontend frontend(host, port);

    while (true)
    {
        frontend.update();
        TimeKeeper::sleep(0.01);
    }

    /* Yeah, right. */
    return true;
}

Frontend::Frontend(const char *host, int port)
{
    RCMessageFactory<RCReply>::initialize();
    link = new RCLinkFrontend(host, port);
    RCREPLY.setLink(link);
}

void Frontend::update()
{
    link->update();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

