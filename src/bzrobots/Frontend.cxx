#include "Frontend.h"

#include "TimeKeeper.h"
#include "RCMessageFactory.h"
#include "TestRobot.h"

#include "bzsignal.h"

#include <iostream>
#include <unistd.h>

bool Frontend::run(const char *host, int port)
{
  pid_t pid = fork();
  if (pid < 0)
    return false;
  else if (pid > 0)
    return true;

  fclose(stdin); // Shouldn't mess around with that here ;-)
  bzSignal(SIGINT, SIG_DFL);

  Frontend frontend;
  if (!frontend.connect(host, port))
  {
    std::cout << "Frontend failed to connect! Bailing! (" << frontend.getError() << ")" << std::endl;
    return false;
  }

  std::cout << "Frontend initialized, " << host << ":" << port << std::endl;
  frontend.start();
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

void Frontend::start()
{
  while (link->getStatus() != RCLink::Connected)
  {
    link->update();
    TimeKeeper::sleep(0.5);
  }

  TestRobot bot(link);
  bot.run();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

