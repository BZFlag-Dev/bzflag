/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* system implementation headers */
#include <ostream>
#include <sstream>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

/* common implementation headers */
#include "network.h"
#include "NetHandler.h"

/* BZRobots API Header */
#include "AdvancedRobot.h"

#define BZROBOTS_PROTO_VERSION    "0001"

#define RC_LINK_RECVBUFLEN 100000
#define RC_LINK_SENDBUFLEN 100000
#define RC_LINK_MAXARGS 50
#define RC_LINK_OVERFLOW_MSG "\nerror Connection Stalled.  RC stopped reading data!\n"

#define RC_LINK_NOIDENTIFY_MSG "error IdentifyFrontend expected\n"
#define RC_LINK_IDENTIFY_STR "IdentifyBackend "

#define PROTOCOL_DEBUG

using namespace BZRobots;

/**
 * RCLinkBot: RCLink Emulator for backwards compatability with the BYU project
 */
class RCLinkBot : public AdvancedRobot {
  public:

    typedef enum {
      Disconnected,
      SocketError,
      Listening,
      Connecting,
      Connected
    } State;

    RCLinkBot() {}
    ~RCLinkBot() {}

  private:
    State status;
    int listenfd, connfd;
    char recvbuf[RC_LINK_RECVBUFLEN];
    char sendbuf[RC_LINK_SENDBUFLEN];
    int recv_amount, send_amount;
    bool input_toolong, output_overflow;
    std::string error;

    void startListening(int port);
    bool tryAccept();
    int updateWrite();
    int updateRead();
    //int updateParse(int maxlines = 0);
    bool waitForData();
    bool sendBuffer(const char* message);
    void sendPacket(const char* data, unsigned int size, bool killit = false);
    void update();

  public:
    void run();

    void onBattleEnded(const BattleEndedEvent& e);
    void onBulletHit(const BulletHitEvent& e);
    void onBulletMissed(const BulletMissedEvent& e);
    void onDeath(const DeathEvent& e);
    void onHitByBullet(const HitByBulletEvent& e);
    void onHitWall(const HitWallEvent& e);
    void onRobotDeath(const RobotDeathEvent& e);
    void onScannedRobot(const ScannedRobotEvent& e);
    void onSpawn(const SpawnEvent& e);
    void onStatus(const StatusEvent& e);
    void onWin(const WinEvent& e);
};

extern "C" {
  AdvancedRobot* create() {
    return new RCLinkBot();
  }
  void destroy(AdvancedRobot* robot) {
    delete robot;
  }
}

void RCLinkBot::startListening(int port) {
  struct sockaddr_in sa;

  if (status != Disconnected) {
    return;
  }

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  //sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    return;
  }

  /* Used so we can re-bind to our port while a previous connection is
   * still in TIME_WAIT state.
   */
  const char reuse_addr = 1;

  setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

  if (bind(listenfd, (sockaddr*)&sa, sizeof(sa)) == -1) {
    close(listenfd);
    return;
  }

  if (listen(listenfd, 1) == -1) {
    close(listenfd);
    return;
  }

  /*
  BzfNetwork::setNonBlocking(listenfd);
  setNoDelay(listenfd);
  */

  status = Listening;
}


bool RCLinkBot::tryAccept() {
  if (status != Listening) {
    error = "Cannot accept when not listening!";
    return false;
  }
  // O_NONBLOCK is set so we'll probably return immediately.
  connfd = accept(listenfd, NULL, 0);
  if (connfd == -1) {
    error = strerror(errno);
    return false;
  }

  /*
  BzfNetwork::setNonBlocking(connfd);
  setNoDelay(connfd);
  */

  status = Connecting;
  send_amount = 0;
  recv_amount = 0;
  input_toolong = false;

  std::cout << "RCLinkBot: Accepted a new connection." << std::endl;

  sendPacket(RC_LINK_IDENTIFY_STR, (unsigned int)strlen(RC_LINK_IDENTIFY_STR));
  sendPacket(BZROBOTS_PROTO_VERSION, (unsigned int)strlen(BZROBOTS_PROTO_VERSION));
  sendPacket("\n", 1);
  return true;
}

/*
 * Send as much data as possible from our outgoing buffer.
 */
int RCLinkBot::updateWrite() {
  char* bufptr = sendbuf;
  int prev_send_amount = send_amount;

  if (status != Connected && status != Connecting) {
    return -1;
  }

  if (output_overflow) {
    int errorlen = strlen(RC_LINK_OVERFLOW_MSG);
    if (send_amount + errorlen <= RC_LINK_SENDBUFLEN) {
      // The output buffer has some space--we can recover.
      memcpy(sendbuf + send_amount, RC_LINK_OVERFLOW_MSG, errorlen);
      send_amount += errorlen;
      output_overflow = false;
      std::cout << "RCLink: Clearing output_overflow." << std::endl;
    }
    else {
      std::cout << "RCLink: Couldn't fix overflow. errorlen = " << errorlen
                << ", send_amount = " << send_amount << std::endl;
    }
  }

  while (true) {
    if (send_amount == 0) {
      break;
    }

    int nwritten = send(connfd, bufptr, send_amount, 0);
    if (nwritten == -1 && errno == EAGAIN) {
      break;
    }
    else if (nwritten == -1) {
      std::cout << "RCLink: Write failed. Disconnecting. Error: " << strerror(errno) << std::endl;
      status = SocketError;
      return -1;
    }
    else {
      bufptr += nwritten;
      send_amount -= nwritten;
    }
  }

  if (bufptr != sendbuf && send_amount > 0) {
    memmove(sendbuf, bufptr, send_amount);
  }

  return prev_send_amount - send_amount;
}

/*
 * Fill up the receive buffer with any available incoming data.  Return the
 * number of bytes of data read or -1 if the connection has died.
 */
int RCLinkBot::updateRead() {
  int prev_recv_amount = recv_amount;

  if (status != Connected && status != Connecting) {
    return -1;
  }

  // read in as much data as possible
  while (true) {
    if (recv_amount == RC_LINK_RECVBUFLEN) {
      break;
    }

    int nread = recv(connfd, recvbuf + recv_amount, RC_LINK_RECVBUFLEN - recv_amount, 0);
    if (nread == 0) {
      std::cout << "RCLink: Remote host closed connection." << std::endl;
      status = Listening;
      return -1;
    }
    else if (nread == -1 && errno != EAGAIN) {
      std::cout << "RCLink: Read failed. Error: " << strerror(errno) << std::endl;
      status = SocketError;
      return -1;
    }
    else if (nread == -1) {
      // got no data (remember, read is set to be nonblocking)
      break;
    }
    else {
      recv_amount += nread;
    }
  }

  return recv_amount - prev_recv_amount;
}

/*
 * Parse as many objects as possible (via parseCommand).
 * Return the number created.
 */
/*
int RCLinkBot::updateParse(int maxlines)
{
  int ncommands = 0;
  char *bufptr = recvbuf;
  char *newline;

  if (recv_amount == 0) {
    return 0;
  }

  while (true) {
    // Sometimes a remote agent will add unnecessary null characters after a
    // newline.  Drop them:
    while (recv_amount >= 1 && *bufptr == '\0') {
      bufptr++;
      recv_amount--;
    }
    if (recv_amount == 0) {
      break;
    }

    newline = (char *)memchr(bufptr, '\n', recv_amount);
    if (newline == NULL) {
      if (input_toolong) {
        // We're throwing out everything up to the next newline.
        recv_amount = 0;
        break;
      } else {
        // We need to read more before we can do anything.
        break;
      }
    } else {
      // We have a full input line.
      recv_amount -= newline - bufptr + 1;

      if (input_toolong) {
        input_toolong = false;
      } else {
        if (*bufptr == '\n' || (*bufptr == '\r' && *(bufptr+1) == '\n')) {
          // empty line: ignore
        } else {
          *newline = '\0';
          if (parseCommand(bufptr)) {
            ncommands++;
          }
        }
      }

      bufptr = newline + 1;

      if (maxlines == 1) {
        break;
      }
    }
  }

  if (bufptr != recvbuf && recv_amount > 0) {
    memmove(recvbuf, bufptr, recv_amount);
  }

  if (recv_amount == RC_LINK_RECVBUFLEN) {
    input_toolong = true;
  std::cout << "RCLink: Input line too long. Discarding." << std::endl;
    recv_amount = 0;
  }

  return ncommands;
}
*/

bool RCLinkBot::sendBuffer(const char* message) {
  unsigned int messagelen = (unsigned int)strlen(message);

  if (output_overflow) {
    error = "Output overflow! (more data than buffer can take)";
    return false;
  }

  if (send_amount + messagelen > RC_LINK_SENDBUFLEN) {
    std::cout << "RCLink: Setting output_overflow." << std::endl;
    error = "Output overflow! (more data than the buffer can take)";
    output_overflow = true;
    return false;
  }

  memcpy(sendbuf + send_amount, message, messagelen);
#ifdef PROTOCOL_DEBUG
  std::cout << "[  send] " << (sendbuf + send_amount); std::cout.flush();
#endif
  send_amount += messagelen;

  updateWrite();
  return true;
}

void RCLinkBot::sendPacket(const char* data, unsigned int size, bool killit) {
  send(connfd, data, size, 0);
  if (killit) {
    close(connfd);
  }
}

void RCLinkBot::update() {
  if (status != Connected && status != Connecting) {
    return;
  }

  updateWrite();
  int amount = updateRead();

  if (amount == -1) {
    status = Listening;
    return;
  }

  if (status == Connected) {
    //updateParse();
  }
  else if (status == Connecting) {
    int ncommands = 0;//updateParse(1);
    if (ncommands) {
      /*
        RCRequest *req = popRequest();
        if (req && req->getType() == "IdentifyFrontend") {
          status = Connected;
        } else {
      std::cout << "RCLink: Expected an 'IdentifyFrontend'." << std::endl;
      sendPacket(RC_LINK_NOIDENTIFY_MSG, (unsigned int)strlen(RC_LINK_NOIDENTIFY_MSG),true);
          status = Listening;
        }
      */
    }
  }
}


void RCLinkBot::run() {
  startListening(1234);
  while (true) {
    update();
    doNothing();
  }
}

void RCLinkBot::onBattleEnded(const BattleEndedEvent& /*e*/) {
}

void RCLinkBot::onBulletHit(const BulletHitEvent& /*e*/) {
}

void RCLinkBot::onBulletMissed(const BulletMissedEvent& /*e*/) {
}

void RCLinkBot::onDeath(const DeathEvent& /*e*/) {
}

void RCLinkBot::onHitByBullet(const HitByBulletEvent& /*e*/) {
}

void RCLinkBot::onHitWall(const HitWallEvent& /*e*/) {
}

void RCLinkBot::onRobotDeath(const RobotDeathEvent& /*e*/) {
}

void RCLinkBot::onScannedRobot(const ScannedRobotEvent& /*e*/) {
}

void RCLinkBot::onSpawn(const SpawnEvent& /*e*/) {
}

void RCLinkBot::onStatus(const StatusEvent& /*e*/) {
}

void RCLinkBot::onWin(const WinEvent& /*e*/) {
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
