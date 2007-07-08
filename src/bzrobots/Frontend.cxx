#include "Frontend.h"

#include "TimeKeeper.h"
#include "RCMessageFactory.h"

#include <unistd.h>

#include <iostream>

bool Frontend::run(const char *host, int port)
{
  pid_t pid = fork();
  if (pid < 0)
    return false;
  else if (pid > 0)
    return true;

  Frontend frontend;
  if (!frontend.connect(host, port))
  {
    std::cout << "Frontend failed to connect! Bailing! (" << frontend.getError() << ")" << std::endl;
    return false;
  }

  std::cout << "Frontend initialized, " << host << ":" << port << std::endl;
  while (frontend.update())
  {
    TimeKeeper::sleep(0.01);
  }

  std::cout << "Frontend disconnected / failed! (" << frontend.getError() << ")" << std::endl;
  return false;
}

Frontend::Frontend() :sentStuff(false)
{
  RCMessageFactory<RCReply>::initialize();
}
bool Frontend::connect(const char *host, int port)
{
  link = new RCLinkFrontend();
  RCREPLY.setLink(link);
  return link->connect(host, port);
}

bool Frontend::update()
{
  if (!link->update())
    return false;
  if (link->getStatus() == RCLink::Connected && !sentStuff)
  {
    TimeKeeper::sleep(2.0);
    link->send(SetFireReq());
    link->send(ExecuteReq());
    link->send(GetGunHeatReq());
    std::cout << "[OK] GetGunHeat 0" << std::endl;

    link->waitForReply("GetGunHeat");
    std::cout << "[OK] Reply" << std::endl;

    RCReply *reply;
    while ((reply = link->popReply()))
      std::cout << "Got message: " << reply->asString() << std::endl;
    sentStuff = true;
  }
  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

