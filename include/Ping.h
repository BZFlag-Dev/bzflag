/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__PING_H__
#define	__PING_H__

#include "common.h"

/* system headers */
#include <fstream>

/* common interface headers */
#include "Address.h"
#include "Pack.h"
#include "multicast.h"


// 8 uint16's and 13 uint8's hex encoded
static const int	PingPacketHexPackedSize = 4 * 8 + 2 * 13;


/** PingPacket:
 *	Encapsulates server `ping' info.
 */
class PingPacket {
  public:
			PingPacket();
			~PingPacket();

    bool		read(int fd, struct sockaddr_in*);
    bool		write(int fd, const struct sockaddr_in*) const;
    bool		waitForReply(int fd, const Address& from,
				int millisecondsToBlock = 0);

    void*		pack(void*, const char* version) const;
    void*		unpack(void*, char* version);

    void		packHex(char*) const;
    void		unpackHex(char*);
    void		zeroPlayerCounts();
    void		writeToFile(std::ostream& out) const;
    bool		readFromFile(std::istream& in);
    static void		repackHexPlayerCounts(char*, int* counts);

    static bool	isRequest(int fd, struct sockaddr_in*);
    static bool	sendRequest(int fd, const struct sockaddr_in*);

  public:
    ServerId		serverId;
    Address			sourceAddr;
	uint16_t		gameType;
    uint16_t		gameOptions;
    uint16_t		maxShots;
    uint16_t		shakeWins;
    uint16_t		shakeTimeout;		// 1/10ths of second
    uint16_t		maxPlayerScore;
    uint16_t		maxTeamScore;
    uint16_t		maxTime;		// seconds
    uint8_t		maxPlayers;
    uint8_t		rogueCount;
    uint8_t		rogueMax;
    uint8_t		redCount;
    uint8_t		redMax;
    uint8_t		greenCount;
    uint8_t		greenMax;
    uint8_t		blueCount;
    uint8_t		blueMax;
    uint8_t		purpleCount;
    uint8_t		purpleMax;
    uint8_t		observerCount;
    uint8_t		observerMax;

  private:
    static int		hex2bin(char);
    static char		bin2hex(int);
    static char*	packHex16(char*, uint16_t);
    static char*	unpackHex16(char*, uint16_t&);
    static char*	packHex8(char*, uint8_t);
    static char*	unpackHex8(char*, uint8_t&);

  private:
    static const int	PacketSize;
};


#endif  /* __PING_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
