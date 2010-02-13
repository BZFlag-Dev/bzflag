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

/*
 * Encapsulates communication between local player and the server.
 */

#ifndef	BZF_SERVER_LINK_H
#define	BZF_SERVER_LINK_H


#include "common.h"

#include <string>
#include "global.h"
#include "network.h"
#include "Address.h"
#include "Protocol.h"
#include "Flag.h"
#include "ShotPath.h"
#include "BufferedNetworkMessage.h"
#include "vectors.h"

class ShotPath;


class ServerLink {
public:
  enum State {
    Okay = 0,
    SocketError = 1,
    Rejected = 2,
    BadVersion = 3,
    Hungup = 4,		// only used by Winsock
    CrippledVersion = 5,
    Refused = 6
  };

  enum Abilities {
    Nothing = 0,
    CanDoUDP = 1,
    SendScripts = 2,
    SendTextures = 4,
    HasMessageLink = 8
  };

  ServerLink(const std::string& serverName,
             const Address& serverAddress,
	     int port = ServerPort);
  ~ServerLink();

  inline State           getState()   const { return state; }
  inline int             getSocket()  const { return fd; } // file descriptor actually
  inline const PlayerId& getId()      const { return id; }
  inline const char*     getVersion() const { return version; }

  inline const std::string& getJoinServer()   const { return joinServer;   }
  inline int                getJoinPort()     const { return joinPort;     }
  inline const std::string& getJoinCallsign() const { return joinCallsign; }

  inline const std::string& getRejectionMessage() { return rejectionMessage; }

  void send(uint16_t code, uint16_t len, const void* msg);

  // if millisecondsToBlock < 0 then block forever
  int read(uint16_t& code, uint16_t& len, void* msg, int millisecondsToBlock = 0);
  int read(BufferedNetworkMessage *msg, int millisecondsToBlock = 0);

  bool readEnter(std::string& reason, uint16_t& code, uint16_t& rejcode);

  void sendCaps(PlayerId id, bool downloads, bool sounds );
  void sendEnter(PlayerId, PlayerType, NetworkUpdates, TeamColor,
                 const char* name, const char* token, const char* referrer);

  void sendCaptureFlag(TeamColor);
  void sendDropFlag(const fvec3& position);
  void sendKilled(const PlayerId victim, const PlayerId shooter,
                  int reason, int shotId, const FlagType* flag, int phydrv);
  // FIXME -- This is very ugly, but required to build bzadmin with gcc 2.9.5.
  //	  It should be changed to something cleaner.
#ifndef BUILDING_BZADMIN
  void sendPlayerUpdate(Player*);
  void sendBeginShot(const FiringInfo&);
#endif
  void sendEndShot(const PlayerId&, int shotId, int reason);

  void sendHit(const PlayerId &source, const PlayerId &shooter, int shotId);
  void sendVarRequest();

  void sendAlive(const PlayerId playerId);
  void sendTeleport(int from, int to);
  void sendShotInfo(const ShotPath& shotPath, char infoType, const fvec3& pos,
                    uint32_t obstacleGUID = (uint32_t)-1,
                    int linkSrcID = -1, int linkDstID = -1);
  void sendTransferFlag(const PlayerId&, const PlayerId&);
  void sendNewRabbit();
  void sendPaused(bool paused);
  void sendNewPlayer(int botID, TeamColor team);

  void sendExit();
  void sendAutoPilot(bool autopilot);
  void sendMessage(const PlayerId& to, const char message[MessageLen]);
  void sendLagPing(char pingRequest[]);
  void sendUDPlinkRequest();

  void sendOSVersion(const PlayerId player, const std::string &vers);

  void sendCustomData(const std::string &key, const std::string &value);
  void sendCustomData(const char* key, const std::string &value ) {
    if (key != NULL) { sendCustomData(std::string(key), value); }
  }

  static ServerLink*	getServer(); // const
  static void		setServer(ServerLink*);

  void enableOutboundUDP();
  void confirmIncomingUDP();

  void flush();

private:
  State		state;
  int		fd;

  struct sockaddr	usendaddr;
  int			urecvfd;
  struct sockaddr	urecvaddr; // the clients udp listen address
  bool			ulinkup;

  PlayerId		id;
  char			version[9];
  static ServerLink*	server;
  int			server_abilities;

  std::string	rejectionMessage;

  int		udpLength;
  char	       *udpBufferPtr;
  char		ubuf[MaxPacketLen];

  bool oldNeedForSpeed;
  unsigned int previousFill;
  char txbuf[MaxPacketLen];

  std::string joinServer;
  int         joinPort;
  std::string joinCallsign;
};


#endif // BZF_SERVER_LINK_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
