/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * PingPacket:
 *	Encapsulates server `ping' info.
 */

#ifndef BZF_PING_H
#define BZF_PING_H

#include "Address.h"
#include "Pack.h"
#include "multicast.h"

static const int		PingPacketHexPackedSize = 2 * 2 * 18;

class PingPacket {
public:
	PingPacket();
	~PingPacket();

	bool				read(int fd, struct sockaddr_in*);
	bool				write(int fd, const struct sockaddr_in*) const;
	bool				waitForReply(int fd, const Address& from,
							int millisecondsToBlock = 0);

	void*				pack(void*, const char* version) const;
	void*				unpack(void*, char* version);

	void				packHex(char*) const;
	void				unpackHex(char*);
	static void			repackHexPlayerCounts(char*, int* counts);

	static bool			isRequest(int fd, struct sockaddr_in*,
							int* minReplyTTL = NULL);
	static bool			sendRequest(int fd, const struct sockaddr_in*,
							int minReplyTTL = 0);

public:
	ServerId			serverId;
	Address				sourceAddr;
	uint16_t			gameStyle;
	uint16_t			maxPlayers;
	uint16_t			maxShots;
	uint16_t			rogueCount;
	uint16_t			redCount;
	uint16_t			greenCount;
	uint16_t			blueCount;
	uint16_t			purpleCount;
	uint16_t			rogueMax;
	uint16_t			redMax;
	uint16_t			greenMax;
	uint16_t			blueMax;
	uint16_t			purpleMax;
	uint16_t			shakeWins;
	uint16_t			shakeTimeout;			// 1/10ths of second
	uint16_t			maxPlayerScore;
	uint16_t			maxTeamScore;
	uint16_t			maxTime;				// seconds

private:
	static int			hex2bin(char);
	static char			bin2hex(int);
	static char*		packHex16(char*, uint16_t);
	static char*		unpackHex16(char*, uint16_t&);

private:
	static const int	PacketSize;
};

#endif // BZF_PING_H
