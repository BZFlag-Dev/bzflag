/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Encapsulates communication between local player and the server.
 */

#ifndef	BZF_SERVER_LINK_H
#define	BZF_SERVER_LINK_H

#include "common.h"
#include "global.h"
#include "Address.h"
#include "Protocol.h"
#include "ShotPath.h"

class ServerLink {
  public:
    enum State {
			Okay = 0,
			SocketError = 1,
			Rejected = 2,
			BadVersion = 3,
			Hungup = 4,		// only used by Winsock
			CrippledVersion = 5
    };

    enum Abilities {
			Nothing = 0,
			CanDoUDP = 1,
			SendScripts = 2,
			SendTextures = 4,
			HasMessageLink = 8
    };

			ServerLink(const Address& serverAddress,
					int port = ServerPort, int number = 0);
			~ServerLink();

    State		getState() const;
    int			getSocket() const;	// file descriptor actually
    const PlayerId&	getId() const;
    const char*		getVersion() const;

    void		send(uint16_t code, uint16_t len, const void* msg);
    // if millisecondsToBlock < 0 then block forever
    int			read(uint16_t& code, uint16_t& len, void* msg,
						int millisecondsToBlock = 0);

    void		sendEnter(PlayerType, TeamColor,
					const char* name, const char* email);
    void		sendCaptureFlag(TeamColor);
    void		sendGrabFlag(int flagIndex);
    void		sendDropFlag(const float* position);
    void		sendKilled(const PlayerId&, int reason, int shotId);
    void		sendBeginShot(const FiringInfo&);
    void		sendEndShot(const PlayerId&, int shotId, int reason);
    void		sendAlive(const float* pos, const float* fwd);
    void		sendTeleport(int from, int to);
    void		sendNewScore(int wins, int losses);
    void		sendNewRabbit();
    void                sendUDPlinkRequest();
    void		enableUDPCon();

    static ServerLink*	getServer(); // const
    static void		setServer(ServerLink*);
    void		setUDPRemotePort(unsigned short port);

  private:
    State		state;
    int			fd;

    uint32_t		remoteAddress;
    int			usendfd;
    struct sockaddr     usendaddr;
    int			urecvfd;
    struct sockaddr     urecvaddr;
    bool		ulinkup;

    PlayerId		id;
    char		version[9];
    static ServerLink*	server;
    int			server_abilities;
};

#define SEND 1
#define RECEIVE 0

//
// ServerLink
//

inline ServerLink::State ServerLink::getState() const
{
  return state;
}

inline int		ServerLink::getSocket() const
{
  return fd;
}

inline const PlayerId&	ServerLink::getId() const
{
  return id;
}

inline const char*	ServerLink::getVersion() const
{
  return version;
}

#endif // BZF_SERVER_LINK_H
// ex: shiftwidth=2 tabstop=8
