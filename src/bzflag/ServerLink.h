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
 * Encapsulates communication between local player and the server.
 */

#ifndef BZF_SERVER_LINK_H
#define BZF_SERVER_LINK_H

#include "common.h"
#include "global.h"
#include "Address.h"
#include "Protocol.h"
#include "ShotPath.h"
#include "Flag.h"

struct PacketQueue {
public:
	unsigned short seqno;
	void *data;
	int length;
	struct PacketQueue *next;
};

class ServerLink {
public:
	enum State {
					Okay = 0,
					SocketError = 1,
					Rejected = 2,
					BadVersion = 3,
					Hungup = 4,				// only used by Winsock
					CrippledVersion = 5
	};

	ServerLink(const Address& serverAddress, int port = ServerPort);
	~ServerLink();

	State				getState() const;
	int					getSocket() const;		// file descriptor actually
	const PlayerId&		getId() const;
	const char*			getVersion() const;
	void				enableUDPCon();

	void				send(uint16_t code, uint16_t len, const void* msg);
	// if millisecondsToBlock < 0 then block forever
	int					read(uint16_t& code, uint16_t& len, void* msg,
							int millisecondsToBlock = 0);

	void				sendEnter(PlayerType, TeamColor,
							const char* name, const char* email);
	void				sendPlayerUpdate(const Player*);
	void				sendCaptureFlag(TeamColor);
	void				sendGrabFlag(int flagIndex);
	void				sendDropFlag(FlagDropReason reason, const float* position);
	void				sendKilled(const PlayerId&, int shotId);
	void				sendBeginShot(const FiringInfo&);
	void				sendEndShot(const PlayerId&, int shotId, int reason);
	void				sendAlive(const float* pos, const float* fwd);
	void				sendTeleport(int from, int to);
	void				sendNewScore(int wins, int losses);

	void*				getPacketFromServer(uint16_t* length, uint16_t* seqno);
	void				enqueuePacket(int op, int rseqno, void *msg, int n);
	void				disqueuePacket(int op, int rseqno);
	void* 				assembleSendPacket(uint32_t *length);
	void* 				assembleCDPacket(uint32_t* length);
	void 				disassemblePacket(void *msg, int *numpackets);

	static ServerLink*	getServer(); // const
	static void			setServer(ServerLink*);

	void				sendClientVersion();
private:
	State				state;
	int					tcpfd;
	int					udpfd;
	struct sockaddr		servaddr;

	PlayerId			id;
	char				version[9];
	static ServerLink*	server;
	int					server_abilities;

	struct PacketQueue  *uqueue;
	struct PacketQueue  *dqueue;
	unsigned short      lastRecvPacketNo;
	unsigned short      currentRecvSeq;
	unsigned short 		lastSendPacketNo;
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

inline int				ServerLink::getSocket() const
{
	return tcpfd;
}

inline const PlayerId&	ServerLink::getId() const
{
	return id;
}

inline const char*		ServerLink::getVersion() const
{
	return version;
}

#endif // BZF_SERVER_LINK_H
// ex: shiftwidth=4 tabstop=4
