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
 * Encapsulates communication between local player and other players.
 */

#ifndef	BZF_PLAYER_LINK_H
#define	BZF_PLAYER_LINK_H

#include "common.h"
#include "global.h"
#include "Address.h"
#include "Protocol.h"
#include "multicast.h"
#include "udpnetwork.h"

class Player;
class ServerLink;

class PlayerLink {
  public:
    enum State {
			Okay = 0,
			SocketError = 1,
			ServerUDPRelay = 2,
			ServerRelay = 3
    };

			PlayerLink(const Address& multicastAddress,
					int port = BroadcastPort,
					int ttl = 8,
					const char* net_interface = NULL);
			~PlayerLink();

    State		getState() const;
    int			getInSocket() const;	// file descriptor actually
    int			getOutSocket() const;	// file descriptor actually

    int			getTTL() const { return ttl; }
    void		setTTL(int ttl);

    // if millisecondsToBlock < 0 then block forever
    int			read(uint16_t& code, uint16_t& len, void* msg,
						int millisecondsToBlock = 0);


    static PlayerLink*	getMulticast(); // const
    static void		setMulticast(PlayerLink*);
    void		setPortForUPD(unsigned short port);
    void		enableUDPConIfRelayed();
  private:
    State		state;
    int			ttl;
    int			fdIn;
    int			fdOut;
    struct sockaddr_in	inAddr;
    struct sockaddr_in	outAddr;
    static PlayerLink*	multicast;
};

//
// PlayerLink
//

inline PlayerLink::State PlayerLink::getState() const
{
  return state;
}

inline int		PlayerLink::getInSocket() const
{
  return fdIn;
}

inline int		PlayerLink::getOutSocket() const
{
  return fdOut;
}

#endif // BZF_PLAYER_LINK_H
// ex: shiftwidth=2 tabstop=8
