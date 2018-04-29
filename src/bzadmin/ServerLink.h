/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "Address.h"
#include "Protocol.h"
#include "Flag.h"

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

			ServerLink(const Address& serverAddress,
					int port = ServerPort);
			~ServerLink();

    State		getState() const;
    const std::string&	getRejectionMessage() { return rejectionMessage; }
    int			getSocket() const;	// file descriptor actually
    const PlayerId&	getId() const;
    const char*		getVersion() const;

    void		send(uint16_t code, uint16_t len, const void* msg);
    // if millisecondsToBlock < 0 then block forever
    int			read(uint16_t& code, uint16_t& len, void* msg,
						int millisecondsToBlock = 0);

    void		sendEnter(PlayerType, TeamColor,
				  const char* name, const char* motto, const char* token);
    bool		readEnter(std::string& reason,
				  uint16_t& code, uint16_t& rejcode);

    static ServerLink*	getServer(); // const
    static void		setServer(ServerLink*);
    void		enableOutboundUDP();
    void		confirmIncomingUDP();

  private:
    State		state;
    int			fd;

    struct sockaddr	usendaddr;
    int			urecvfd;
    struct sockaddr	urecvaddr; // the clients udp listen address
    bool		ulinkup;

    PlayerId		id;
    char		version[9];
    static ServerLink*	server;
    int			server_abilities;

    std::string		rejectionMessage;

    int			udpLength;
    const char*		udpBufferPtr;
    char		ubuf[MaxPacketLen];
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
