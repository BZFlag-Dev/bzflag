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
static const char copyright[] = "Copyright (c) 1993 - 2002 Tim Riker";

// to enforce a game time limit
#define TIMELIMIT
// to dump score info to stdout
//#define PRINTSCORE to include code to dump score info to stdout

// Like verbose debug messages?
#define DEBUG1 if (debug >= 1) printf
#define DEBUG2 if (debug >= 2) printf
#define DEBUG3 if (debug >= 3) printf
#define DEBUG4 if (debug >= 4) printf

#define SERVERLOGINMSG true

const int MaxPlayers = 40;
const int MaxShots = 10;
const int udpBufSize = 128000;

#if defined(__sgi)
#define FD_SETSIZE (MaxPlayers + 10)
#endif /* defined(__sgi) */

// must be before windows.h
#include "network.h"

#if defined(_WIN32)
#include <windows.h>
#endif /* defined(_WIN32) */

#include <stdio.h>
#if !defined(_WIN32)
#include <fcntl.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include "PlatformFactory.h"
#include <time.h>
#include "common.h"
#include "global.h"
#include "Protocol.h"
#include "Address.h"
#include "Pack.h"
#include "TimeKeeper.h"
#include "Flag.h"
#include "Team.h"
#include "Ping.h"
#include "TimeBomb.h"
#include "RegionManagerFlagSpawn.h"
#include "RegionReaderFlagSpawn.h"
#include "ConfigFileManager.h"
#include <sstream>

#include "WorldInfo.h"
#include "WorldFile.h"
#include "WorldStandardTeam.h"
#include "WorldStandardRandom.h"

#include "RegionManagerBase.h"
#include "RegionManagerFlagSpawn.h"
#include "RegionManagerObstacle.h"

#include "TransformedShape.h"
#include "ShapeBox.h"

// every ListServerReAddTime server add ourself to the list
// server again.  this is in case the list server has reset
// or dropped us for some reason.
static const float ListServerReAddTime = 30.0f * 60.0f;

// maximum number of list servers to advertise ourself to
static const int MaxListServers = 5;

static const float FlagHalfLife = 45.0f;
// do NOT change
static const int NotConnected = -1;
static const PlayerId InvalidPlayer = 0xff;
static const PlayerId ServerPlayer = 253; // FIXME - enum maybe?

// custom server login message
static char *servermsg = NULL;

enum PlayerState {
	PlayerNoExist, // does not exist
	PlayerAccept, // got connect, sending hello
	PlayerInLimbo, // not entered
	PlayerDead, // dead
	PlayerAlive // alive
};

struct PacketQueue {
	public:
		unsigned short seqno;
		void *data;
		int length;
		struct PacketQueue *next;
};

#ifdef DEBUG
#define NETWORK_STATS
#endif
#ifdef NETWORK_STATS
struct MessageCount {
	public:
		uint32_t count; 
		uint16_t code;
		uint16_t maxSize;
};
// does not include MsgNull
#define MessageTypes 38
#endif
struct PlayerInfo {
	public:
		// time accepted
		TimeKeeper time;
		// socket file descriptor
		int fd;
		// current state of player
		PlayerState state;
		// type of player
		PlayerType type;
		// player's pseudonym
		char callSign[CallSignLen];
		// player's email address
		char email[EmailLen];
		// player's team
		TeamColor team;
		// flag index player has
		int flag;
		// player's score
		int wins, losses;

		// input buffers
		// bytes read in current msg
		int tcplen;
		// current TCP msg
		char tcpmsg[MaxPacketLen];
		// bytes read in current msg
		int udplen;
		// current UDP msg
		char udpmsg[MaxPacketLen];

		// output buffer
		int outmsgOffset;
		int outmsgSize;
		int outmsgCapacity;
		char *outmsg;

		// UDP connection
		bool ulinkup;
		struct sockaddr_in uaddr;
		// TCP connection
		struct sockaddr_in taddr;

		// UDP message queue
		struct PacketQueue *uqueue;
		struct PacketQueue *dqueue;
		unsigned short lastRecvPacketNo;
		unsigned short lastSendPacketNo;

		bool toBeKicked;

		bool Admin;

		// lag measurement
		bool lagkillerpending;
		TimeKeeper lagkillertime;
		float lagavg,lagalpha;
		int lagcount,laglastwarn,lagwarncount;
#ifdef NETWORK_STATS
		// message stats bloat
		TimeKeeper perSecondTime[2];
		uint32_t perSecondCurrentBytes[2];
		uint32_t perSecondMaxBytes[2];
		uint32_t perSecondCurrentMsg[2];
		uint32_t perSecondMaxMsg[2];
		uint32_t msgBytes[2];
		struct MessageCount msg[2][MessageTypes];
#endif
};

#define SEND 1
#define RECEIVE 0

struct FlagInfo {
	public:
		// flag info
		Flag flag;
		// player who has flag
		PlayerId playerId;
		// how many grabs before removed
		int grabs;
		// true if flag must be in game
		bool required;
		// time flag will land
		TimeKeeper dropDone;
};

struct TeamInfo {
	public:
		Team team;
};

#include "WorldInfo.h"

class ListServerLink {
	public:
		Address address;
		int port;
		int socket;
		const char *nextMessage;
};

// server address to listen on
static Address serverAddress;
// default port
static int wksPort = ServerPort;
// well known service socket
static int wksSocket;
static bool useGivenPort = false;
static bool useFallbackPort = false;
// udpSocket should also be on serverAddress
static int udpSocket;
static PingPacket pingReply;
static const char *serverInterface = NULL;
// highest fd used
static int maxFileDescriptor;
// players list
static PlayerInfo player[MaxPlayers];
// team info
static TeamInfo team[NumTeams];
// flags list
static FlagInfo *flag = NULL;
// num flags in flag list
static int numFlags;
static int numFlagsInAir;
// types of extra flags allowed
static FlagId *allowedFlags = NULL;
static int numAllowedFlags;
// num randomly generated flags
static int numExtraFlags;
static bool done = false;
// true if hit time/score limit
static bool gameOver = true;
static int exitCode = 0;
// true if -b on cmd line
bool randomBoxes;
// true if -h on cmd line
bool randomHeights;
// true if -t on cmd line
bool useTeleporters;
// true if -g on cmd line
static bool oneGameOnly;
static int gameStyle;
static PlayerId maxPlayers = MaxPlayers;
// max simulataneous per player
static uint16_t maxShots;
static uint16_t maxTeam[NumTeams];
static uint16_t shakeWins = 0;
static uint16_t shakeTimeout = 0;
static float linearAcceleration;
static float angularAcceleration;
static int maxPlayerScore = 0;
static int maxTeamScore = 0;
static int debug = 0;

#ifdef PRINTSCORE
static bool printScore = false;
#endif
#ifdef TIMELIMIT
static float timeLimit = 0.0f;
static float timeElapsed = 0.0f;
static TimeKeeper gameStartTime;
#endif
static bool publicizeServer = false;
static std::string publicizedAddress;
static bool publicizedAddressGiven = false;
static const char *publicizedTitle = NULL;
static const char *listServerURL = DefaultListServerURL;
static TimeKeeper listServerLastAddTime;
static ListServerLink listServerLinks[MaxListServers];
static int listServerLinksCount = 0;

static WorldInfo *world = NULL;
static char *worldDatabase = NULL;
static int worldDatabaseSize = 0;
float basePos[NumTeams][3];
float baseRotation[NumTeams];
float baseSize[NumTeams][3];
float safetyBasePos[NumTeams][3];
static const char *worldFile = NULL;

static float lagwarnthresh = -1.0;
static int maxlagwarn = 10000;
static char *password = NULL;

static void removePlayer(PlayerId playerId);
static void resetFlag(int flagIndex);

//
// rest of server (no more classes, just functions)
//

void *getPacketFromClient(PlayerId playerId, uint16_t *length, uint16_t *rseqno)
{
	struct PacketQueue *moving = player[playerId].dqueue;
	struct PacketQueue *remindme = NULL;
	while (moving != NULL) {
		if (moving->next == NULL) {
			void *remember = moving->data;
			*length = moving->length;
			if (rseqno)
				*rseqno = moving->seqno;
			if (remindme)
				remindme->next = NULL;
			else
				player[playerId].dqueue = NULL;
			free(moving);
			return remember;
		}
		remindme = moving;
		moving = moving->next;
	}
	*length = 0;
	return NULL;
}

void printQueueDepth(PlayerId playerId)
{
	int d,u;
	struct PacketQueue *moving;
	moving = player[playerId].dqueue;
	d = 0;
	while (moving) {
		d++;
		moving = moving->next;
	}
	u = 0;
	moving = player[playerId].uqueue;
	while (moving) {
		u++;
		moving = moving->next;
	}
	DEBUG4("Player %d RECV QUEUE %d   SEND QUEUE %d\n", playerId, d,u);
}

bool enqueuePacket(PlayerId playerId, int op, int rseqno, void *msg, int n)
{
	struct PacketQueue *oldpacket;
	struct PacketQueue *newpacket;

	if (op == SEND)
		oldpacket = player[playerId].uqueue;
	else {
		oldpacket = player[playerId].dqueue;
	}

	if (oldpacket) {
		if (oldpacket->data)
			free(oldpacket->data);
		free(oldpacket);
	}

	newpacket = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));
	newpacket->seqno = rseqno;
	newpacket->data = (unsigned char *)malloc(n);
	memcpy((unsigned char *)newpacket->data, (unsigned char *)msg, n);
	newpacket->length = n;
	newpacket->next = NULL;

	if (op == SEND)
		player[playerId].uqueue = newpacket;
	else
		player[playerId].dqueue = newpacket;

	return true;
}


void disqueuePacket(PlayerId playerId, int op, int /* rseqno */)
{
	struct PacketQueue *oldpacket;

	if (op == SEND)
		oldpacket = player[playerId].uqueue;
	else {
		oldpacket = player[playerId].dqueue;
	}

	if (oldpacket) {
		if (oldpacket->data)
			free(oldpacket->data);
		free(oldpacket);
	}

	if (op == SEND)
		player[playerId].uqueue = NULL;
	else
		player[playerId].dqueue = NULL;
}


void *assembleSendPacket(PlayerId playerId, int *len)
{
	struct PacketQueue *moving = player[playerId].uqueue;
	unsigned char *assemblybuffer;
	int n = MaxPacketLen, packets = 0, startseq = (-1), endseq = 0, noinqueue;
	unsigned char *buf;

	assemblybuffer = (unsigned char *)malloc(n);
	buf = assemblybuffer;

	buf = (unsigned char *)nboPackUShort(buf, 0xfeed);
	buf = (unsigned char *)nboPackUShort(buf, player[playerId].lastRecvPacketNo);
	n -= 4;

	// lets find how deep the send queue is
	noinqueue = 0;
	while (moving) {
		noinqueue++;
		moving = moving->next;
		if (moving)
			startseq = moving->seqno;
	}

	// lets see if it is too large (CAN'T BE, Queue is always 1 length)

	if (noinqueue > 128) {
		// we have more than 128 not aknowledged packets
		DEBUG1("%d Packets outstanding\n",noinqueue);
	}

	// this is actually the number of single
	// packets we send with a single write
	// 1 means we only send the most recent
	// 2 the last two most recent, etc...
	// it is currently advisable to use 1 here as
	// a. lines are resonable stable (not much loss)
	// b. an ISDN link will be flooded with 4 > player
	noinqueue -= 1;

	moving = player[playerId].uqueue;

	packets = 0;
	startseq = -1;

	while (moving) {
		packets++;
		if (packets > noinqueue) {
			if (startseq < 0)
				startseq = moving->seqno;
			endseq = moving->seqno;
			n -= 2;
			if (n <= 2)
				break;
			buf = (unsigned char *)nboPackUShort(buf, moving->length);
			n -= 2;
			if (n <= 2)
				break;
			buf = (unsigned char *)nboPackUShort(buf, moving->seqno);
			n -= moving->length;
			if (n <= 2)
				break;
			memcpy((unsigned char *)buf, (unsigned char *)moving->data, moving->length);
			buf += moving->length;
		} // noinqueue
		moving = moving->next;
	}
	buf = (unsigned char *)nboPackUShort(buf, 0xffff);
	n -= 2;
	if (n <= 2) {
		DEBUG3("ASSEMBLE SEND PACKET OVERRUN BUFFER\n");
		*len = 0;
		return assemblybuffer;
	}
	if (n < 4096)
		DEBUG3("Warning: TOO Long a PACKET: %d %d\n",noinqueue, packets);
	*len = (MaxPacketLen - n);
	DEBUG4("ASSEMBLY %d packets, %d - %d\n",packets,startseq, endseq);
	return assemblybuffer;
}

void disassemblePacket(PlayerId playerId, void *msg, int *nopackets)
{
	unsigned short marker;
	unsigned short usdata;
	unsigned char *buf = (unsigned char *)msg;

	int npackets = 0;

	DEBUG4("::: Disassemble Packet\n");

	buf = (unsigned char *)nboUnpackUShort(buf, marker);
	if (marker!= 0xfeed) {
		DEBUG3("Reject UPacket because invalid header %04x\n", marker);
		return;
	}
	buf = (unsigned char *)nboUnpackUShort(buf, usdata);

	disqueuePacket(playerId, SEND, usdata);

	while (true) {
		unsigned short seqno;
		unsigned short length;
		int ilength;

		buf = (unsigned char *)nboUnpackUShort(buf, length);
		ilength = length;
		if (length == 0xffff)
			break;
		else if (ilength > 1024) {
			DEBUG1("* RECEIVE PACKET BUFFER OVERFLOW ATTEMPT: %d sent %d Bytes\n",
					playerId, ilength);
			break;
		}
		buf = (unsigned char *)nboUnpackUShort(buf, seqno);
		DEBUG4("SEQ RECV %d Enqueing now...\n",seqno);
		enqueuePacket(playerId, RECEIVE, seqno, buf, length);
		buf+= length;
		npackets++;
	}
	DEBUG4("%d: Got %d packets\n", (int)time(0), npackets);
	// printQueueDepth(playerId);
	*nopackets = npackets;
}


const void *assembleUDPPacket(PlayerId playerId, const void *b, int *l)
{
	int length = *l;

	DEBUG4("ENQUEUE %d [%d]\n",length, player[playerId].lastSendPacketNo);
	enqueuePacket(playerId, SEND, player[playerId].lastSendPacketNo, (void *)b, length);

	player[playerId].lastSendPacketNo++;

	DEBUG4("ASSEMBLE\n");
	return assembleSendPacket(playerId, l);
}

// write an UDP packet down the link to the client, we don't know if it comes through
// so this code is using a queuing mechanism.

static int puwrite(PlayerId playerId, const void *b, int l)
{
	PlayerInfo& p = player[playerId];
	const void *tobesend = b;

	//DEBUG4("INTO PUWRITE\n");

	tobesend = assembleUDPPacket(playerId,b,&l);

	if (!tobesend || (l == 0)) {
		removePlayer(playerId);
		DEBUG1("Send Queue Overrun for player %d (%s)\n", playerId, p.callSign);
		if (tobesend)
			free((unsigned char *)tobesend);
		return -1;
	}

	DEBUG4("PUWRITE - ASSEMBLED UDP LEN %d for Player %d\n",l,playerId);
	// write as much data from buffer as we can in one send()

	int n;

#ifdef TESTLINK
	if ((random()%LINKQUALITY) != 0) {
#endif
		n = sendto(udpSocket, (const char *)tobesend, l, 0, (struct sockaddr*)&p.uaddr, sizeof(p.uaddr));
#ifdef TESTLINK
	} else
		DEBUG1("Drop Packet due to Test\n");
#endif
	if (tobesend)
		free((unsigned char *)tobesend);

	// handle errors
	if (n < 0) {
		// get error code
		const int err = getErrno();

		// just try again later if it's one of these errors
		if (err == EAGAIN || err == EINTR)
			return -1;

		// if socket is closed then give up
		if (err == ECONNRESET || err == EPIPE) {
			DEBUG4("REMOVE: ECONNRESET/EPIPE\n");
			removePlayer(playerId);
			return -1;
		}

		// dump other errors and continue
		nerror("error on UDP write");
		DEBUG1("player is %d (%s)\n", playerId, p.callSign);
		DEBUG1("%d bytes\n", n);

		// we may actually run into not enough buffer space here
		// but as the link is unreliable anyway we never treat it as
		// hard error
	}

	return n;
}

static int prealwrite(PlayerId playerId, const void *b, int l)
{
	PlayerInfo& p = player[playerId];
	assert(p.fd != NotConnected && l > 0);

	// write as much data from buffer as we can in one send()
	const int n = send(p.fd, (const char *)b, l, 0);

	// handle errors
	if (n < 0) {
		// get error code
		const int err = getErrno();

		// just try again later if it's one of these errors
		if (err == EAGAIN || err == EINTR)
			return -1;

		// if socket is closed then give up
		if (err == ECONNRESET || err == EPIPE) {
			DEBUG4("REMOVE: Reset socket (4)\n");
			removePlayer(playerId);
			return -1;
		}

		// dump other errors and remove the player
		nerror("error on write");
		DEBUG1("player is %d (%s)\n", playerId, p.callSign);
		DEBUG4("REMOVE: WRITE ERROR\n");
		removePlayer(playerId);
		return -1;
	}

	return n;
}

// try to write stuff from the output buffer
static void pflush(PlayerId playerId)
{
	PlayerInfo& p = player[playerId];
	if (p.fd == NotConnected || p.outmsgSize == 0)
		return;

	const int n = prealwrite(playerId, p.outmsg + p.outmsgOffset, p.outmsgSize);
	if (n > 0) {
		p.outmsgOffset += n;
		p.outmsgSize   -= n;
	}
}

static void pwrite(PlayerId playerId, const void *b, int l)
{
	PlayerInfo& p = player[playerId];
	if (p.fd == NotConnected || l == 0)
		return;

	// Check if UDP Link is used instead of TCP, if so jump into puwrite
	if (p.ulinkup) {
		uint16_t len, code;
		void *buf = (void *)b;
		buf = nboUnpackUShort(buf, len);
		buf = nboUnpackUShort(buf, code);

		// only send bulk messages by UDP
		switch (code) {
		case MsgShotBegin:
		case MsgShotEnd:
		case MsgPlayerUpdate:
		case MsgShotUpdate:
			puwrite(playerId,b,l);
			return;
		}
	}

	// try flushing buffered data
	pflush(playerId);

	//DEBUG4("TCP write\n");
	// if the buffer is empty try writing the data immediately
	if (p.fd != NotConnected && p.outmsgSize == 0) {
		const int n = prealwrite(playerId, b, l);
		if (n > 0) {
			b  = (const void*)(((const char*)b) + n);
			l -= n;
		}
	}

	// write leftover data to the buffer
	if (p.fd != NotConnected && l > 0) {
		// is there enough room in buffer?
		if (p.outmsgCapacity < p.outmsgSize + l) {
			// double capacity until it's big enough
			int newCapacity = (p.outmsgCapacity == 0) ? 512 : p.outmsgCapacity;
			while (newCapacity < p.outmsgSize + l)
				newCapacity <<= 1;

			// if the buffer is getting too big then drop the player.  chances
			// are the network is down or too unreliable to that player.
			// FIXME -- is 20kB to big?  to small?
			if (newCapacity >= 20 * 1024) {
				DEBUG1("dropping unresponsive player %d (%s) with %d bytes queued\n",
						playerId, p.callSign, p.outmsgSize + l);
				DEBUG4("REMOVE: CAPACITY\n");
				removePlayer(playerId);
				return;
			}

			// allocate memory
			char *newbuf = new char[newCapacity];

			// copy old data over
			memmove(newbuf, p.outmsg + p.outmsgOffset, p.outmsgSize);

			// cutover
			delete[] p.outmsg;
			p.outmsg         = newbuf;
			p.outmsgOffset   = 0;
			p.outmsgCapacity = newCapacity;
		}

		// if we can't fit new data at the end of the buffer then move existing
		// data to head of buffer
		// FIXME -- use a ring buffer to avoid moving memory
		if (p.outmsgOffset + p.outmsgSize + l > p.outmsgCapacity) {
			memmove(p.outmsg, p.outmsg + p.outmsgOffset, p.outmsgSize);
			p.outmsgOffset = 0;
		}

		// append data
		memmove(p.outmsg + p.outmsgOffset + p.outmsgSize, b, l);
		p.outmsgSize += l;
	}
}

static char sMsgBuf[MaxPacketLen];
static char *getDirectMessageBuffer()
{
	return &sMsgBuf[2*sizeof(short)];
}

#ifdef NETWORK_STATS
void initPlayerMessageStats(PlayerId playerId)
{
	int i;
	struct MessageCount *msg;
	int direction;

	for (direction = 0; direction <= 1; direction++) {
		msg = player[playerId].msg[direction];
		for (i = 0; i < MessageTypes && msg[i].code != 0; i++) {
			msg[i].count = 0;
			msg[i].code = 0;
		}
		player[playerId].msgBytes[direction] = 0;
		player[playerId].perSecondTime[direction] = player[playerId].time;
		player[playerId].perSecondCurrentMsg[direction] = 0;
		player[playerId].perSecondMaxMsg[direction] = 0;
		player[playerId].perSecondCurrentBytes[direction] = 0;
		player[playerId].perSecondMaxBytes[direction] = 0;
	}
}

int countMessage(PlayerId playerId, uint16_t code, int len, int direction)
{
	int i;
	struct MessageCount *msg;

	// add length of type and length
	len += 4;
	player[playerId].msgBytes[direction] += len;
	msg = player[playerId].msg[direction];
	TimeKeeper now = TimeKeeper::getCurrent();
	for (i = 0; i < MessageTypes && msg[i].code != 0; i++)
		if (msg[i].code == code)
			break;
	msg[i].code = code;
	if (msg[i].maxSize < len)
		msg[i].maxSize = len;
	msg[i].count++;
	if (now - player[playerId].perSecondTime[direction] < 1.0f) {
		player[playerId].perSecondCurrentMsg[direction]++;
		player[playerId].perSecondCurrentBytes[direction] += len;
	}
	else {
		player[playerId].perSecondTime[direction] = now;
		if (player[playerId].perSecondMaxMsg[direction] <
				player[playerId].perSecondCurrentMsg[direction])
			player[playerId].perSecondMaxMsg[direction] =
					player[playerId].perSecondCurrentMsg[direction];
		if (player[playerId].perSecondMaxBytes[direction] <
				player[playerId].perSecondCurrentBytes[direction])
			player[playerId].perSecondMaxBytes[direction] =
					player[playerId].perSecondCurrentBytes[direction];
		player[playerId].perSecondCurrentMsg[direction] = 0;
		player[playerId].perSecondCurrentBytes[direction] = 0;
	}
	return (msg[i].count);
}

void dumpPlayerMessageStats(PlayerId playerId)
{
	int i;
	struct MessageCount *msg;
	int total;
	int direction;

	DEBUG1("Player connect time: %f\n",
			TimeKeeper::getCurrent() - player[playerId].time);
	for (direction = 0; direction <= 1; direction++) {
		total = 0;
		DEBUG1("Player messages %s:", direction ? "out" : "in");
		msg = player[playerId].msg[direction];
		for (i = 0; i < MessageTypes && msg[i].code != 0; i++) {
			DEBUG1(" %c%c:%u(%u)", msg[i].code >> 8, msg[i].code & 0xff,
					msg[i].count, msg[i].maxSize);
			total += msg[i].count;
		}
		DEBUG1(" total:%u(%u) ", total, player[playerId].msgBytes[direction]);
		DEBUG1("max msgs/bytes per second: %u/%u\n",
				player[playerId].perSecondMaxMsg[direction],
				player[playerId].perSecondMaxBytes[direction]);

	}
	fflush(stdout);
}
#endif

static void directMessage(PlayerId playerId, uint16_t code, int len, const void *msg)
{
	if (player[playerId].fd == NotConnected)
		return;

	// send message to one player
	void *bufStart = (char *)msg - 2*sizeof(short);;
	void *buf = bufStart;
	buf = nboPackUShort(buf, uint16_t(len));
	buf = nboPackUShort(buf, code);
	pwrite(playerId, bufStart, len + 4);
#ifdef NETWORK_STATS
	countMessage(playerId, code, len, 1);
#endif
}

static void broadcastMessage(uint16_t code, int len, const void *msg)
{
	// send message to everyone
	for (PlayerId i = 0; i < maxPlayers; i++)
		if (player[i].state > PlayerInLimbo)
			directMessage(i, code, len, msg);
}

static void setupUDPLink(int t, struct sockaddr_in* uaddr) {

	memcpy((char *)&player[t].uaddr, (char *)uaddr, sizeof(struct sockaddr_in));

	// show some message on the console
	DEBUG3("UDP link created, player %d remote %s:%d\n",
			t, inet_ntoa(player[t].uaddr.sin_addr), ntohs(player[t].uaddr.sin_port));

	// init the queues
	player[t].uqueue = player[t].dqueue = NULL;
	player[t].lastRecvPacketNo = player[t].lastSendPacketNo = 0;

	player[t].ulinkup = 1;
	return;
}

// obsolete now
void OOBQueueUpdate(int t, uint32_t rseqno) {
	if (rseqno > 0)
		disqueuePacket(t, SEND, rseqno);
}

static void setNoDelay(int fd)
{
	// turn off TCP delay (collection).  we want packets sent immediately.
#if defined(_WIN32)
	BOOL on = TRUE;
#else
	int on = 1;
#endif
	struct protoent *p = getprotobyname("tcp");
	if (p && setsockopt(fd, p->p_proto, TCP_NODELAY, (SSOType)&on, sizeof(on)) < 0) {
		nerror("enabling TCP_NODELAY");
	}
}

// uread - interface to the UDP Receive routines
static int uread(int *playerId, int *nopackets)
{
	int n = 0;
	struct sockaddr_in uaddr;
	unsigned char ubuf[MaxPacketLen];
	AddrLen recvlen = sizeof(uaddr);
	//DEBUG4("Into UREAD\n");

	*nopackets = 0;

	PlayerInfo *pPlayerInfo;
	if ((n = recvfrom(udpSocket, (char *)ubuf, MaxPacketLen, MSG_PEEK, (struct sockaddr*)&uaddr, &recvlen)) != -1) {
		uint16_t len, lseqno;
		void *pmsg;
		PlayerId pi;
		for (pi = 0, pPlayerInfo = player; pi < MaxPlayers; pi++, pPlayerInfo++) {
			if ((pPlayerInfo->ulinkup) &&
					(pPlayerInfo->uaddr.sin_port == uaddr.sin_port) &&
					(memcmp(&pPlayerInfo->uaddr.sin_addr, &uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0)) {
				break;
			}
		}

		// get the packet
		n = recv(udpSocket, (char *)ubuf, MaxPacketLen, 0);
		if (pi == MaxPlayers) {
			// didn't find a player match, is it a MsgUDPLinkEstablished?
			uint16_t len, code;
			void *buf = ubuf + 8;
			buf = nboUnpackUShort(buf, len);
			buf = nboUnpackUShort(buf, code);
			DEBUG4("uread() packet len %4x code %4x %s:%d\n", len, code, inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
			if (code == MsgUDPLinkEstablished)
			{
				buf = nboUnpackUByte(buf, pi);
				setupUDPLink(pi, &uaddr);
			}
			else
			{	
				// no match, discard packet
				DEBUG3("uread() discard packet! %s:%d choices p(l) h:p", inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
				for (pi = 0, pPlayerInfo = player; pi < MaxPlayers; pi++, pPlayerInfo++) {
					if (pPlayerInfo->fd != -1) {
						DEBUG3(" %d(%d) %s:%d",
								pi, pPlayerInfo->ulinkup,
								inet_ntoa(pPlayerInfo->uaddr.sin_addr),
								ntohs(pPlayerInfo->uaddr.sin_port));
					}
				}
				DEBUG3("\n");
				*playerId = 0;
				return 0;
			}
		}

		*playerId = pi;
		pPlayerInfo = &player[pi];
		DEBUG4("uread() player %d %s:%d len %d from %s:%d on %i\n",
		pi, inet_ntoa(pPlayerInfo->uaddr.sin_addr),
		ntohs(pPlayerInfo->uaddr.sin_port), n, inet_ntoa(uaddr.sin_addr),
		ntohs(uaddr.sin_port), udpSocket);

		if (n > 0) {
			// got something! now disassemble the package block into single BZPackets
			// filling up the dqueue with these packets
			disassemblePacket(pi, ubuf, nopackets);

			// old code is obsolete
			// if (*nopackets > 6 )
			//   pucdwrite(playerId);
		}
		// have something in the receive buffer? so get it
		// due to the organization sequence and reliability is always granted
		// even if some packets are lost during transfer
		pmsg =  getPacketFromClient(pi, &len, &lseqno);
		if (pmsg != NULL) {
			int clen = len;
			if (clen < 1024) {
				memcpy(pPlayerInfo->udpmsg,pmsg,clen);
				pPlayerInfo->udplen = clen;
			}
			// be sure to free the packet again
			free(pmsg);
			DEBUG4("GOT UDP READ %d Bytes [%d]\n",len, lseqno);
			return pPlayerInfo->udplen;
		}
	}
	return 0;
}

static int pread(PlayerId playerId, int l)
{
	PlayerInfo& p = player[playerId];
	//DEBUG1("pread,playerId,l %i %i\n",playerId,l);
	if (p.fd == NotConnected || l == 0)
		return 0;

	// read more data into player's message buffer
	const int e = recv(p.fd, p.tcpmsg + p.tcplen, l, 0);

	// accumulate bytes read
	if (e > 0) {
		p.tcplen += e;
	} else if (e < 0) {
		// handle errors
		// get error code
		const int err = getErrno();

		// ignore if it's one of these errors
		if (err == EAGAIN || err == EINTR)
			return 0;

		// if socket is closed then give up
		if (err == ECONNRESET || err == EPIPE) {
			DEBUG1("REMOVE: Socket reset %d (%s)\n", playerId, p.callSign);
			removePlayer(playerId);
			return -1;
		}

		// dump other errors and remove the player
		nerror("error on read");
		DEBUG1("player is %d (%s)\n", playerId, p.callSign);
		DEBUG4("REMOVE: READ ERROR\n");
		removePlayer(playerId);
		return -1;
	} else {
		// disconnected
		DEBUG1("REMOVE: Disconnected %d (%s)\n", playerId, p.callSign);
		removePlayer(playerId);
		return -1;
	}

	return e;
}

static void sendFlagUpdate(int flagIndex, int index = -1)
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, flagIndex);
	buf = flag[flagIndex].flag.pack(buf);
	if (index == -1)
		broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
	else
		directMessage(index, MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
}

static void sendTeamUpdate(int teamIndex, int index = -1)
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, teamIndex);
	buf = team[teamIndex].team.pack(buf);
	if (index == -1)
		broadcastMessage(MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart);
	else
		directMessage(index, MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart);
}

static void sendPlayerUpdate(PlayerId playerId, int index)
{
	void *buf, *bufStart = getDirectMessageBuffer();
	PlayerInfo *pPlayer = &player[playerId];
	buf = nboPackUByte(bufStart, playerId);
	buf = nboPackUShort(buf, uint16_t(pPlayer->type));
	buf = nboPackUShort(buf, uint16_t(pPlayer->team));
	buf = nboPackUShort(buf, uint16_t(pPlayer->wins));
	buf = nboPackUShort(buf, uint16_t(pPlayer->losses));
	buf = nboPackString(buf, pPlayer->callSign, CallSignLen);
	buf = nboPackString(buf, pPlayer->email, EmailLen);
	if (playerId == index) {
		// send all players info about player[playerId]
		for (PlayerId i = 0; i < maxPlayers; i++)
			if (player[i].state > PlayerInLimbo && i != playerId)
				directMessage(i, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
	}
	directMessage(index, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
}

static void closeListServer(int index)
{
	assert(index >= 0 && index < MaxListServers);
	if (index >= listServerLinksCount)
		return;

	ListServerLink& link = listServerLinks[index];
	if (link.socket != NotConnected) {
		shutdown(link.socket, 2);
		closesocket(link.socket);
		DEBUG4("Closing List server %d\n",index);
		link.socket = NotConnected;
		link.nextMessage = "";
	}
}

static void closeListServers()
{
	for (int i = 0; i < listServerLinksCount; ++i)
		closeListServer(i);
}

static void openListServer(int index)
{
	assert(index >= 0 && index < MaxListServers);
	if (index >= listServerLinksCount)
		return;

	ListServerLink& link = listServerLinks[index];
	link.nextMessage = "";

	// start opening connection if not already doing so
	if (link.socket == NotConnected) {
		link.socket = socket(AF_INET, SOCK_STREAM, 0);
		DEBUG4("Opening List Server %d\n",index);
		if (link.socket == NotConnected) {
			closeListServer(index);
			return;
		}

		// set to non-blocking for connect
		if (BzfNetwork::setNonBlocking(link.socket) < 0) {
			closeListServer(index);
			return;
		}

		// connect.  this should fail with EINPROGRESS but check for
		// success just in case.
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(link.port);
		addr.sin_addr   = link.address;
		if (connect(link.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
			if (getErrno() != EINPROGRESS) {
				nerror("connecting to list server");
				closeListServer(index);
			}
			else {
				if (maxFileDescriptor < link.socket)
					maxFileDescriptor = link.socket;
			}
		}
	}
}

static void sendMessageToListServer(const char *msg)
{
	// ignore if not publicizing
	if (!publicizeServer)
		return;

	// start opening connections if not already doing so
	for (int i = 0; i < listServerLinksCount; i++) {
		openListServer(i);

		// record next message to send.  note that each message overrides
		// any other message, except SETNUM doesn't override ADD (cos ADD
		// sends SETNUM data anyway).
		ListServerLink& link = listServerLinks[i];
		if (strcmp(msg, "SETNUM") != 0 || strcmp(link.nextMessage, "ADD") != 0)
			link.nextMessage = msg;
	}
}

static void sendMessageToListServerForReal(int index)
{
	assert(index >= 0 && index < MaxListServers);
	if (index >= listServerLinksCount)
		return;

	// ignore if link not connected
	ListServerLink& link = listServerLinks[index];
	if (link.socket == NotConnected)
		return;

	char msg[1024];
	if (strcmp(link.nextMessage, "ADD") == 0) {
		// update player counts in ping reply.  pretend there are no players
		// if the game is over.
		if (gameOver) {
			pingReply.rogueCount = team[0].team.activeSize;
			pingReply.redCount = team[1].team.activeSize;
			pingReply.greenCount = team[2].team.activeSize;
			pingReply.blueCount = team[3].team.activeSize;
			pingReply.purpleCount = team[4].team.activeSize;
		}
		else {
			pingReply.rogueCount = 0;
			pingReply.redCount = 0;
			pingReply.greenCount = 0;
			pingReply.blueCount = 0;
			pingReply.purpleCount = 0;
		}

		// encode ping reply as ascii hex digits
		char gameInfo[PingPacketHexPackedSize];
		pingReply.packHex(gameInfo);

		// send ADD message
		sprintf(msg, "%s %s %d %s %.*s %.256s\n\n", link.nextMessage,
				publicizedAddress.c_str(),
				VERSION % 1000,
				ServerVersion,
				PingPacketHexPackedSize, gameInfo,
				publicizedTitle);
	}
	else if (strcmp(link.nextMessage, "REMOVE") == 0) {
		// send REMOVE
		sprintf(msg, "%s %s\n\n", link.nextMessage, publicizedAddress.c_str());
	}
	else if (strcmp(link.nextMessage, "SETNUM") == 0) {
		// pretend there are no players if the game is over
		if (gameOver)
			sprintf(msg, "%s %s 0 0 0 0 0\n\n",
					link.nextMessage,
					publicizedAddress.c_str());
		else
			sprintf(msg, "%s %s %d %d %d %d %d\n\n", link.nextMessage,
					publicizedAddress.c_str(),
					team[0].team.activeSize,
					team[1].team.activeSize,
					team[2].team.activeSize,
					team[3].team.activeSize,
					team[4].team.activeSize);
	}
	if (debug >= 3) {
		std::cerr << msg;
	}
	send(link.socket, msg, strlen(msg), 0);

	// hangup (we don't care about replies)
	closeListServer(index);
}

static void publicize()
{
	// hangup any previous list server sockets
	closeListServers();

	// list server initialization
	listServerLinksCount  = 0;

	// parse the list server URL if we're publicizing ourself
	if (publicizeServer && publicizedTitle) {
		// dereference URL, including following redirections.  get no
		// more than MaxListServers urls.
		BzfNetwork::URLList urls, failedURLs;
		urls.push_back(listServerURL);
		BzfNetwork::dereferenceURLs(urls, MaxListServers, failedURLs);

		for (unsigned int j = 0; j < failedURLs.size(); ++j)
			DEBUG1("failed: %s\n", failedURLs[j].c_str());

		// check url list for validity
		for (unsigned int i = 0; i < urls.size(); ++i) {
			// parse url
			std::string protocol, hostname, pathname;
			int port = ServerPort + 1;
			if (!BzfNetwork::parseURL(urls[i], protocol, hostname, port, pathname))
				continue;

			// ignore if not right protocol
			if (protocol != "bzflist")
				continue;

			// ignore if port is bogus
			if (port < 1 || port > 65535)
				continue;

			// ignore if bad address
			Address address = Address::getHostAddress(hostname.c_str());
			if (address.isAny())
				continue;

			// add to list
			listServerLinks[listServerLinksCount].address = address;
			listServerLinks[listServerLinksCount].port = port;
			listServerLinks[listServerLinksCount].socket  = NotConnected;
			listServerLinksCount++;
		}

		// schedule message for list server
		sendMessageToListServer("ADD");
	}
}

static bool serverStart()
{
#if defined(_WIN32)
	const BOOL optOn = TRUE;
	BOOL opt = optOn;
#else
	const int optOn = 1;
	int opt = optOn;
#endif
	maxFileDescriptor = 0;

	// init addr:port structure
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr = serverAddress;

	// look up service name and use that port if no port given on
	// command line.  if no service then use default port.
	if (!useGivenPort) {
		struct servent *service = getservbyname("bzfs", "tcp");
		if (service) {
			wksPort = ntohs(service->s_port);
		}
	}
	pingReply.serverId.port = addr.sin_port = htons(wksPort);

	// open well known service port
	wksSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (wksSocket == -1) {
		nerror("couldn't make connect socket");
		return false;
	}
#ifdef SO_REUSEADDR
	/* set reuse address */
	opt = optOn;
	if (setsockopt(wksSocket, SOL_SOCKET, SO_REUSEADDR, (SSOType)&opt, sizeof(opt)) < 0) {
		nerror("serverStart: setsockopt SO_REUSEADDR");
		closesocket(wksSocket);
		return false;
	}
#endif
	if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
		if (!useFallbackPort) {
			nerror("couldn't bind connect socket");
			closesocket(wksSocket);
			return false;
		}

		// if we get here then try binding to any old port the system gives us
		addr.sin_port = htons(0);
		if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
			nerror("couldn't bind connect socket");
			closesocket(wksSocket);
			return false;
		}

		// fixup ping reply
		AddrLen addrLen = sizeof(addr);
		if (getsockname(wksSocket, (struct sockaddr*)&addr, &addrLen) >= 0)
			pingReply.serverId.port = addr.sin_port;

		// fixup publicized name will want it here later
		wksPort = ntohs(addr.sin_port);
	}

	if (listen(wksSocket, 5) == -1) {
		nerror("couldn't make connect socket queue");
		closesocket(wksSocket);
		return false;
	}
	maxFileDescriptor = wksSocket;

	int n;
	// we open a udp socket on the same port
	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		nerror("couldn't make udp connect socket");
		return false;
	}

	// increase send/rcv buffer size
#if defined(_WIN32)
	n = setsockopt(udpSocket,SOL_SOCKET,SO_SNDBUF,(const char *)&udpBufSize,sizeof(int));
#else
	n = setsockopt(udpSocket,SOL_SOCKET,SO_SNDBUF,(const void *)&udpBufSize,sizeof(int));
#endif
	if (n < 0) {
		nerror("couldn't increase udp send buffer size");
		closesocket(wksSocket);
		closesocket(udpSocket);
		return false;
	}

#if defined(_WIN32)
	n = setsockopt(udpSocket,SOL_SOCKET,SO_RCVBUF,(const char *)&udpBufSize,sizeof(int));
#else
	n = setsockopt(udpSocket,SOL_SOCKET,SO_RCVBUF,(const void *)&udpBufSize,sizeof(int));
#endif
	if (n < 0) {
		nerror("couldn't increase udp receive buffer size");
		closesocket(wksSocket);
		closesocket(udpSocket);
		return false;
	}
	addr.sin_port = htons(wksPort);
	if (bind(udpSocket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		nerror("couldn't bind udp listen port");
		closesocket(wksSocket);
		closesocket(udpSocket);
		return false;
	}
	// don't buffer info, send it immediately
	BzfNetwork::setNonBlocking(udpSocket);
	maxFileDescriptor = udpSocket;

	for (PlayerId i = 0; i < MaxPlayers; i++) {	// no connections
		player[i].fd = NotConnected;
		player[i].state = PlayerNoExist;
		player[i].outmsg = NULL;
		player[i].outmsgSize = 0;
		player[i].outmsgOffset = 0;
		player[i].outmsgCapacity = 0;
	}

	listServerLinksCount = 0;
	publicize();
	return true;
}

static void serverStop()
{
	// shut down server
	// first ignore further attempts to kill me
	PLATFORM->signalCatch(kSigINT,  kSigIGN);
	PLATFORM->signalCatch(kSigTERM, kSigIGN);

	// reject attempts to talk to server
	shutdown(wksSocket, 2);
	closesocket(wksSocket);

	// tell players to quit
	int i;
	for (i = 0; i < MaxPlayers; i++)
		directMessage(i, MsgSuperKill, 0, getDirectMessageBuffer());

	// close connections
	for (i = 0; i < MaxPlayers; i++)
		if (player[i].fd != NotConnected) {
			shutdown(player[i].fd, 2);
			closesocket(player[i].fd);
			delete[] player[i].outmsg;
		}

	// now tell the list servers that we're going away.  this can
	// take some time but we don't want to wait too long.  we do
	// our own multiplexing loop and wait for a maximum of 3 seconds
	// total.
	sendMessageToListServer("REMOVE");
	TimeKeeper start = TimeKeeper::getCurrent();
	do {
		// compute timeout
		float waitTime = 3.0f - (TimeKeeper::getCurrent() - start);
		if (waitTime <= 0.0f)
			break;

		// check for list server socket connection
		int fdMax = -1;
		fd_set write_set;
		FD_ZERO(&write_set);
		for (i = 0; i < listServerLinksCount; i++)
			if (listServerLinks[i].socket != NotConnected) {
				FD_SET(listServerLinks[i].socket, &write_set);
				fdMax = listServerLinks[i].socket;
			}
		if (fdMax == -1)
			break;

		// wait for socket to connect or timeout
		struct timeval timeout;
		timeout.tv_sec = long(floorf(waitTime));
		timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
		int nfound = select(fdMax + 1, NULL, (fd_set*)&write_set, 0, &timeout);
		// check for connection to list server
		if (nfound > 0)
			for (i = 0; i < listServerLinksCount; ++i)
				if (listServerLinks[i].socket != NotConnected &&
						FD_ISSET(listServerLinks[i].socket, &write_set))
					sendMessageToListServerForReal(i);
	} while (true);

	// stop list server communication
	closeListServers();
}

static void relayPlayerPacket(int index, uint16_t len, const void *rawbuf)
{
	// relay packet to all players
	for (PlayerId i = 0; i < maxPlayers; i++)
		if ((i != index) && (player[i].state > PlayerInLimbo))
			pwrite(i, rawbuf, len + 4);
}

#include "WorldRegions.h"
static bool defineWorld()
{
	// clean up old database
	delete world;
	delete[] worldDatabase;

	// make world and add buildings
	if (worldFile)
		world = defineWorldFromFile(worldFile);
	else if (gameStyle & TeamFlagGameStyle)
		world = defineTeamWorld();
	else
		world = defineRandomWorld();
	if (world == NULL)
		return false;

	// set regions
	defineWorldRegions(world);

	// package up world
	world->packDatabase();
	// now get world packaged for network transmission
	worldDatabaseSize = 4 + 24 + world->getDatabaseSize() + 2;
	if (gameStyle & TeamFlagGameStyle)
		worldDatabaseSize += 4 * (4 + 9 * 4);
	worldDatabase = new char[worldDatabaseSize];
	void *buf = worldDatabase;
	buf = nboPackUShort(buf, WorldCodeStyle);
	buf = nboPackUShort(buf, 24);
	buf = nboPackUShort(buf, gameStyle);
	buf = nboPackUShort(buf, maxPlayers);
	buf = nboPackUShort(buf, maxShots);
	buf = nboPackUShort(buf, numFlags);
	buf = nboPackFloat(buf, linearAcceleration);
	buf = nboPackFloat(buf, angularAcceleration);
	buf = nboPackUShort(buf, shakeTimeout);
	buf = nboPackUShort(buf, shakeWins);
	// time-of-day will go here
	buf = nboPackUInt(buf, 0);
	if (gameStyle & TeamFlagGameStyle) {
		for (unsigned int i = 1; i < NumTeams; i++) {
			buf = nboPackUShort(buf, WorldCodeBase);
			buf = nboPackUShort(buf, uint16_t(i));
			buf = nboPackVector(buf, basePos[i]);
			buf = nboPackFloat(buf, baseRotation[i]);
			buf = nboPackFloat(buf, baseSize[i][0]);
			buf = nboPackFloat(buf, baseSize[i][1]);
			buf = nboPackVector(buf, RGNMGR_BASE->getSafety(
								static_cast<TeamColor>(i)).get());
		}
	}
	buf = nboPackString(buf, world->getDatabase(), world->getDatabaseSize());
	buf = nboPackUShort(buf, WorldCodeEnd);

	// reset other stuff
	for (unsigned int i = 0; i < NumTeams; i++) {
		team[i].team.size = 0;
		team[i].team.activeSize = 0;
		team[i].team.won = 0;
		team[i].team.lost = 0;
	}
	numFlagsInAir = 0;
	for (int i = 0; i < numFlags; i++)
		resetFlag(i);

	return true;
}

#ifdef PRINTSCORE
static void dumpScore()
{
	int i;

	if (!printScore)
		return;
#ifdef TIMELIMIT
	if (timeLimit > 0.0f)
		std::cout << "#time" << std::endl << int(timeLimit - timeElapsed) << std::endl;
#endif
	std::cout << "#teams" << std::endl;
	for (i = int(RedTeam); i < NumTeams; i++)
		std::cout << team[i].team.won << " " <<
		team[i].team.lost << " " <<
		Team::getName(TeamColor(i)) << std::endl;

	// sort players by team (do it in five easy pieces)
	std::cout << "#players" << std::endl;
	for (i = 0; i < NumTeams; i++)
		for (PlayerId j = 0; j < maxPlayers; j++)
			if (player[j].state > PlayerInLimbo && int(player[j].team) == i) {
				std::cout << player[j].wins << " " <<
						player[j].losses << " " <<
						player[j].callSign << std::endl;
			}

	std::cout << "#end" << std::endl;
}
#endif

static void acceptClient()
{
	// client (not a player yet) is requesting service.
	// accept incoming connection on our well known port
	struct sockaddr_in clientAddr;
	AddrLen addr_len = sizeof(clientAddr);
	int fd = accept(wksSocket, (struct sockaddr*)&clientAddr, &addr_len);
	if (fd == -1) {
		nerror("accepting on wks");
		return;
	}
	DEBUG1("accept() from %s:%d on %i\n",
			inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fd);

	// don't buffer info, send it immediately
	setNoDelay(fd);
	BzfNetwork::setNonBlocking(fd);

	if (fd > maxFileDescriptor)
		maxFileDescriptor = fd;

	// send server version and playerid
	char buffer[9];
	memcpy(buffer, ServerVersion, 8);
	// send 0xff if list is full
	buffer[8] = (char)0xff;

	PlayerId playerId;
	// find open slot in players list
	for (playerId = 0; playerId < maxPlayers; playerId++)
		if (player[playerId].state == PlayerNoExist)
			break;
	if (playerId == maxPlayers) {
		send(fd, (const char*)buffer, sizeof(buffer), 0);
		return;
	}

	buffer[8] = (uint8_t)playerId;
	send(fd, (const char*)buffer, sizeof(buffer), 0);

	// FIXME add new client server welcome packet here when client code is ready

	// update player state
	player[playerId].time = TimeKeeper::getCurrent();
	player[playerId].fd = fd;
	player[playerId].state = PlayerInLimbo;
	player[playerId].tcplen = 0;
	player[playerId].udplen = 0;
	assert(player[playerId].outmsg == NULL);
	player[playerId].outmsgSize = 0;
	player[playerId].outmsgOffset = 0;
	player[playerId].outmsgCapacity = 0;
#ifdef NETWORK_STATS
	initPlayerMessageStats(playerId);
#endif

	// if game was over and this is the first player then game is on
	if (gameOver) {
		int count = 0;
		for (PlayerId i = 0; i < maxPlayers; i++)
			if (player[i].state >= PlayerInLimbo)
				count++;
		if (count == 1) {
			gameOver = false;
#ifdef TIMELIMIT
			gameStartTime = TimeKeeper::getCurrent();
			timeElapsed = 0.0f;
#endif
		}
	}
}

static void sendMessage(PlayerId playerId, PlayerId targetPlayer, const char *message)
{
	// player is sending a message to a particular player, a team, or all.
	// send MsgMessage
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerId);
	buf = nboPackUByte(buf, targetPlayer);
	buf = nboPackString(buf, message, MessageLen);
	broadcastMessage(MsgMessage, (char*)buf-(char*)bufStart, bufStart);
}

static void addPlayer(PlayerId playerId)
{
	// reject player if asks for bogus team or rogue and rogues aren't allowed
	// or if the team is full.

	// look if there is as name clash, we won't allow this
	int i;
	for (i = 0; i < maxPlayers; i++)
	{
		if (i == playerId)
			continue;
		if (strnocasecmp(player[i].callSign,player[playerId].callSign) == 0)
			break;
	}
	if (i < maxPlayers)
	{
		// this is a hack; would better add a new reject type
		player[playerId].team = NoTeam;
	}

	TeamColor t = player[playerId].team;
	if ((t == NoTeam && (player[playerId].type == TankPlayer ||
			player[playerId].type == ComputerPlayer)) ||
			(t == RogueTeam && !(gameStyle & RoguesGameStyle)) ||
			(team[int(t)].team.activeSize >= maxTeam[int(t)])) {
		uint16_t code = RejectBadRequest;
		if (player[playerId].type != TankPlayer &&
				player[playerId].type != ComputerPlayer)
			code = RejectBadType;
		else if (t == NoTeam)
			code = RejectBadTeam;
		else if (t == RogueTeam && !(gameStyle & RoguesGameStyle))
			code = RejectNoRogues;
		else if (team[int(t)].team.activeSize >= maxTeam[int(t)]) {
			// if team is full then check if server is full
			code = RejectServerFull;
			for (unsigned int i = RogueTeam; i < NumTeams; i++)
				if (team[i].team.activeSize < maxTeam[i]) {
					code = RejectTeamFull;
					break;
				}
		}

		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUShort(bufStart, code);
		directMessage(playerId, MsgReject, (char*)buf-(char*)bufStart, bufStart);
		return;
	}

	player[playerId].ulinkup = false;
	player[playerId].toBeKicked = false;
	player[playerId].Admin = false;

	player[playerId].lastRecvPacketNo = 0;
	player[playerId].lastSendPacketNo = 0;

	player[playerId].uqueue = NULL;
	player[playerId].dqueue = NULL;

	player[playerId].lagkillerpending = false;
	player[playerId].lagavg = 0;
	player[playerId].lagcount = 0;
	player[playerId].laglastwarn = 0;
	player[playerId].lagwarncount = 0;
	player[playerId].lagalpha = 1;


	// accept player
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerId);
	directMessage(playerId, MsgAccept, (char*)buf-(char*)bufStart, bufStart);

	// abort if we hung up on the client
	if (player[playerId].fd == NotConnected)
		return;

	// player is signing on (has already connected via addClient).
	player[playerId].state = PlayerDead;
	player[playerId].flag = -1;
	player[playerId].wins = 0;
	player[playerId].losses = 0;

	// update team state and if first active player on team,
	// add team's flag and reset it's score
	bool resetTeamFlag = false;
	int teamIndex = int(player[playerId].team);
	if ((player[playerId].type == TankPlayer ||
			player[playerId].type == ComputerPlayer) &&
			++team[teamIndex].team.activeSize == 1) {
		team[teamIndex].team.won = 0;
		team[teamIndex].team.lost = 0;
		if ((gameStyle & int(TeamFlagGameStyle)) && teamIndex != int(RogueTeam))
			// can't call resetFlag() here cos it'll screw up protocol for
			// player just joining, so do it later
			resetTeamFlag = true;
	}

	// send new player updates on each player, all existing flags, and all teams.
	// watch out for connection being closed because of an error.
	for (unsigned int j = 0; j < NumTeams &&
								player[playerId].fd != NotConnected; j++)
		sendTeamUpdate(j, playerId);
	for (i = 0; i < numFlags && player[playerId].fd != NotConnected; i++)
		if (flag[i].flag.status != FlagNoExist)
			sendFlagUpdate(i, playerId);
	for (i = 0; i < maxPlayers && player[playerId].fd != NotConnected; i++)
		if (player[i].state > PlayerInLimbo && i != playerId)
			sendPlayerUpdate(i, playerId);

	// if new player connection was closed (because of an error) then stop here
	if (player[playerId].fd == NotConnected)
		return;

	// send MsgAddPlayer to everybody -- this concludes MsgEnter response
	// to joining player
	sendPlayerUpdate(playerId, playerId);

	// send update of info for team just joined
	sendTeamUpdate(teamIndex);

	// send time update to new player if we're counting down
	if (timeLimit > 0.0f) {
		float timeLeft = timeLimit - (TimeKeeper::getCurrent() - gameStartTime);
		if (timeLeft < 0.0f) {
			// oops
			timeLeft = 0.0f;
		}

		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
		directMessage(playerId, MsgTimeUpdate, (char*)buf-(char*)bufStart, bufStart);
	}

	// again check if player was disconnected
	if (player[playerId].fd == NotConnected)
		return;

	// reset that flag
	if (resetTeamFlag)
		resetFlag(teamIndex-1);

	// tell the list server the new number of players
	sendMessageToListServer("SETNUM");

#ifdef PRINTSCORE
	dumpScore();
#endif
	char message[MessageLen];

#ifdef SERVERLOGINMSG
	sprintf(message,"BZFlag server %d.%d%c%d, http://BZFlag.org/",
			(VERSION / 10000000) % 100, (VERSION / 100000) % 100,
			(char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000);
	sendMessage(ServerPlayer, playerId, message);

	if (servermsg && (strlen(servermsg) > 0)) {
		sprintf(message,"%s",servermsg);
		sendMessage(ServerPlayer, playerId, message);
	}
#endif
}

static Real getSafeFlagRadius(FlagId id)
{
	// compute safety radius.  flag should not be within this distance
	// of any building.
	Real r = TankRadius;
	if (id == ObesityFlag)
		r *= R_(2.0) * ObeseFactor;
	return r;
}

static bool getLandingPosition(Vec3& p, Real r)
{
	// intersect ray from flag position downwards
	static const Vec3 down(0.0, 0.0, -1.0);
	IntersectionPoint hit;
	const RegionPrimitive* region =
							RGNMGR_OBSTACLE->intersect(hit, Ray(p, down));

	// compute the landing position.  this is the intersection or
	// the spot on the ground under pos if there was no intersection.
	if (region == NULL)
		hit.t = p[2];
	p = p + hit.t * down;

	// no good if jumping is off and flag is above a building
	if ((gameStyle & JumpingGameStyle) == 0 && region != NULL)
		return false;

	// FIXME -- this is hopefully temporary.  it's too hard to grab a
	// flag that's on a pyramid because you can't drive well on one.
	// no good if surface isn't horizontal
	if (region != NULL && hit.normal[2] < R_(0.99999))
		return false;

	// see if landing position is too close to a building
	return !RGNMGR_OBSTACLE->isNear(p, r, region);
}

static bool isGoodFlagSpawn(Vec3& p, Real r)
{
	// no good if point is in a building
	if (RGNMGR_OBSTACLE->isInside(p))
		return false;

	// compute landing position and see if it's okay
	return getLandingPosition(p, r);
}

static bool repositionFlag(int flagIndex)
{
	FlagInfo *pFlagInfo = &flag[flagIndex];
	assert(pFlagInfo->flag.id != NullFlag);

	// pick a valid random position for the flag.  if there are no
	// valid positions for the flag then return false.
	Vec3 p;
	const Real r = getSafeFlagRadius(pFlagInfo->flag.id);
	do {
		if (!RGNMGR_FLAG_SPAWN->spawn(p, pFlagInfo->flag.id))
			return false;
	} while (!isGoodFlagSpawn(p, r));

	// save it
	pFlagInfo->flag.position[0] = p[0];
	pFlagInfo->flag.position[1] = p[1];
	pFlagInfo->flag.position[2] = p[2];

	return true;
}

static bool addFlag(int flagIndex)
{
	assert(flagIndex >= 0);
	assert(flag[flagIndex].flag.id != NullFlag);

	// flag in now entering game
	flag[flagIndex].flag.status = FlagComing;
	numFlagsInAir++;

	// compute drop time
	const float flightTime = 2.0f * sqrtf(-2.0f * FlagAltitude / Gravity);
	flag[flagIndex].flag.flightTime = 0.0f;
	flag[flagIndex].flag.flightEnd = flightTime;
	flag[flagIndex].flag.initialVelocity = -0.5f * Gravity * flightTime;
	flag[flagIndex].dropDone = TimeKeeper::getCurrent();
	flag[flagIndex].dropDone += flightTime;

	// how many times will it stick around
	if (flag[flagIndex].flag.type == FlagSticky)
		flag[flagIndex].grabs = 1;
	else
		flag[flagIndex].grabs = int(floor(4.0f * (float)bzfrand())) + 1;

	// pick a spot for the flag
	return repositionFlag(flagIndex);
}

static bool randomFlag(int flagIndex)
{
	assert(flagIndex >= 0);
	assert(flag[flagIndex].flag.id == NullFlag);

	// pick a random flag
	flag[flagIndex].flag.id = allowedFlags[int(numAllowedFlags * (float)bzfrand())];
	flag[flagIndex].flag.type = Flag::getType(flag[flagIndex].flag.id);
	if (!addFlag(flagIndex)) {
		flag[flagIndex].flag.id = NullFlag;
		return false;
	}

	return true;
}

static void resetFlag(int flagIndex)
{
	// NOTE -- must not be called until world is defined
	assert(world != NULL);
	assert(flagIndex >= 0);

	FlagInfo *pFlagInfo = &flag[flagIndex];
	// reset a flag's info
	pFlagInfo->playerId = InvalidPlayer;
	pFlagInfo->flag.status = FlagNoExist;

	// if it's a random flag, reset flag id
	if (flagIndex >= numFlags - numExtraFlags)
		pFlagInfo->flag.id = NullFlag;

	// required flags mustn't just disappear
	if (pFlagInfo->required) {
		if (int(pFlagInfo->flag.id) >= FirstTeamFlag &&
			int(pFlagInfo->flag.id) <= LastTeamFlag) {
			if (team[int(pFlagInfo->flag.id)].team.activeSize == 0) {
				pFlagInfo->flag.status = FlagNoExist;
			}
			else {
				if (!repositionFlag(flagIndex)) {
					// FIXME -- no spawn area for team flag.  report error.
				}
				pFlagInfo->flag.status = FlagOnGround;
			}
		}
		else if (pFlagInfo->flag.id == NullFlag) {
			// if no spawn area for flag then bail
			if (!randomFlag(flagIndex))
				return;
		}
		else {
			// if no spawn area for flag then bail
			if (!addFlag(flagIndex))
				return;
		}
	}

	sendFlagUpdate(flagIndex);
}

static void zapFlag(int flagIndex)
{
	// called when a flag must just disappear -- doesn't fly
	// into air, just *poof* vanishes.
	assert(flagIndex >= 0);

	// see if someone had grabbed flag.  tell 'em to drop it.
	const PlayerId playerId = flag[flagIndex].playerId;
	if (playerId != InvalidPlayer) {
		flag[flagIndex].playerId = InvalidPlayer;
		flag[flagIndex].flag.status = FlagNoExist;
		player[playerId].flag = -1;

		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, DropReasonForced);
		buf = nboPackUByte(buf, playerId);
		buf = nboPackUShort(buf, uint16_t(flagIndex));
		buf = flag[flagIndex].flag.pack(buf);
		broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
	}

	// if flag was flying then it flies no more
	if (flag[flagIndex].flag.status == FlagInAir ||
			flag[flagIndex].flag.status == FlagComing ||
			flag[flagIndex].flag.status == FlagGoing)
		numFlagsInAir--;

	// reset flag status
	resetFlag(flagIndex);
}

static void removePlayer(PlayerId playerId)
{
	// player is signing off or sent a bad packet.  since the
	// bad packet can come before MsgEnter, we must be careful
	// not to undo operations that haven't been done.
	// first shutdown connection

	// check if we are called again for a dropped player!
	if (player[playerId].fd == NotConnected)
		return;

	// status message
	DEBUG1("Player %s [%d] is removed\n",player[playerId].callSign,playerId);

	// shutdown TCP socket
	shutdown(player[playerId].fd, 2);
	closesocket(player[playerId].fd);
	player[playerId].fd = NotConnected;

	// free up the UDP packet buffers
	if (player[playerId].uqueue)
		disqueuePacket(playerId, SEND, 65536);
	if (player[playerId].dqueue)
		disqueuePacket(playerId, RECEIVE, 65536);

	player[playerId].uqueue = NULL;
	player[playerId].dqueue = NULL;
	player[playerId].lastRecvPacketNo = 0;
	player[playerId].lastSendPacketNo = 0;

	// shutdown the UDP socket
	memset(&player[playerId].uaddr,0,sizeof(player[playerId].uaddr));

	// no UDP connection anymore
	player[playerId].ulinkup = false;
	player[playerId].toBeKicked = false;
	player[playerId].udplen = 0;

	player[playerId].tcplen = 0;

	player[playerId].callSign[0] = 0;

	if (player[playerId].outmsg != NULL) {
		delete[] player[playerId].outmsg;
		player[playerId].outmsg = NULL;
	}

	// player is outta here.  if player never joined a team then
	// don't count as a player.
	if (player[playerId].state == PlayerInLimbo) {
		player[playerId].state = PlayerNoExist;
		return;
	}

	player[playerId].state = PlayerNoExist;

	if (player[playerId].team != NoTeam) {
		// if player had flag then flag just disappears.  it'd be nice
		// to have it fly as if dropped, but we've no idea what the
		// player's position is.
		if (player[playerId].flag != -1)
			zapFlag(player[playerId].flag);

		// tell everyone player has left
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, playerId);
		broadcastMessage(MsgRemovePlayer, (char*)buf-(char*)bufStart, bufStart);

		// decrease team size
		int teamNum = int(player[playerId].team);
		--team[teamNum].team.size;
		if (player[playerId].type == TankPlayer ||
				player[playerId].type == ComputerPlayer)
			--team[teamNum].team.activeSize;

		// if last active player on team then remove team's flag
		if (teamNum != int(RogueTeam) &&
				(player[playerId].type == TankPlayer ||
				player[playerId].type == ComputerPlayer) &&
				team[teamNum].team.activeSize == 0 &&
				(gameStyle & int(TeamFlagGameStyle)))
			zapFlag(teamNum - 1);

		// send team update
		sendTeamUpdate(teamNum);
	}

#ifdef NETWORK_STATS
	dumpPlayerMessageStats(playerId);
#endif
	// tell the list server the new number of players
	sendMessageToListServer("SETNUM");

	// anybody left?
	int i;
	for (i = 0; i < maxPlayers; i++)
		if (player[i].state > PlayerInLimbo)
			break;

	// if everybody left then reset world
	if (i == maxPlayers) {
		if (oneGameOnly) {
			done = true;
			exitCode = 0;
		}
		else if (!defineWorld()) {
			done = true;
			exitCode = 1;
		}
		else {
			// republicize ourself.  this dereferences the URL chain
			// again so we'll notice any pointer change when any game
			// is over (i.e. all players have quit).
			publicize();
		}
	}
}

static void sendWorld(PlayerId playerId, int ptr)
{
	// send another small chunk of the world database
	assert(world != NULL && worldDatabase != NULL);
	void *buf, *bufStart = getDirectMessageBuffer();
	int size = 256, left = worldDatabaseSize - ptr;
	if (ptr >= worldDatabaseSize) {
		size = 0;
		left = 0;
	}
	else {
		if (ptr + size >= worldDatabaseSize)
			size = worldDatabaseSize - ptr;
	}
	buf = nboPackUShort(bufStart, uint16_t(left));
	buf = nboPackString(buf, (char*)worldDatabase + ptr, size);
	directMessage(playerId, MsgGetWorld, (char*)buf-(char*)bufStart, bufStart);
}

static void playerAlive(PlayerId playerId, const float *pos, const float *fwd)
{
	// player is coming alive.
	player[playerId].state = PlayerAlive;
	player[playerId].flag = -1;

	// send MsgAlive
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerId);
	buf = nboPackVector(buf,pos);
	buf = nboPackVector(buf,fwd);
	broadcastMessage(MsgAlive, (char*)buf-(char*)bufStart, bufStart);
}

static void checkTeamScore(PlayerId playerId, int teamIndex)
{
	if (maxTeamScore == 0 || teamIndex == (int)RogueTeam) return;
	if (team[teamIndex].team.won - team[teamIndex].team.lost >= maxTeamScore) {
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, playerId);
		buf = nboPackUShort(buf, uint16_t(teamIndex));
		broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
		gameOver = true;
	}
}

static void playerKilled(int victimIndex, int killerIndex,
														int16_t shotIndex)
{
	// victim has been destroyed.  keep score.
	if (killerIndex == InvalidPlayer ||
		player[victimIndex].state != PlayerAlive) return;
	player[victimIndex].state = PlayerDead;

	// send MsgKilled
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, victimIndex);
	buf = nboPackUByte(buf, killerIndex);
	buf = nboPackShort(buf, shotIndex);
	broadcastMessage(MsgKilled, (char*)buf-(char*)bufStart, bufStart);

	// we try to estimate lag by measuring time between broadcast of MsgKilled
	// and MsgScore reply of the killer; should give us a good approximation
	//  of the killer's round trip time
	if (victimIndex != killerIndex &&            // suicide doesn't change score twice
			!player[killerIndex].lagkillerpending) { // two rapid kills
		player[killerIndex].lagkillerpending = true;
		player[killerIndex].lagkillertime = TimeKeeper::getCurrent();
	}

	// zap flag player was carrying.  clients should send a drop flag
	// message before sending a killed message, so this shouldn't happen.
	if (player[victimIndex].flag != -1)
		zapFlag(player[victimIndex].flag);

	// change the team scores -- rogues don't have team scores.  don't
	// change team scores for individual player's kills in capture the
	// flag mode.
	int winningTeam = (int)NoTeam;
	if (!(gameStyle & TeamFlagGameStyle)) {
		if (player[victimIndex].team == player[killerIndex].team) {
			if (player[killerIndex].team != RogueTeam)
				if (killerIndex == victimIndex)
					team[int(player[victimIndex].team)].team.lost += 1;
				else
					team[int(player[victimIndex].team)].team.lost += 2;
		}
		else {
			if (player[killerIndex].team != RogueTeam) {
				winningTeam = int(player[killerIndex].team);
				team[winningTeam].team.won++;
			}
			if (player[victimIndex].team != RogueTeam)
				team[int(player[victimIndex].team)].team.lost++;
			sendTeamUpdate(int(player[killerIndex].team));
		}
		sendTeamUpdate(int(player[victimIndex].team));
	}
#ifdef PRINTSCORE
	dumpScore();
#endif
	if (winningTeam != (int)NoTeam)
		checkTeamScore(killerIndex, winningTeam);
}

static void grabFlag(PlayerId playerId, int flagIndex)
{
	// player wants to take possession of flag
	if (player[playerId].state != PlayerAlive ||
			player[playerId].flag != -1 ||
			flag[flagIndex].flag.status != FlagOnGround)
		return;
	// okay, player can have it
	flag[flagIndex].flag.status = FlagOnTank;
	flag[flagIndex].flag.owner = playerId;
	flag[flagIndex].playerId = playerId;
	player[playerId].flag = flagIndex;

	// send MsgGrabFlag
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerId);
	buf = nboPackUShort(buf, uint16_t(flagIndex));
	buf = flag[flagIndex].flag.pack(buf);
	broadcastMessage(MsgGrabFlag, (char*)buf-(char*)bufStart, bufStart);
}

static void dropFlag(PlayerId playerId, FlagDropReason reason, float pos[3])
{
	assert(world != NULL);

	// player wants to drop flag.  we trust that the client won't tell
	// us to drop a sticky flag until the requirements are satisfied.
	const int flagIndex = player[playerId].flag;
	FlagInfo *pFlagInfo = &flag[flagIndex];
	if (flagIndex == -1 || pFlagInfo->flag.status != FlagOnTank)
		return;

	// okay, go ahead and drop it
	pFlagInfo->playerId = InvalidPlayer;
	if (pFlagInfo->flag.type == FlagNormal || --flag[flagIndex].grabs > 0)
		pFlagInfo->flag.status = FlagInAir;
	else
		pFlagInfo->flag.status = FlagGoing;
	numFlagsInAir++;

	// flags are dropped from the top of the tank
	pos[2] += TankHeight;

	// get landing position
	Vec3 landingPos = pos;
	const FlagId flagId = pFlagInfo->flag.id;
	bool okay = getLandingPosition(landingPos, getSafeFlagRadius(flagId));

	// figure out landing spot -- if flag in a Bad Place
	// when dropped, move to safety position or make it going
	TeamColor teamBase = RGNMGR_BASE->isInside(landingPos);
	bool isTeamFlag = (flagId >= FirstTeamFlag) && (flagId <= LastTeamFlag);
	if (pFlagInfo->flag.status == FlagGoing) {
		pFlagInfo->flag.landingPosition[0] = pos[0];
		pFlagInfo->flag.landingPosition[1] = pos[1];
		pFlagInfo->flag.landingPosition[2] = pos[2];
	}
	else if (isTeamFlag && teamBase != NoTeam) {
		// team flag landing on a team base.  if it's the flag's
		// base, fine.  if not then use the safety position.
		if ((TeamColor)flagId == teamBase) {
			pFlagInfo->flag.landingPosition[0] = landingPos[0];
			pFlagInfo->flag.landingPosition[1] = landingPos[1];
			pFlagInfo->flag.landingPosition[2] = landingPos[2];
		}
		else {
			const Vec3& p = RGNMGR_BASE->getSafety(teamBase);
			pFlagInfo->flag.landingPosition[0] = p[0];
			pFlagInfo->flag.landingPosition[1] = p[1];
			pFlagInfo->flag.landingPosition[2] = p[2];
		}
	}
	else {
		// flag is falling onto a building or the ground.  we need to
		// make sure it's not falling too close to a building.
		if (okay) {
			// position was fine
			pFlagInfo->flag.landingPosition[0] = landingPos[0];
			pFlagInfo->flag.landingPosition[1] = landingPos[1];
			pFlagInfo->flag.landingPosition[2] = landingPos[2];
		}
		else if (isTeamFlag) {
			// can't find a good position and it's a team flag,
			// which must not disappear.  try around center of
			// the game board.
			Matrix xform;
			xform.setTranslate(0.0, 0.0, 100.0);
			TransformedShape tmp(new ShapeBox(10.0, 10.0, 100.0), xform);
			Vec3 p;
			bool okay = false;
			static const unsigned int NumRetries = 100;
			for (unsigned int i = 0; !okay && i < NumRetries; ++i) {
				tmp.getRandomPoint(p);
				okay = isGoodFlagSpawn(p, getSafeFlagRadius(flagId));
			}

			// if we found a good spot then use it, otherwise use
			// the team flag spawn region.
			if (okay) {
				pFlagInfo->flag.landingPosition[0] = p[0];
				pFlagInfo->flag.landingPosition[1] = p[1];
				pFlagInfo->flag.landingPosition[2] = p[2];
			}
			else if (!repositionFlag(flagIndex)) {
				// uh oh, can't find a place for the flag!
				// FIXME -- do something!
			}
		}
		else {
			// can't find a good place.  make the flag go away.
			pFlagInfo->flag.status             = FlagGoing;
			pFlagInfo->flag.landingPosition[0] = pos[0];
			pFlagInfo->flag.landingPosition[1] = pos[1];
			pFlagInfo->flag.landingPosition[2] = pos[2];
		}
	}

	pFlagInfo->flag.position[0] = pFlagInfo->flag.landingPosition[0];
	pFlagInfo->flag.position[1] = pFlagInfo->flag.landingPosition[1];
	pFlagInfo->flag.position[2] = pFlagInfo->flag.landingPosition[2];
	pFlagInfo->flag.launchPosition[0] = pos[0];
	pFlagInfo->flag.launchPosition[1] = pos[1];
	pFlagInfo->flag.launchPosition[2] = pos[2] + TankHeight;

	// compute flight info -- flight time depends depends on start and end
	// altitudes and desired height above start altitude.
	const float thrownAltitude = (pFlagInfo->flag.id == ShieldFlag) ?
								ShieldFlight * FlagAltitude : FlagAltitude;
	const float maxAltitude = pos[2] + thrownAltitude;
	const float upTime = sqrtf(-2.0f * thrownAltitude / Gravity);
	const float downTime = sqrtf(-2.0f * (maxAltitude -
								pFlagInfo->flag.landingPosition[2]) / Gravity);
	const float flightTime = upTime + downTime;

	// set flight info
	pFlagInfo->dropDone = TimeKeeper::getCurrent();
	pFlagInfo->dropDone += flightTime;
	pFlagInfo->flag.flightTime = 0.0f;
	pFlagInfo->flag.flightEnd = flightTime;
	pFlagInfo->flag.initialVelocity = -Gravity * upTime;

	// player no longer has flag -- send MsgDropFlag
	player[playerId].flag = -1;
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, reason);
	buf = nboPackUByte(buf, playerId);
	buf = nboPackUShort(buf, uint16_t(flagIndex));
	buf = flag[flagIndex].flag.pack(buf);
	broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);

	// notify of new flag state
	sendFlagUpdate(flagIndex);
}

static void captureFlag(PlayerId playerId, float pos[3])
{
	// player captured a flag.  can either be enemy flag in player's own
	// team base, or player's own flag in enemy base.
	int flagIndex = int(player[playerId].flag);
	if (flagIndex < 0 ||
			int(flag[flagIndex].flag.id) < int(FirstTeamFlag) ||
			int(flag[flagIndex].flag.id) > int(LastTeamFlag))
		return;

	// player no longer has flag and put flag back at it's base
	player[playerId].flag = -1;
	resetFlag(flagIndex);

	// send MsgDropFlag
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, DropReasonCaptured);
	buf = nboPackUByte(buf, playerId);
	buf = nboPackUShort(buf, uint16_t(flagIndex));
	buf = flag[flagIndex].flag.pack(buf);
	broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);

	// everyone on losing team is dead
	for (PlayerId i = 0; i < maxPlayers; i++)
		if (player[i].fd != NotConnected &&
			int(flag[flagIndex].flag.id) == int(player[i].team) &&
			player[i].state == PlayerAlive) {
			player[i].state = PlayerDead;
		}

	// update score (rogues can't capture flags)
	int winningTeam = (int)NoTeam;
	if (int(flag[flagIndex].flag.id) != int(player[playerId].team)) {
		// player captured enemy flag
		winningTeam = int(player[playerId].team);
		team[winningTeam].team.won++;
		sendTeamUpdate(winningTeam);
	}
	team[int(flag[flagIndex].flag.id)].team.lost++;
	sendTeamUpdate(int(flag[flagIndex].flag.id));
#ifdef PRINTSCORE
	dumpScore();
#endif
	if (winningTeam != (int)NoTeam)
		checkTeamScore(playerId, winningTeam);
}

static void shotFired(int /*playerId*/, const void *buf, int len)
{
	// player has fired shot -- send MsgShotBegin
	broadcastMessage(MsgShotBegin, len, buf);
}

static void shotEnded(PlayerId id, int16_t shotIndex, uint16_t reason)
{
	// shot has ended prematurely -- send MsgShotEnd
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, id);
	buf = nboPackShort(buf, shotIndex);
	buf = nboPackUShort(buf, reason);
	broadcastMessage(MsgShotEnd, (char*)buf-(char*)bufStart, bufStart);
}

static void scoreChanged(PlayerId playerId, uint16_t wins, uint16_t losses)
{
	// lag measurement
	// got reference time?
	if (player[playerId].lagkillerpending) {
		PlayerInfo &killer = player[playerId];
		TimeKeeper now = TimeKeeper::getCurrent(),&then = killer.lagkillertime;
		float timepassed = now-then;
		killer.lagkillerpending = false;
		// huge lags might be error!?
		if (timepassed < 10.0) {
			// time is smoothed exponentially using a dynamic smoothing factor
			killer.lagavg = killer.lagavg*(1-killer.lagalpha)+killer.lagalpha*timepassed;
			killer.lagalpha = killer.lagalpha/(0.9f+killer.lagalpha);
			killer.lagcount++;
			// warn players from time to time whose lag is > threshold (-lagwarn)
			if (lagwarnthresh > 0 && killer.lagavg > lagwarnthresh &&
					killer.lagcount-killer.laglastwarn > 5 + 2 * killer.lagwarncount) {
				char message[MessageLen];
				sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***",
						int(killer.lagavg * 1000));
				sendMessage(ServerPlayer, playerId,	message);
				killer.laglastwarn = killer.lagcount;
				killer.lagwarncount++;;
				if (killer.lagwarncount++ > maxlagwarn) {
					// drop the player
					sprintf(message,"You have been kicked due to excessive lag (you have been warned %d times).",
						maxlagwarn);
					sendMessage(ServerPlayer, playerId, message);
					removePlayer(playerId);
				}
			}
		}
	}

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerId);
	buf = nboPackUShort(buf, wins);
	buf = nboPackUShort(buf, losses);
	broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);
	player[playerId].wins = wins;
	player[playerId].losses = losses;
#ifdef PRINTSCORE
	dumpScore();
#endif

	// see if the player reached the score limit
	if (maxPlayerScore != 0 &&
			player[playerId].wins - player[playerId].losses >= maxPlayerScore) {
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, playerId);
		buf = nboPackUShort(buf, uint16_t(NoTeam));
		broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
		gameOver = true;
	}
}

static void sendTeleport(PlayerId playerId, uint16_t from, uint16_t to)
{
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerId);
	buf = nboPackUShort(buf, from);
	buf = nboPackUShort(buf, to);
	broadcastMessage(MsgTeleport, (char*)buf-(char*)bufStart, bufStart);
}

// parse player comands (messages with leading /)
static void parseCommand(const char *message, int t)
{
	// /password command allows player to become operator
	if (strncmp(message + 1,"password ",9) == 0) {
		if (password && strncmp(message + 10, password, strlen(password)) == 0) {
			player[t].Admin = true;
			sendMessage(ServerPlayer, t, "You are now an administrator!");
		} else {
			sendMessage(ServerPlayer, t, "Wrong Password!");
		}
	// /shutdownserver terminates the server
	} else if (player[t].Admin && strncmp(message + 1, "shutdownserver", 8) == 0) {
		done = true;
	// /superkill closes all player connections
	} else if (player[t].Admin && strncmp(message + 1, "superkill", 8) == 0) {
		for (PlayerId i = 0; i < MaxPlayers; i++)
			directMessage(i, MsgSuperKill, 0, getDirectMessageBuffer());
		gameOver = true;
	// /gameover command allows operator to end the game
	} else if (player[t].Admin && strncmp(message + 1, "gameover", 8) == 0) {
		void *buf, *bufStart = getDirectMessageBuffer();
		buf = nboPackUByte(bufStart, t);
		buf = nboPackUShort(buf, uint16_t(NoTeam));
		broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
		gameOver = true;
	// /flag command allows operator to control flags
	} else if (player[t].Admin && strncmp(message + 1, "flag ", 5) == 0) {
		if (strncmp(message + 6, "reset", 5) == 0) {
			for (int i = 0; i < numFlags; i++) {
				// see if someone had grabbed flag.  tell 'em to drop it.
				const PlayerId playerId = flag[i].playerId;
				if (playerId != InvalidPlayer) {
					flag[i].playerId = InvalidPlayer;
					flag[i].flag.status = FlagNoExist;
					player[playerId].flag = -1;

					void *buf, *bufStart = getDirectMessageBuffer();
					buf = nboPackUByte(bufStart, DropReasonForced);
					buf = nboPackUByte(buf, playerId);
					buf = nboPackUShort(buf, uint16_t(i));
					buf = flag[i].flag.pack(buf);
					broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
				}
				resetFlag(i);
			}
		} else if (strncmp(message + 6, "up", 2) == 0) {
			for (int i = 0; i < numFlags; i++) {
				if (int(flag[i].flag.id) < int(FirstTeamFlag) ||
						int(flag[i].flag.id) > int(LastTeamFlag)) {
					// see if someone had grabbed flag.  tell 'em to drop it.
					const PlayerId playerId = flag[i].playerId;
					if (playerId != InvalidPlayer) {
						flag[i].playerId = InvalidPlayer;
						flag[i].flag.status = FlagNoExist;
						player[playerId].flag = -1;

						void *buf, *bufStart = getDirectMessageBuffer();
						buf = nboPackUByte(bufStart, DropReasonForced);
						buf = nboPackUByte(buf, playerId);
						buf = nboPackUShort(buf, uint16_t(i));
						buf = flag[i].flag.pack(buf);
						broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
					}
					flag[i].flag.status = FlagGoing;
					if (!flag[i].required)
						flag[i].flag.id = NullFlag;
					sendFlagUpdate(i);
				}
			}
		} else if (strncmp(message + 6, "show", 4) == 0) {
			for (int i = 0; i < numFlags; i++) {
				char message[MessageLen]; // FIXME
				sprintf(message, "%d p:%d r:%d g:%d i:%s s:%d p:%3.1fx%3.1fx%3.1f",
						i, flag[i].playerId, flag[i].required, flag[i].grabs,
						Flag::getAbbreviation(flag[i].flag.id),
						flag[i].flag.status,
						flag[i].flag.position[0],
						flag[i].flag.position[1],
						flag[i].flag.position[2]);
				sendMessage(ServerPlayer, t, message);
			}
		}
	// /kick command allows operator to remove players
	} else if (player[t].Admin && strncmp(message + 1, "kick ", 5) == 0) {
		int i;
		const char *victimname = message + 6;
		for (i = 0; i < maxPlayers; i++)
			if (player[i].fd != NotConnected && strcmp(player[i].callSign, victimname) == 0)
				break;
		if (i < maxPlayers) {
			char kickmessage[MessageLen];
			player[i].toBeKicked = false;
			sprintf(kickmessage,"Your were kicked off the server by %s", player[t].callSign);
			sendMessage(ServerPlayer, i, kickmessage);
			removePlayer(i);
		} else {
			char errormessage[MessageLen];
			sprintf(errormessage, "player %s not found", victimname);
			sendMessage(ServerPlayer, t, errormessage);
		}
	}
	// /lagstats gives simple statistics about players' lags
	else if (strncmp(message+1,"lagstats",8) == 0) {
		for (PlayerId i = 0; i < maxPlayers; i++)
			if (player[i].fd != NotConnected) {
				char reply[MessageLen];
				sprintf(reply, "%-12s(%d) : %4dms (%d)", player[i].callSign,
						i, int(player[i].lagavg*1000), player[i].lagcount);
				sendMessage(ServerPlayer, t, reply);
			}
	}
	else {
		sendMessage(ServerPlayer, t, "unknown command");
	}
}

static void handleCommand(int t, uint16_t code, uint16_t len, void *rawbuf)
{
	void *buf = (void*)((char*)rawbuf + 4);
#ifdef NETWORK_STATS
	countMessage(t, code, len, 0);
#endif
	switch (code) {
		// player joining
		case MsgEnter: {
			// data: type, team, name, email
			uint16_t type, team;
			buf = nboUnpackUShort(buf, type);
			buf = nboUnpackUShort(buf, team);
			player[t].type = PlayerType(type);
			player[t].team = TeamColor(team);
			buf = nboUnpackString(buf, player[t].callSign, CallSignLen);
			buf = nboUnpackString(buf, player[t].email, EmailLen);
			addPlayer(t);
			DEBUG1("Player %s [%d] has joined\n", player[t].callSign, t);
			break;
		}

		// player closing connection
		case MsgExit:
			// data: <none>
			DEBUG4("Player %s [%d] has dropped\n", player[t].callSign, t);
			DEBUG4("REMOVE: DROPPED BY REQUEST\n");
			removePlayer(t);
			break;

		// player wants more of world database
		case MsgGetWorld: {
			// data: count (bytes read so far)
			// worlds shouldn't be too big, 64k is plenty (famous last words)
			uint16_t ptr;
			buf = nboUnpackUShort(buf, ptr);
			if (ptr == 0) {
				// update time of day in world database
				const uint32_t epochOffset = (uint32_t)time(NULL);
				void *epochPtr = ((char*)worldDatabase) + 24;
				nboPackUInt(epochPtr, epochOffset);
			}
			sendWorld(t, int(ptr));
			break;
		}

		// player is coming alive
		case MsgAlive: {
			// data: position, forward-vector
			float pos[3], fwd[3];
			buf = nboUnpackVector(buf, pos);
			buf = nboUnpackVector(buf, fwd);
			playerAlive(t, pos, fwd);
			break;
		}

		// player declaring self destroyed
		case MsgKilled: {
			// data: id of killer, shot id of killer
			PlayerId killer;
			int16_t shot;
			buf = nboUnpackUByte(buf, killer);
			buf = nboUnpackShort(buf, shot);
			playerKilled(t, killer, shot);
			break;
		}

		// player requesting to grab flag
		case MsgGrabFlag: {
			// data: flag index
			uint16_t flag;
			buf = nboUnpackUShort(buf, flag);
			grabFlag(t, int(flag));
			break;
		}

		// player requesting to drop flag
		case MsgDropFlag: {
			// data: position of drop
			uint8_t reason;
			float pos[3];
			buf = nboUnpackUByte(buf, reason);
			buf = nboUnpackVector(buf, pos);
			if (reason == DropReasonCaptured)
				captureFlag(t, pos);
			else
				dropFlag(t, (FlagDropReason)reason, pos);
			break;
		}

		// shot fired
		case MsgShotBegin:
			// data: firing info
			// special case -- don't unpack firing info cos we just pass it on
			shotFired(t, buf, int(len));
			break;

		// shot ended prematurely
		case MsgShotEnd: {
			// data: shooter id, shot number, reason
			PlayerId sourcePlayer;
			int16_t shot;
			uint16_t reason;
			buf = nboUnpackUByte(buf, sourcePlayer);
			buf = nboUnpackShort(buf, shot);
			buf = nboUnpackUShort(buf, reason);
			shotEnded(sourcePlayer, shot, reason);
			break;
		}

		// player score changed
		case MsgScore: {
			// data: wins, losses
			uint16_t wins, losses;
			buf = nboUnpackUShort(buf, wins);
			buf = nboUnpackUShort(buf, losses);
			scoreChanged(t, wins, losses);
			break;
		}

		// player teleported
		case MsgTeleport: {
			uint16_t from, to;
			buf = nboUnpackUShort(buf, from);
			buf = nboUnpackUShort(buf, to);
			sendTeleport(t, from, to);
			break;
		}

		// player sending a message
		case MsgMessage: {
			// data: target player, target team, message string
			PlayerId sourcePlayer,targetPlayer;
			char message[MessageLen];
			buf = nboUnpackUByte(buf, sourcePlayer); // will ignore this
			buf = nboUnpackUByte(buf, targetPlayer);
			buf = nboUnpackString(buf, message, sizeof(message));
			DEBUG4("Player %s [%d]: %s\n",player[t].callSign, t, message);
			// check for command
			if (message[0] == '/') {
				parseCommand(message, t);
			}
			else
				sendMessage(t, targetPlayer, message);
			break;
		}

		// player is ready to receive data over UDP connection
		case MsgUDPLinkEstablished: {
			uint16_t queueUpdate;
			buf = nboUnpackUShort(buf, queueUpdate);
			OOBQueueUpdate(t, queueUpdate);
			DEBUG3("STATUS: Up UDP CON received\n");
			break;
		}

		// just forward these. FIXME playerid should get overwritten someplace
		case MsgPlayerUpdate:
		case MsgShotUpdate:
			relayPlayerPacket(t, len, rawbuf);
			break;
	}
}

static void terminateServer(Signal /*sig*/)
{
	exitCode = 0;
	done     = true;
}

static const char *usageString =
"[-a <vel> <rot>] [-b] [-c] [+f {good|<id>}] [-f {bad|<id>}] [-g] "
"[-h] "
"[-i interface] "
"[-j] "
"[-lagdrop <num>] "
"[-lagwarn <time/ms>] "
"[-mp {<count>|[<count>],[<count>],[<count>],[<count>],[<count>]}] "
"[-mps <score>] "
"[-ms <shots>] "
"[-mts <score>] "
"[-p <port>] "
"[-noudp] "
"[-passwd <password>] "
#ifdef PRINTSCORE
"[-printscore] "
#endif
"[-public <server-description>] "
"[-publicaddr <server-hostname>[:<server-port>]] "
"[-publiclist <list-server-url>] "
"[+r] "
"[-r] "
"[{+s|-s} [<num>]] "
"[-sa] "
"[-st <time>] [-sw <num>] [-synctime] "
"[-t] "
#ifdef TIMELIMIT
"[-time <seconds>] "
#endif
"[-srvmsg <text>] "
"[-world <filename>]";


static void printVersion(std::ostream& out)
{
	out << copyright << std::endl;

	out << "BZFlag server, version " <<
			(VERSION / 10000000) % 100 << "." <<
			(VERSION / 100000) % 100 <<
			(char)('a' - 1 + (VERSION / 1000) % 100) <<
			VERSION % 1000 <<
			std::endl;

	out << "  protocol " << ServerVersion[4] << ".";
	if (ServerVersion[5] != '0')
		out << ServerVersion[5];
	out << ServerVersion[6] << (char)tolower(ServerVersion[7]) << std::endl;
}

static void usage(const char *pname)
{
	printVersion(std::cerr);
	std::cerr << "usage: " << pname << " " << usageString << std::endl;
	std::cerr << "\t -help: for more info" << std::endl;
	exit(1);
}

static void extraUsage(const char *pname)
{
	printVersion(std::cout);
	std::cout << "usage: " << pname << " " << usageString << std::endl;
	std::cout << "\t -a: maximum acceleration settings" << std::endl;
	std::cout << "\t -b: randomly oriented buildings" << std::endl;
	std::cout << "\t -c: capture-the-flag style game" << std::endl;
//  std::cout << "\t -d: increase debugging level" << std::endl;
	std::cout << "\t +f: always have flag <id> available" << std::endl;
	std::cout << "\t -f: never randomly generate flag <id>" << std::endl;
	std::cout << "\t -g: serve one game and then exit" << std::endl;
	std::cout << "\t -h: use random building heights" << std::endl;
	std::cout << "\t -i: listen on <interface>" << std::endl;
	std::cout << "\t -j: allow jumping" << std::endl;
	std::cout << "\t -mp: maximum players total or per team" << std::endl;
	std::cout << "\t -mps: set player score limit on each game" << std::endl;
	std::cout << "\t -ms: maximum simultaneous shots per player" << std::endl;
	std::cout << "\t -mts: set team score limit on each game" << std::endl;
	std::cout << "\t -noudp: never use the new UDP networking" << std::endl;
	std::cout << "\t -p: use alternative port (default is " <<
										ServerPort << ")" << std::endl;
#ifdef PRINTSCORE
	std::cout << "\t -printscore: write score to stdout whenever it changes" << std::endl;
#endif
	std::cout << "\t -public <server-description>" << std::endl;
	std::cout << "\t -publicaddr <effective-server-hostname>[:<effective-server-port>]" << std::endl;
	std::cout << "\t -publiclist <list-server-url>" << std::endl;
	std::cout << "\t -q: don't listen for or respond to pings" << std::endl;
	std::cout << "\t +r: all shots ricochet" << std::endl;
	std::cout << "\t -r: allow rogue tanks" << std::endl;
	std::cout << "\t +s: always have <num> super flags (default=16)" << std::endl;
	std::cout << "\t -s: allow up to <num> super flags (default=16)" << std::endl;
	std::cout << "\t -sa: insert antidote superflags" << std::endl;
	std::cout << "\t -srvmsg: specify a <msg> to print upon client login" << std::endl;
	std::cout << "\t -st: shake bad flags in <time> seconds" << std::endl;
	std::cout << "\t -sw: shake bad flags after <num> wins" << std::endl;
	std::cout << "\t -synctime: synchronize time of day on all clients" << std::endl;
	std::cout << "\t -t: allow teleporters" << std::endl;
#ifdef TIMELIMIT
	std::cout << "\t -time: set time limit on each game" << std::endl;
#endif
	std::cout << "\t -world: world file to load" << std::endl;
	std::cout << "\t -passwd: specify a <password> for operator commands" << std::endl;
	std::cout << "\t -lagwarn: lag warning threshhold time [ms]" << std::endl;
	std::cout << "\t -lagdrop: drop player after this many lag warnings" << std::endl;
	std::cout << "\nFlag codes:" << std::endl;
	for (int f = int(FirstSuperFlag); f <= int(LastSuperFlag); f++)
		std::cout << "\t " << std::setw(2) << Flag::getAbbreviation(FlagId(f)) <<
		" " << std::setw(0) << Flag::getName(FlagId(f)) << std::endl;
	exit(0);
}

static int lookupFlag(const char *code)
{
	int f = atoi(code);

	if (f == 0)
		for (f = int(FirstSuperFlag); f <= int(LastSuperFlag); f++)
			if (strnocasecmp(code, Flag::getAbbreviation(FlagId(f))) == 0)
				break;
	if (f < int(FirstSuperFlag) || f > int(LastSuperFlag))
		f = int(NoFlag);
	return f;
}

static bool parsePlayerCount(const char *argv)
{
	// either a single number or 5 optional numbers separated by 4
	// (mandatory) commas.
	const char *scan = argv;
	while (*scan && *scan != ',') scan++;
	if (*scan == ',') {
		// okay, it's the comma separated list.  count commas
		int commaCount = 1;
		while (*++scan)
			if (*scan == ',')
				commaCount++;
		if (commaCount != 4) {
			std::cerr << "improper player count list" << std::endl;
			return false;
		}

		// reset the counts
		// no limits by default
		for (unsigned int i = 0; i < NumTeams; i++)
			maxTeam[i] = MaxPlayers;

		// now get the new counts

		// number of counts given
		unsigned int countCount = 0;
		scan = argv;
		for (unsigned int i = 0; i < NumTeams; i++) {
			char *tail;
			long count = strtol(scan, &tail, 10);
			if (tail != scan) {
				// got a number
				countCount++;
				if (count < 0)
					maxTeam[i] = 0;
				else
					if (count > MaxPlayers)
						maxTeam[i] = MaxPlayers;
				else
					maxTeam[i] = uint16_t(count);
			}
			while (*tail && *tail != ',') tail++;
			scan = tail + 1;
		}

		// if rogues not allowed then zero that count
		if (!(gameStyle & int(RoguesGameStyle)))
			maxTeam[RogueTeam] = 0;

		// if all counts explicitly listed then add 'em up and set maxPlayers
		if (countCount == NumTeams) {
			maxPlayers = 0;
			for (unsigned int i = 0; i < NumTeams; i++)
				maxPlayers += maxTeam[i];
		}
	}
	else {
		char *tail;
		long count = strtol(argv, &tail, 10);
		if (argv == tail) {
			std::cerr << "improper player count" << std::endl;
			return false;
		}
		if (count < 1)
			maxPlayers = 1;
		else
			if (count > MaxPlayers)
				maxPlayers = MaxPlayers;
		else maxPlayers = uint16_t(count);
	}
	return true;
}

static bool setRequiredFlag(FlagInfo& flag, FlagId id)
{
	flag.required = true;
	flag.flag.id = id;
	flag.flag.type = Flag::getType(id);
	return true;
}

static void parse(int argc, char **argv)
{
	// initialize state
	gameStyle = PlainGameStyle;
	randomBoxes = false;
	randomHeights = false;
	useTeleporters = false;
	oneGameOnly = false;
	numExtraFlags = 0;
	maxShots = 1;
	linearAcceleration = 0.0f;
	angularAcceleration = 0.0f;
	numAllowedFlags = 0;
	delete[] flag;  flag = NULL;
	delete[] allowedFlags;  allowedFlags = NULL;

	// prepare flag counts
	int f, flagCount[LastFlag + 1];
	bool flagDisallowed[LastFlag + 1];
	for (int i = int(FirstFlag); i <= int(LastFlag); i++) {
		flagCount[i] = 0;
		flagDisallowed[i] = false;
	}
	bool allFlagsOut = false;

	// prepare team max counts
	// no limits by default
	for (unsigned int i = 0; i < NumTeams; i++)
		maxTeam[i] = MaxPlayers;

	// parse command line
	int playerCountArg = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-srvmsg") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for -srvmsg" << std::endl;
				usage(argv[0]);
			}
			servermsg = argv[i];
		}
		else if (strcmp(argv[i], "-world") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for -world" << std::endl;
				usage(argv[0]);
			}
			worldFile = argv[i];
		}
		else if (strcmp(argv[i], "+f") == 0) {
			// add required flag
			if (++i == argc) {
				std::cerr << "argument expected for +f" << std::endl;
				usage(argv[0]);
			}
			if (strcmp(argv[i], "good") == 0) {
				flagCount[VelocityFlag]++;
				flagCount[QuickTurnFlag]++;
				flagCount[OscOverthrusterFlag]++;
				flagCount[RapidFireFlag]++;
				flagCount[MachineGunFlag]++;
				flagCount[GuidedMissileFlag]++;
				flagCount[LaserFlag]++;
				flagCount[RicochetFlag]++;
				flagCount[SuperBulletFlag]++;
				flagCount[InvisibleBulletFlag]++;
				flagCount[StealthFlag]++;
				flagCount[TinyFlag]++;
				flagCount[NarrowFlag]++;
				flagCount[ShieldFlag]++;
				flagCount[SteamrollerFlag]++;
				flagCount[ShockWaveFlag]++;
				flagCount[PhantomZoneFlag]++;
				flagCount[GenocideFlag]++;
				flagCount[JumpingFlag]++;
				flagCount[IdentifyFlag]++;
				flagCount[CloakingFlag]++;
			}
			else {
				if ((f = lookupFlag(argv[i])) == int(NoFlag)) {
					std::cerr << "invalid flag `" << argv[i] << "'" << std::endl;
					usage(argv[0]);
				}
				flagCount[f]++;
			}
		}
		else if (strcmp(argv[i], "+r") == 0) {
			// all shots ricochet style
			gameStyle |= int(RicochetGameStyle);
		}
		else if (strcmp(argv[i], "+s") == 0) {
			// set required number of random flags
			if (i+1 < argc && isdigit(argv[i+1][0])) {
				++i;
				if ((numExtraFlags = atoi(argv[i])) == 0)
					numExtraFlags = 16;
			}
			else {
				numExtraFlags = 16;
			}
			allFlagsOut = true;
		}
		else if (strcmp(argv[i], "-a") == 0) {
			// momentum settings
			if (i + 2 >= argc) {
				std::cerr << "two arguments expected for " << argv[i] << std::endl;
				usage(argv[0]);
			}
			linearAcceleration = (float)atof(argv[++i]);
			angularAcceleration = (float)atof(argv[++i]);
			if (linearAcceleration < 0.0f)
				linearAcceleration = 0.0f;
			if (angularAcceleration < 0.0f)
				angularAcceleration = 0.0f;
			gameStyle |= int(InertiaGameStyle);
		}
		else if (strcmp(argv[i], "-b") == 0) {
			// random rotation to boxes in capture-the-flag game
			randomBoxes = true;
		}
		else if (strcmp(argv[i], "-c") == 0) {
			// capture the flag style
			gameStyle |= int(TeamFlagGameStyle);
		}
		else if (strncmp(argv[i], "-d", 2) == 0) {
			// increase debug level
			int count = 0;
			char *scan;
			for (scan = argv[i]+1; *scan == 'd'; scan++) count++;
			if (*scan != '\0') {
				std::cerr << "bad argument " << argv[i] << std::endl;
				usage(argv[0]);
			}
			debug += count;
		}
		else if (strcmp(argv[i], "-f") == 0) {
			// disallow given flag
			if (++i == argc) {
				std::cerr << "argument expected for -f" << std::endl;
				usage(argv[0]);
			}
			if (strcmp(argv[i], "bad") == 0) {
				flagDisallowed[ColorblindnessFlag] = true;
				flagDisallowed[ObesityFlag] = true;
				flagDisallowed[LeftTurnOnlyFlag] = true;
				flagDisallowed[RightTurnOnlyFlag] = true;
				flagDisallowed[MomentumFlag] = true;
				flagDisallowed[BlindnessFlag] = true;
				flagDisallowed[JammingFlag] = true;
				flagDisallowed[WideAngleFlag] = true;
			}
			else {
				if ((f = lookupFlag(argv[i])) == int(NoFlag)) {
					std::cerr << "invalid flag `" << argv[i] << "'" << std::endl;
					usage(argv[0]);
				}
				flagDisallowed[f] = true;
			}
		}
		else if (strcmp(argv[i], "-g") == 0) {
			oneGameOnly = true;
		}
		else if (strcmp(argv[i], "-h") == 0) {
			randomHeights = true;
		}
		else if (strcmp(argv[i], "-help") == 0) {
			extraUsage(argv[0]);
		}
		else if (strcmp(argv[i], "-i") == 0) {
			// use a different interface
			if (++i == argc) {
				std::cerr << "argument expected for -i" << std::endl;
				usage(argv[0]);
			}
			serverInterface = argv[i];
		}
		else if (strcmp(argv[i], "-j") == 0) {
			// allow jumping
			gameStyle |= int(JumpingGameStyle);
		}
		else if (strcmp(argv[i], "-mp") == 0) {
			// set maximum number of players
			if (++i == argc) {
				std::cerr << "argument expected for -mp" << std::endl;
				usage(argv[0]);
			}
			playerCountArg = i;
		}
		else if (strcmp(argv[i], "-ms") == 0) {
			// set maximum number of shots
			if (++i == argc) {
				std::cerr << "argument expected for -ms" << std::endl;
				usage(argv[0]);
			}
			int newMaxShots = atoi(argv[i]);
			if (newMaxShots < 1) {
				std::cerr << "using minimum number of shots of 1" << std::endl;
				maxShots = 1;
			}
			else if (newMaxShots > MaxShots) {
				std::cerr << "using maximum number of shots of " << MaxShots << std::endl;
				maxShots = uint16_t(MaxShots);
			}
			else maxShots = uint16_t(newMaxShots);
		}
		else if (strcmp(argv[i], "-mps") == 0) {
			// set maximum player score
			if (++i == argc) {
				std::cerr << "argument expected for -mps" << std::endl;
				usage(argv[0]);
			}
			maxPlayerScore = atoi(argv[i]);
			if (maxPlayerScore < 1) {
				std::cerr << "disabling player score limit" << std::endl;
				maxPlayerScore = 0;
			}
		}
		else if (strcmp(argv[i], "-mts") == 0) {
			// set maximum team score
			if (++i == argc) {
				std::cerr << "argument expected for -mts" << std::endl;
				usage(argv[0]);
			}
			maxTeamScore = atoi(argv[i]);
			if (maxTeamScore < 1) {
				std::cerr << "disabling team score limit" << std::endl;
				maxTeamScore = 0;
			}
		}
		else if (strcmp(argv[i], "-p") == 0) {
			// use a different port
			if (++i == argc) {
				std::cerr << "argument expected for -p" << std::endl;
				usage(argv[0]);
			}
			wksPort = atoi(argv[i]);
			if (wksPort < 1 || wksPort > 65535)
				wksPort = ServerPort;
			else
				useGivenPort = true;
		}
		else if (strcmp(argv[i], "-pf") == 0) {
			// try wksPort first and if we can't open that port then
			// let system assign a port for us.
			useFallbackPort = true;
		}
#ifdef PRINTSCORE
		else if (strcmp(argv[i], "-printscore") == 0) {
			// dump score whenever it changes
			printScore = true;
		}
#endif
		else if (strcmp(argv[i], "-public") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for -public" << std::endl;
				usage(argv[0]);
			}
			publicizeServer = true;
			publicizedTitle = argv[i];
			if (strlen(publicizedTitle) > 127) {
				argv[i][127] = '\0';
				std::cerr << "description too long... truncated" << std::endl;
			}
		}
		else if (strcmp(argv[i], "-publicaddr") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for -publicaddr" << std::endl;
				usage(argv[0]);
			}
			publicizedAddress = argv[i];
			publicizedAddressGiven = true;
		}
		else if (strcmp(argv[i], "-publiclist") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for -publiclist" << std::endl;
				usage(argv[0]);
			}
			listServerURL = argv[i];
		}
		else if (strcmp(argv[i], "-r") == 0) {
			// allow rogues
			gameStyle |= int(RoguesGameStyle);
		}
		else if (strcmp(argv[i], "-s") == 0) {
			// allow up to given number of random flags
			if (i+1 < argc && isdigit(argv[i+1][0])) {
				++i;
				if ((numExtraFlags = atoi(argv[i])) == 0)
					numExtraFlags = 16;
			}
			else {
				numExtraFlags = 16;
			}
			allFlagsOut = false;
		}
		else if (strcmp(argv[i], "-sa") == 0) {
			// insert antidote flags
			gameStyle |= int(AntidoteGameStyle);
		}
		else if (strcmp(argv[i], "-st") == 0) {
			// set shake timeout
			if (++i == argc) {
				std::cerr << "argument expected for -st" << std::endl;
				usage(argv[0]);
			}
			float timeout = (float)atof(argv[i]);
			if (timeout < 0.1f) {
				shakeTimeout = 1;
				std::cerr << "using minimum shake timeout of " << 0.1f * (float)shakeTimeout << std::endl;
			}
			else if (timeout > 300.0f) {
				shakeTimeout = 3000;
				std::cerr << "using maximum shake timeout of " << 0.1f * (float)shakeTimeout << std::endl;
			}
			else {
				shakeTimeout = uint16_t(timeout * 10.0f + 0.5f);
			}
			gameStyle |= int(ShakableGameStyle);
		}
		else if (strcmp(argv[i], "-sw") == 0) {
			// set shake win count
			if (++i == argc) {
				std::cerr << "argument expected for -sw" << std::endl;
				usage(argv[0]);
			}
			int count = atoi(argv[i]);
			if (count < 1) {
				shakeWins = 1;
				std::cerr << "using minimum shake win count of " << shakeWins << std::endl;
			}
			else if (count > 20) {
				shakeWins = 20;
				std::cerr << "using maximum shake win count of " << shakeWins << std::endl;
			}
			else {
				shakeWins = uint16_t(count);
			}
			gameStyle |= int(ShakableGameStyle);
		}
		else if (strcmp(argv[i], "-synctime") == 0) {
			// client clocks should be synchronized to server clock
			gameStyle |= int(TimeSyncGameStyle);
		}
		else if (strcmp(argv[i], "-t") == 0) {
			// allow teleporters
			useTeleporters = true;
		}
#ifdef TIMELIMIT
		else if (strcmp(argv[i], "-time") == 0) {
			// allow teleporters
			if (++i == argc) {
				std::cerr << "argument expected for -time" << std::endl;
				usage(argv[0]);
			}
			timeLimit = (float)atof(argv[i]);
			if (timeLimit <= 0.0f) {
				timeLimit = 300.0f;
				std::cerr << "using time limit of " << (int)timeLimit << " seconds" << std::endl;
			}
			timeElapsed = timeLimit;
		}
#endif
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0) {
			printVersion(std::cout);
			exit(0);
		}
		else if (strcmp(argv[i], "-passwd") == 0 || strcmp(argv[i], "-password") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for " << argv[i] << std::endl;
				usage(argv[0]);
			}
			password = argv[i];
		}
		else if (strcmp(argv[i], "-lagwarn") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for " << argv[i] << std::endl;
				usage(argv[0]);
			}
			lagwarnthresh = atoi(argv[i])/1000.0f;
		}
		else if (strcmp(argv[i], "-lagdrop") == 0) {
			if (++i == argc) {
				std::cerr << "argument expected for " << argv[i] << std::endl;
				usage(argv[0]);
			}
			maxlagwarn = atoi(argv[i]);
		}
		else {
			std::cerr << "bad argument " << argv[i] << std::endl;
			usage(argv[0]);
		}
	}

	// get player counts.  done after other arguments because we need
	// to ignore counts for rogues if rogues aren't allowed.
	if (playerCountArg > 0 && !parsePlayerCount(argv[playerCountArg]))
		usage(argv[0]);

	// make table of allowed extra flags
	if (numExtraFlags > 0) {
		// first disallow flags inconsistent with game style
		if (gameStyle & InertiaGameStyle)
			flagDisallowed[MomentumFlag] = true;
		if (gameStyle & JumpingGameStyle)
			flagDisallowed[JumpingFlag] = true;
		if (gameStyle & RicochetGameStyle)
			flagDisallowed[RicochetFlag] = true;
		if (!useTeleporters)
			flagDisallowed[PhantomZoneFlag] = true;

		// now count how many aren't disallowed
		for (int i = FirstSuperFlag; i <= LastSuperFlag; i++)
			if (!flagDisallowed[i])
				numAllowedFlags++;

		// if none allowed then no extra flags either
		if (numAllowedFlags == 0) {
			numExtraFlags = 0;
		}

		// otherwise make table of allowed flags
		else {
			allowedFlags = new FlagId[numAllowedFlags];
			int j = 0;
			for (int i = FirstSuperFlag; i <= LastSuperFlag; i++)
				if (!flagDisallowed[i])
					allowedFlags[j++] = FlagId(i);
		}
	}

	// allocate space for flags
	numFlags = numExtraFlags;
	// rogues don't get a flag
	if (gameStyle & TeamFlagGameStyle)
		numFlags += NumTeams - 1;
	for (int i = FirstFlag; i <= LastFlag; i++)
		numFlags += flagCount[i];
	flag = new FlagInfo[numFlags];

	// prep flags
	for (int i = 0; i < numFlags; i++) {
		flag[i].flag.id = NullFlag;
		flag[i].flag.status = FlagNoExist;
		flag[i].flag.type = FlagNormal;
		flag[i].flag.owner = 0;
		flag[i].flag.position[0] = 0.0f;
		flag[i].flag.position[1] = 0.0f;
		flag[i].flag.position[2] = 0.0f;
		flag[i].flag.launchPosition[0] = 0.0f;
		flag[i].flag.launchPosition[1] = 0.0f;
		flag[i].flag.launchPosition[2] = 0.0f;
		flag[i].flag.landingPosition[0] = 0.0f;
		flag[i].flag.landingPosition[1] = 0.0f;
		flag[i].flag.landingPosition[2] = 0.0f;
		flag[i].flag.flightTime = 0.0f;
		flag[i].flag.flightEnd = 0.0f;
		flag[i].flag.initialVelocity = 0.0f;
		flag[i].playerId = InvalidPlayer;
		flag[i].grabs = 0;
		flag[i].required = false;
	}
	f = 0;
	if (gameStyle & TeamFlagGameStyle) {
		flag[0].required = true;
		flag[0].flag.id = RedFlag;
		flag[0].flag.type = FlagNormal;
		flag[1].required = true;
		flag[1].flag.id = GreenFlag;
		flag[1].flag.type = FlagNormal;
		flag[2].required = true;
		flag[2].flag.id = BlueFlag;
		flag[2].flag.type = FlagNormal;
		flag[3].required = true;
		flag[3].flag.id = PurpleFlag;
		flag[3].flag.type = FlagNormal;
		f = 4;
	}
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "+f") == 0) {
			i++;
			if (strcmp(argv[i], "good") == 0) {
				if (setRequiredFlag(flag[f], VelocityFlag))
					f++;
				if (setRequiredFlag(flag[f], QuickTurnFlag))
					f++;
				if (setRequiredFlag(flag[f], OscOverthrusterFlag))
					f++;
				if (setRequiredFlag(flag[f], RapidFireFlag))
					f++;
				if (setRequiredFlag(flag[f], MachineGunFlag))
					f++;
				if (setRequiredFlag(flag[f], GuidedMissileFlag))
					f++;
				if (setRequiredFlag(flag[f], LaserFlag))
					f++;
				if (setRequiredFlag(flag[f], RicochetFlag))
					f++;
				if (setRequiredFlag(flag[f], SuperBulletFlag))
					f++;
				if (setRequiredFlag(flag[f], InvisibleBulletFlag))
					f++;
				if (setRequiredFlag(flag[f], StealthFlag))
					f++;
				if (setRequiredFlag(flag[f], TinyFlag))
					f++;
				if (setRequiredFlag(flag[f], NarrowFlag))
					f++;
				if (setRequiredFlag(flag[f], ShieldFlag))
					f++;
				if (setRequiredFlag(flag[f], SteamrollerFlag))
					f++;
				if (setRequiredFlag(flag[f], ShockWaveFlag))
					f++;
				if (setRequiredFlag(flag[f], PhantomZoneFlag))
					f++;
				if (setRequiredFlag(flag[f], GenocideFlag))
					f++;
				if (setRequiredFlag(flag[f], JumpingFlag))
					f++;
				if (setRequiredFlag(flag[f], IdentifyFlag))
					f++;
				if (setRequiredFlag(flag[f], CloakingFlag))
					f++;
			}
			else {
				flag[f].flag.id = FlagId(lookupFlag(argv[i]));
				flag[f].required = true;
				flag[f].flag.type = Flag::getType(flag[f].flag.id);
				f++;
			}
			gameStyle |= int(SuperFlagGameStyle);
		}
	}
	for (; f < numFlags; f++) {
		flag[f].required = allFlagsOut;
		gameStyle |= int(SuperFlagGameStyle);
	}

	// debugging
	if (debug >= 1) {
		// print style
		std::cerr << "style: " << std::ios::hex << gameStyle << std::ios::dec << std::endl;
		if (gameStyle & int(TeamFlagGameStyle))
			std::cerr << "  capture the flag" << std::endl;
		if (gameStyle & int(SuperFlagGameStyle))
			std::cerr << "  super flags allowed" << std::endl;
		if (gameStyle & int(RoguesGameStyle))
			std::cerr << "  rogues allowed" << std::endl;
		if (gameStyle & int(JumpingGameStyle))
			std::cerr << "  jumping allowed" << std::endl;
		if (gameStyle & int(InertiaGameStyle))
			std::cerr << "  inertia: " << linearAcceleration << "," <<
					angularAcceleration << std::endl;
		if (gameStyle & int(RicochetGameStyle))
			std::cerr << "  all shots ricochet" << std::endl;
		if (gameStyle & int(ShakableGameStyle))
			std::cerr << "  shakable bad flags: timeout=" <<
					0.1f * float(shakeTimeout) <<
					", wins=" << shakeWins << std::endl;
		if (gameStyle & int(AntidoteGameStyle))
			std::cerr << "  antidote flags" << std::endl;
	}
}

int main(int argc, char **argv)
{
	int nfound;

	// check time bomb
	if (timeBombBoom()) {
		std::cerr << "This release expired on " << timeBombString() << "." << std::endl;
		std::cerr << "Please upgrade to the latest release." << std::endl;
		exit(0);
	}

	// print expiration date
	if (timeBombString()) {
		char bombMessage[80];
		std::cerr << "This release will expire on " << timeBombString() << "." << std::endl;
		sprintf(bombMessage, "Version %d.%d%c%d",
		(VERSION / 10000000) % 100, (VERSION / 100000) % 100,
		(char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000);
		std::cerr << bombMessage << std::endl;
	}

	// trap some signals.  let user kill server.
	if (PLATFORM->signalCatch(kSigINT, kSigIGN) != kSigIGN)
		PLATFORM->signalCatch(kSigINT, terminateServer);
	PLATFORM->signalCatch(kSigTERM, terminateServer);
	PLATFORM->signalCatch(kSigPIPE, kSigIGN);

	// initialize
#if defined(_WIN32)
	{
		static const int major = 2, minor = 2;
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
			DEBUG1("Failed to initialize winsock.  Terminating.\n");
			return 1;
		}
		if (LOBYTE(wsaData.wVersion) != major ||
				HIBYTE(wsaData.wVersion) != minor) {
			DEBUG1("Version mismatch in winsock;"
					"  got %d.%d.  Terminating.\n",
					(int)LOBYTE(wsaData.wVersion),
					(int)HIBYTE(wsaData.wVersion));
			WSACleanup();
			return 1;
		}
	}
#endif /* defined(_WIN32) */
	bzfsrand(time(0));

	// parse arguments
	parse(argc, argv);

	if (serverInterface)
		serverAddress = Address::getHostAddress(serverInterface);


	// my address to publish.  allow arguments to override (useful for
	// firewalls).  use my official hostname if it appears to be
	// canonicalized, otherwise use my IP in dot notation.
	// set publicized address if not set by arguments
	if (publicizedAddress.empty()) {
		publicizedAddress = Address::getHostName();
		if (strchr(publicizedAddress.c_str(), '.') == NULL)
			publicizedAddress = serverAddress.getDotNotation();
		if (wksPort != ServerPort) {
			char portString[20];
			sprintf(portString, ":%d", wksPort);
			publicizedAddress += portString;
		}
	}

	// prep ping reply
	if (serverAddress.isAny())
		pingReply.serverId.serverHost = Address::getHostAddress();
	else
		pingReply.serverId.serverHost = serverAddress;
	pingReply.serverId.port = htons(wksPort);
	pingReply.serverId.number = 0;
	pingReply.gameStyle = gameStyle;
	pingReply.maxPlayers = maxPlayers;
	pingReply.maxShots = maxShots;
	pingReply.rogueMax = maxTeam[0];
	pingReply.redMax = maxTeam[1];
	pingReply.greenMax = maxTeam[2];
	pingReply.blueMax = maxTeam[3];
	pingReply.purpleMax = maxTeam[4];
	pingReply.shakeWins = shakeWins;
	pingReply.shakeTimeout = shakeTimeout;
	pingReply.maxTime = (int)timeLimit;
	pingReply.maxPlayerScore = maxPlayerScore;
	pingReply.maxTeamScore = maxTeamScore;

	// start listening and prepare world database
	if (!defineWorld() || !serverStart()) {
#if defined(_WIN32)
		WSACleanup();
#endif /* defined(_WIN32) */
		return 1;
	}
	if (debug >= 2) {
		// print networking info
		std::cerr << "listening on " <<
		serverAddress.getDotNotation() << ":" << wksPort << std::endl;
	}

	TimeKeeper lastSuperFlagInsertion = TimeKeeper::getCurrent();
	const float flagExp = -logf(0.5f) / FlagHalfLife;

	int i;
	while (!done) {
		// prepare select set
		fd_set read_set, write_set;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		for (i = 0; i < maxPlayers; i++) {
			if (player[i].fd != NotConnected) {
				//DEBUG1("fdset fd,read %i %lx\n",player[i].fd,read_set);
				FD_SET(player[i].fd, &read_set);

				if (player[i].outmsgSize > 0)
					FD_SET(player[i].fd, &write_set);
			}
		}
		// always listen for connections
		FD_SET(wksSocket, &read_set);

		// always listen for UDP traffic
		FD_SET(udpSocket, &read_set);

		// check for list server socket connected
		for (i = 0; i < listServerLinksCount; i++)
			if (listServerLinks[i].socket != NotConnected)
				FD_SET(listServerLinks[i].socket, &write_set);

		// find timeout when next flag would hit ground
		TimeKeeper tm = TimeKeeper::getCurrent();
		// lets start by waiting 0.25 sec
		float waitTime = 3.0f;
#ifdef TIMELIMIT
		if (timeLimit > 0.0f)
			waitTime = 1.0f;
#endif
		if (numFlagsInAir > 0) {
			for (i = 0; i < numFlags; i++)
				if (flag[i].flag.status != FlagNoExist &&
						flag[i].flag.status != FlagOnTank &&
						flag[i].flag.status != FlagOnGround &&
						flag[i].dropDone - tm < waitTime)
					waitTime = flag[i].dropDone - tm;
		}

		// minmal waitTime
		if (waitTime < 0.0f)
			waitTime = 0.0f;

		// we have no pending packets
		nfound = 0;

		// wait for communication or for a flag to hit the ground
		struct timeval timeout;
		timeout.tv_sec = long(floorf(waitTime));
		timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
		nfound = select(maxFileDescriptor+1, (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);
		//if (nfound)
		//  DEBUG1("nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);

		tm = TimeKeeper::getCurrent();

#ifdef TIMELIMIT
		// see if game time ran out
		if (!gameOver && timeLimit > 0.0f) {
			float newTimeElapsed = tm - gameStartTime;
			float timeLeft = timeLimit - newTimeElapsed;
			if (timeLeft <= 0.0f) {
				timeLeft = 0.0f;
				gameOver = true;
			}
			if (timeLeft == 0.0f || newTimeElapsed - timeElapsed >= 30.0f) {
				void *buf, *bufStart = getDirectMessageBuffer();
				buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
				broadcastMessage(MsgTimeUpdate, (char*)buf-(char*)bufStart, bufStart);
				timeElapsed = newTimeElapsed;
			}
		}
#endif

		// if any flags were in the air, see if they've landed
		if (numFlagsInAir > 0) {
			for (i = 0; i < numFlags; i++) {
				if (flag[i].flag.status == FlagInAir ||
						flag[i].flag.status == FlagComing) {
					if (flag[i].dropDone - tm <= 0.0f) {
						flag[i].flag.status = FlagOnGround;
						numFlagsInAir--;
						sendFlagUpdate(i);
					}
				}
				else if (flag[i].flag.status == FlagGoing) {
					if (flag[i].dropDone - tm <= 0.0f) {
						flag[i].flag.status = FlagNoExist;
						numFlagsInAir--;
						resetFlag(i);
					}
				}
			}
		}

		// maybe add a super flag (only if game isn't over)
		if (!gameOver && numExtraFlags > 0) {
			float t = expf(-flagExp * (tm - lastSuperFlagInsertion));
			if ((float)bzfrand() > t) {
				// find an empty slot for an extra flag
				assert(numFlags >= numExtraFlags);
				for (i = numFlags - numExtraFlags; i < numFlags; i++)
					if (flag[i].flag.id == NullFlag)
						break;
				if (i != numFlags) {
					if (randomFlag(i))
						sendFlagUpdate(i);
				}
				lastSuperFlagInsertion = tm;
			}
		}

		// occasionally add ourselves to the list again (in case we were
		// dropped for some reason).
		if (publicizeServer) {
			if (tm - listServerLastAddTime > ListServerReAddTime) {
				// if there are no list servers and nobody is playing then
				// try publicizing again because we probably failed to get
				// the list last time we published, and if we don't do it
				// here then unless somebody stumbles onto this server then
				// quits we'll never try publicizing ourself again.
				if (listServerLinksCount == 0) {
					// count the number of players
					int i;
					for (i = 0; i < maxPlayers; i++)
						if (player[i].state > PlayerInLimbo)
							break;

					// if nobody playing then publicize
					if (i == maxPlayers)
						publicize();
				}

				// send add request
				sendMessageToListServer("ADD");
				listServerLastAddTime = tm;
			}
		}

		// check messages
		if (nfound > 0) {
			//DEBUG1("chkmsg nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);
			// first check initial contacts
			if (FD_ISSET(wksSocket, &read_set))
				acceptClient();

			// check for connection to list server
			for (i = 0; i < listServerLinksCount; ++i)
				if (listServerLinks[i].socket != NotConnected &&
					FD_ISSET(listServerLinks[i].socket, &write_set))
					sendMessageToListServerForReal(i);

			// check if we have any UDP packets pending
			if (FD_ISSET(udpSocket, &read_set)) {
				int numpackets;
				while (uread(&i, &numpackets) > 0) {
					// read head
					uint16_t len, code;
					void *buf = player[i].udpmsg;
					buf = nboUnpackUShort(buf, len);
					buf = nboUnpackUShort(buf, code);

					// clear out message
					player[i].udplen = 0;

					// handle the command for UDP
					handleCommand(i, code, len, player[i].udpmsg);
				}
			}

			// now check messages from connected players and send queued messages
			for (i = 0; i < maxPlayers; i++) {
				if (player[i].fd != NotConnected && FD_ISSET(player[i].fd, &write_set)) {
					pflush(i);
				}

				if (player[i].state >= PlayerInLimbo && FD_ISSET(player[i].fd, &read_set)) {
					// read header if we don't have it yet
					if (player[i].tcplen < 4) {
						pread(i, 4 - player[i].tcplen);

						// if header not ready yet then skip the read of the body
						if (player[i].tcplen < 4)
							continue;
					}

					// read body if we don't have it yet
					uint16_t len, code;
					void *buf = player[i].tcpmsg;
					buf = nboUnpackUShort(buf, len);
					buf = nboUnpackUShort(buf, code);
					if (player[i].tcplen < 4 + (int)len) {
						pread(i, 4 + (int)len - player[i].tcplen);

						// if body not ready yet then skip the command handling
						if (player[i].tcplen < 4 + (int)len)
							continue;
					}

					// clear out message
					player[i].tcplen = 0;

					// handle the command
					handleCommand(i, code, len, player[i].tcpmsg);
				}
			}
		}
		else if (nfound < 0) {
			if (getErrno() != EINTR) {
				// test code - do not uncomment, will cause big stuttering
				// sleep(1);
			}
		}
	}

	serverStop();

	// free misc stuff
	delete[] flag;  flag = NULL;
	delete[] allowedFlags;  allowedFlags = NULL;
	delete world;
	delete[] worldDatabase;
#if defined(_WIN32)
	WSACleanup();
#endif /* defined(_WIN32) */

	// done
	return exitCode;
}
// vim: ts=4
