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

#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <iostream>
#include "ServerLink.h"
#include "Pack.h"
#include "LocalPlayer.h"
#include "ErrorHandler.h"
#include "network.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "PlatformFactory.h"

#define UDEBUG if (UDEBUGMSG) printf
#define UDEBUGMSG false

#if defined(DEBUG)
#define NETWORK_STATS
#endif

#if defined(NETWORK_STATS)
#include "bzfio.h"
#include "TimeKeeper.h"
static TimeKeeper		startTime;
static uint32_t			bytesSent;
static uint32_t			bytesReceived;
static uint32_t			packetsSent;
static uint32_t			packetsReceived;
#endif

#if !defined(_WIN32)

static void				timeout(Signal)
{
	PLATFORM->signalCatch(kSigALRM, kSigIGN);
	alarm(0);
}

#else // Connection timeout for Windows

DWORD ThreadID; 			// Thread ID
HANDLE hConnected;				// "Connected" event
HANDLE hThread; 			// Connection thread

typedef struct {
	public:
		int						query;
		CNCTType*				addr;
		int						saddr;
} TConnect;

TConnect conn;

DWORD WINAPI ThreadConnect(LPVOID params)
{
	TConnect *conn = (TConnect*)params;
	if(connect(conn->query, conn->addr, conn->saddr) >= 0) {
		SetEvent(hConnected); // Connect successful
	}
	ExitThread(0);
	return 0;
}

#endif // !defined(_WIN32)

// FIXME -- packet recording
#include <stdio.h>
#include <stdlib.h>
#include "TimeKeeper.h"
FILE* packetStream = NULL;
TimeKeeper packetStartTime;
static const unsigned long serverPacket = 1;
static const unsigned long endPacket = 0;

ServerLink*				ServerLink::server = NULL;

ServerLink::ServerLink(const Address& serverAddress, int port) :
								state(SocketError),		// assume failure
								tcpfd(-1),				// assume failure
								udpfd(-1)				// assume failure
{
	int i;
	char cServerVersion[128];

	struct protoent* p;
#if defined(_WIN32)
	BOOL off = FALSE;
#else
	int off = 0;
#endif

	// queue is empty
	uqueue = NULL;
	dqueue = NULL;

	lastSendPacketNo = lastRecvPacketNo = currentRecvSeq = 0;

	// initialize version to a bogus number
	strcpy(version, "BZFS0000");

	// open connection to server.  first connect to given port.
	// don't wait too long.
	int query = socket(AF_INET, SOCK_STREAM, 0);
	if (query < 0) return;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = serverAddress;

	UDEBUG("Remote %s\n", inet_ntoa(addr.sin_addr));

#if !defined(_WIN32)
	PLATFORM->signalCatch(kSigALRM, timeout);
	alarm(5);
	const bool okay = (connect(query, (CNCTType*)&addr, sizeof(addr)) >= 0);
	alarm(0);
	PLATFORM->signalCatch(kSigALRM, kSigIGN);
#else // Connection timeout for Windows

	// Initialize structure
	conn.query = query;
	conn.addr = (CNCTType*)&addr;
	conn.saddr = sizeof(addr);

	// Create event
	hConnected = CreateEvent(NULL, FALSE, FALSE, "Connected Event");

	hThread=CreateThread(NULL, 0, ThreadConnect, &conn, 0, &ThreadID);
	const bool okay = (WaitForSingleObject(hConnected, 5000) == WAIT_OBJECT_0);
	if (!okay)
		TerminateThread(hThread ,1);

	// Do some cleanup
	CloseHandle(hConnected);
	CloseHandle(hThread);

#endif // !defined(_WIN32)
	if (!okay)
		return;

	// get server version and verify (last digit in version is ignored)
	i = recv(query, (char*)version, 8, 0);
	if (i < 8)
		return;

	sprintf(cServerVersion,"Server version: '%8s'",version);
	printError(cServerVersion);

	if (strncmp(version, ServerVersion, 7) != 0) {
		state = BadVersion;
		return;
	}

	// read local player's id
	i = recv(query, (char*)&id, sizeof(id), 0);
	if (i < (int)sizeof(id))
		return;
	if (id == 0xff) {
		state = Rejected;
		return;
	}

	tcpfd = query;

	// turn on TCP no delay
	p = getprotobyname("tcp");
	if (p)
		setsockopt(tcpfd, p->p_proto, TCP_NODELAY, (SSOType)&off, sizeof(off));  // changed

	state = Okay;
#if defined(NETWORK_STATS)
	bytesSent = 0;
	bytesReceived = 0;
	packetsSent = 0;
	packetsReceived = 0;
#endif

	// FIXME -- packet recording
	if (getenv("BZFLAGSAVE")) {
		packetStream = fopen(getenv("BZFLAGSAVE"), "w");
		packetStartTime = TimeKeeper::getCurrent();
	}
}

ServerLink::~ServerLink()
{
	if (state != Okay) return;
	shutdown(tcpfd, 2);
	closesocket(tcpfd);
	tcpfd = -1;

	if (udpfd >= 0) {
		closesocket(udpfd);
		udpfd = -1;
	}

	// FIXME -- packet recording
	if (packetStream) {
		long dt = (long)((TimeKeeper::getCurrent() - packetStartTime) * 10000.0f);
		fwrite(&endPacket, sizeof(endPacket), 1, packetStream);
		fwrite(&dt, sizeof(dt), 1, packetStream);
		fclose(packetStream);
	}

#if defined(NETWORK_STATS)
	const float dt = TimeKeeper::getCurrent() - startTime;
	std::cerr << "Server network statistics:" << std::endl;
	std::cerr << "  elapsed time    : " << dt << std::endl;
	std::cerr << "  bytes sent      : " << bytesSent << " (" <<
				(float)bytesSent / dt << "/sec)" << std::endl;
	std::cerr << "  packets sent    : " << packetsSent << " (" <<
				(float)packetsSent / dt << "/sec)" << std::endl;
	if (packetsSent != 0)
		std::cerr << "  bytes/packet    : " <<
				(float)bytesSent / (float)packetsSent << std::endl;
	std::cerr << "  bytes received  : " << bytesReceived << " (" <<
				(float)bytesReceived / dt << "/sec)" << std::endl;
	std::cerr << "  packets received: " << packetsReceived << " (" <<
				(float)packetsReceived / dt << "/sec)" << std::endl;
	if (packetsReceived != 0)
		std::cerr << "  bytes/packet    : " <<
				(float)bytesReceived / (float)packetsReceived << std::endl;
#endif
}

ServerLink*				ServerLink::getServer() // const
{
	return server;
}

void					ServerLink::setServer(ServerLink* _server)
{
	server = _server;
}

void					ServerLink::send(uint16_t code, uint16_t len,
														const void* msg)
{
	bool needForSpeed = false;
	if (state != Okay)
		return;
	char msgbuf[MaxPacketLen];
	void* buf = msgbuf;
	buf = nboPackUShort(buf, len);
	buf = nboPackUShort(buf, code);
	if (msg && len != 0) buf = nboPackString(buf, msg, len);

	switch (code) {
		case MsgShotBegin:
		case MsgShotEnd:
		case MsgPlayerUpdate:
		case MsgGMUpdate:
		case MsgUDPLinkEstablished:
			needForSpeed = true;
			break;
	}

	if (needForSpeed && (udpfd >= 0)) {
		uint32_t length;
		int n;

		// UDP send
		void *tobesend;
		UDEBUG("ENQUEUE %d [%d]\n",len+4, lastSendPacketNo);
		enqueuePacket(SEND, lastSendPacketNo, (void *)msgbuf,len+4);
		lastSendPacketNo++;
		UDEBUG("ASSEMBLE\n");
		tobesend = assembleSendPacket(&length);
		if (length == 0) {
			printError("Attention: Server has not answered for a long time");
			printError("           Connection will be dropped.");
			// Server has not answered to clear packet for send buffer, we have
			// reached the UDP limit (8192) and so we assume the connection is
			// broken
			state = Hungup;
		}

		// length = compressPacket(tobesend, length);

		//printError("UDP SEND %d %d",lastSendPacketNo-1, length);
#ifdef TESTLINK
		if ((random()%TESTQUALTIY) != 0)
#endif
		n = sendto(udpfd, (const char *)tobesend,
						length, 0, &servaddr, sizeof(servaddr));
		// we don't care about errors yet
		if (tobesend)
			free((unsigned char *)tobesend);
		return;
	}

	//printError("TCP Send: %02x",code);

	int r = ::send(tcpfd, (const char*)msgbuf, len + 4, 0);
	(void)r; // silence g++
#if defined(_WIN32)
	if (r == SOCKET_ERROR) {
		const int e = WSAGetLastError();
		if (e == WSAENETRESET || e == WSAECONNABORTED ||
		e == WSAECONNRESET || e == WSAETIMEDOUT)
			state = Hungup;
		r = 0;
	}
#endif

#if defined(NETWORK_STATS)
	bytesSent += r;
	packetsSent++;
#endif
}

void*					ServerLink::getPacketFromServer(uint16_t* length, uint16_t* /* seqno */)
{
	struct PacketQueue *moving=dqueue;
	if (moving != NULL) {
		void *remember=moving->data;
		*length = moving->length;
		free(moving);
		dqueue=NULL;
		return remember;
	}
	UDEBUG("*** getpacket NULL\n");
	*length = 0;
	return NULL;
}

void					ServerLink::enqueuePacket(int op, int rseqno, void *msg, int n)
{
	struct PacketQueue *moving;
	struct PacketQueue *newpacket = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));

	if (!newpacket) {
		printError("Fatal: no memory for packetBuffer");
		return;
	}

	if (op == SEND) {
		moving=uqueue;
	} else {
		moving=dqueue;
		// did the last packet wrap around or is OOB?
		if ((lastRecvPacketNo > rseqno) && ((lastRecvPacketNo-rseqno)<32768)) {
		    return;
		}
		lastRecvPacketNo = rseqno;
	}

	if (moving != NULL) {
		if (moving->data) free(moving->data);
		free(moving);
	}
	newpacket->seqno=rseqno;
	newpacket->data=(unsigned char *)malloc(n * sizeof(unsigned char));
	memcpy((unsigned char *)newpacket->data, (unsigned char *)msg, n);
	newpacket->length=n;
	newpacket->next=NULL;
	if (op == SEND)
		uqueue=newpacket;
	else
		dqueue=newpacket;
}

void					ServerLink::disqueuePacket(int op, int /*rseqno*/)
{
	struct PacketQueue *moving;

	if (op == SEND) {
		moving=uqueue;
		uqueue=NULL;
	}
	else {
		moving=dqueue;
		dqueue=NULL;
	}
	if (moving != NULL) {
		if (moving->data)
			free(moving->data);
		free(moving);
	}
}

void* 					ServerLink::assembleSendPacket(uint32_t* length)
{
	struct PacketQueue *moving = uqueue;
	unsigned char *assemblybuffer;
	int in, n;
	unsigned char *buf;

	if (!moving) {
		*length = 0;
		return NULL;
	}

	in = n = 8192;

	assemblybuffer= (unsigned char *)malloc(n);

	buf = assemblybuffer;
	buf = (unsigned char *)nboPackUShort(buf, 0xfeed);
	buf = (unsigned char *)nboPackUShort(buf, lastRecvPacketNo);
	buf = (unsigned char *)nboPackUShort(buf, moving->length);
	buf = (unsigned char *)nboPackUShort(buf, moving->seqno);
	n-=8;
	n-= moving->length;
	if (n>2) {
		memcpy((unsigned char *)buf, (unsigned char *)moving->data, moving->length);
		buf += moving->length;
	}
	else {
		printError("ASSEMBLE SEND PACKET OVERRUN BUFFER");
	}
	if (n<=2) {
		printError("ASSEMBLE SEND PACKET OVERRUN BUFFER");
		assemblybuffer[0]=0x0;  // invalidate
		*length=0;
		return assemblybuffer;
	}

	buf = (unsigned char *)nboPackUShort(buf, 0xffff);
	n-=2;
	*length = (in - n);

	disqueuePacket(SEND,0);

	return assemblybuffer;
}

void 					ServerLink::disassemblePacket(void *msg, int *nopacket)
{
	unsigned short marker;
	unsigned short usdata;
	unsigned char *buf = (unsigned char *)msg;
	int numpacket=0;

	UDEBUG("*** DisassemblePacket\n");

	buf = (unsigned char *)nboUnpackUShort(buf, marker);
	if (marker!=0xfeed) {
		UDEBUG("!!! Reject Packet because invalid header %04x\n", marker);
		return;
	}
	buf = (unsigned char *)nboUnpackUShort(buf, usdata);

	UDEBUG("Server has seen last Seqno: %d\n",usdata);

	while (true) {
		unsigned short seqno;
		unsigned short length;
		buf = (unsigned char *)nboUnpackUShort(buf, length);
		if (length == 0xffff) break;
		buf = (unsigned char *)nboUnpackUShort(buf, seqno);
		enqueuePacket(RECEIVE, seqno, buf, length);
		buf+=length;
		numpacket++;
	}
	*nopacket=numpacket;
}

int						ServerLink::read(uint16_t& code, uint16_t& len,
												void* msg, int blockTime)
{

	code = MsgNull;
	len = 0;

	if (state != Okay) return -1;

	if (udpfd >= 0) {
		int n, num_packets;
		uint16_t lseqno;

		AddrLen recvlen = sizeof(servaddr);
		unsigned char ubuf[8192];
		n = recvfrom(udpfd, (char *)ubuf, 8192, 0, &servaddr, &recvlen);
		if (n>0) {
			disassemblePacket(ubuf, &num_packets);
		}
		void *pmsg =  getPacketFromServer(&len, &lseqno);
		if (pmsg != NULL) {
			// unpack header and get message
			uint16_t sub_length;
			void* buf = pmsg;
			buf = nboUnpackUShort(buf, sub_length);
			buf = nboUnpackUShort(buf, code);
			UDEBUG("<** UDP Packet Code %x Len %x / %x\n",code, sub_length, len);
			len = len - 4;
			memcpy((char *)msg,(char *)buf,len);
			free(pmsg);
			return 1;
		}

		if (UDEBUGMSG) printError("Fallback to normal TCP receive");
		len = 0;
		code = MsgNull;

		blockTime = 0;

	}

	// block for specified period.  default is no blocking (polling)
	struct timeval timeout;
	timeout.tv_sec = blockTime / 1000;
	timeout.tv_usec = blockTime - 1000 * timeout.tv_sec;

	// only check server
	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(tcpfd, &read_set);
	int nfound = select(tcpfd+1, (fd_set*)&read_set, NULL, NULL,
						(struct timeval*)(blockTime >= 0 ? &timeout : NULL));
	if (nfound == 0) return 0;
	if (nfound < 0) return -1;

	// printError("<** TCP Packet Code Recevied %d", time(0));
	// FIXME -- don't really want to take the chance of waiting forever
	// on the remaining select() calls, but if the server and network
	// haven't been hosed then the data will get here soon.  And if the
	// server or network is down then we don't really care anyway.

	// get packet header -- keep trying until we get 4 bytes or an error
	char headerBuffer[4];


	int rlen = 0;
	rlen = recv(tcpfd, (char*)headerBuffer, 4, 0);

	int tlen = rlen;
	while (rlen >= 1 && tlen < 4) {
		FD_ZERO(&read_set);
		FD_SET(tcpfd, &read_set);
		nfound = select(tcpfd+1, (fd_set*)&read_set, NULL, NULL, NULL);
		if (nfound == 0) continue;
		if (nfound < 0) return -1;
		rlen = recv(tcpfd, (char*)headerBuffer + tlen, 4 - tlen, 0);
		if (rlen >= 0) tlen += rlen;
	}
	if (tlen < 4) return -1;
#if defined(NETWORK_STATS)
	bytesReceived += 4;
	packetsReceived++;
#endif

	// unpack header and get message
	void* buf = headerBuffer;
	buf = nboUnpackUShort(buf, len);
	buf = nboUnpackUShort(buf, code);

	//printError("Code is %02x",code);
	if (len > 0)
		rlen = recv(tcpfd, (char*)msg, int(len), 0);
	else
		rlen = 0;
#if defined(NETWORK_STATS)
	if (rlen >= 0) bytesReceived += rlen;
#endif
	if (rlen == int(len)) goto success;	// got whole thing

	// keep reading until we get the whole message
	tlen = rlen;
	while (rlen >= 1 && tlen < int(len)) {
		FD_ZERO(&read_set);
		FD_SET(tcpfd, &read_set);
		nfound = select(tcpfd+1, (fd_set*)&read_set, 0, 0, NULL);
		if (nfound == 0) continue;
		if (nfound < 0) return -1;
		rlen = recv(tcpfd, (char*)msg + tlen, int(len) - tlen, 0);
		if (rlen >= 0) tlen += rlen;
#if defined(NETWORK_STATS)
		if (rlen >= 0) bytesReceived += rlen;
#endif
	}
	if (tlen < int(len)) return -1;

success:
// FIXME -- packet recording
if (packetStream) {
	long dt = (long)((TimeKeeper::getCurrent() - packetStartTime) * 10000.0f);
	fwrite(&serverPacket, sizeof(serverPacket), 1, packetStream);
	fwrite(&dt, sizeof(dt), 1, packetStream);
	fwrite(headerBuffer, 4, 1, packetStream);
	fwrite(msg, len, 1, packetStream);
}
	return 1;
}

void					ServerLink::enableUDPCon()
{
	struct sockaddr_in my_addr;
	AddrLen addr_len = sizeof(my_addr);

	if (getsockname(tcpfd, (struct sockaddr *)&my_addr, &addr_len) < 0) {
		printError("enableUDPCon: Unable to get my address");
		return;
	}

	addr_len = sizeof(servaddr);
	if (getpeername(tcpfd, (struct sockaddr *)&servaddr, &addr_len) < 0) {
		printError("enableUDPCon: Unable to get server address");
		return;
	}

	struct sockaddr_in *servaddr_in = (struct sockaddr_in *)&servaddr;
	printError("UDP connection attempting on: (%d) [%s:%d] -> [%s:%d]",
			udpfd, inet_ntoa(my_addr.sin_addr), ntohs(my_addr.sin_port),
			inet_ntoa(servaddr_in->sin_addr), ntohs(servaddr_in->sin_port));

	if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return;
	}
	// use same ports/addresses as tcp
  	if (bind(udpfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) != 0) {
		printError("Error: Unable to bind UDP socket");
		closesocket(udpfd);
		udpfd = -1;
		return;
	}

	if (BzfNetwork::setNonBlocking(udpfd) < 0) {
		printError("Error: Unable to set NonBlocking for UDP receive socket");
		closesocket(udpfd);
		udpfd = -1;
		return;
	}

	char msg[1];
	void* buf = msg;
	buf = nboPackUByte(buf, id);
	// this goes via udp to confirm our addr on the server end
	send(MsgUDPLinkEstablished, sizeof(msg), msg);
	return;
}

void					ServerLink::sendEnter(PlayerType type,
												TeamColor team,
												const char* name,
												const char* email)
{
	if (state != Okay) return;
	char msg[PlayerIdPLen + 4 + CallSignLen + EmailLen];
	::memset(msg, 0, sizeof(msg));
	void* buf = msg;
	buf = nboPackUShort(buf, uint16_t(type));
	buf = nboPackUShort(buf, uint16_t(team));
	::memcpy(buf, name, ::strlen(name));
	buf = (void*)((char*)buf + CallSignLen);
	::memcpy(buf, email, ::strlen(email));
	buf = (void*)((char*)buf + EmailLen);
	send(MsgEnter, sizeof(msg), msg);
}

void					ServerLink::sendPlayerUpdate(const Player* player)
{
	if (state == SocketError) return;
	char msg[PlayerUpdatePLen];
	void* buf = msg;
	buf = nboPackUByte(buf, player->getId());
	buf = player->pack(buf);
	send(MsgPlayerUpdate, sizeof(msg), msg);
}

void					ServerLink::sendCaptureFlag(TeamColor team)
{
	char msg[2];
	nboPackUShort(msg, uint16_t(team));
	send(MsgCaptureFlag, sizeof(msg), msg);
}

void					ServerLink::sendGrabFlag(int flagIndex)
{
	char msg[2];
	nboPackUShort(msg, uint16_t(flagIndex));
	send(MsgGrabFlag, sizeof(msg), msg);
}

void					ServerLink::sendDropFlag(FlagDropReason reason, const float* position)
{
	char msg[13];
	void* buf = msg;
	buf = nboPackUByte(buf, reason);
	buf = nboPackVector(buf, position);
	send(MsgDropFlag, sizeof(msg), msg);
}

void					ServerLink::sendKilled(const PlayerId& killer,
																int shotId)
{
	char msg[PlayerIdPLen + 2];
	void* buf = msg;
	buf = nboPackUByte(buf, killer);
	buf = nboPackShort(buf, int16_t(shotId));
	send(MsgKilled, sizeof(msg), msg);
}

void					ServerLink::sendBeginShot(const FiringInfo& info)
{
	char msg[FiringInfoPLen];
	void* buf = msg;
	buf = info.pack(buf);
	send(MsgShotBegin, sizeof(msg), msg);
}

void					ServerLink::sendEndShot(const PlayerId& source,
														int shotId, int reason)
{
	char msg[PlayerIdPLen + 4];
	void* buf = msg;
	buf = nboPackUByte(buf, source);
	buf = nboPackShort(buf, int16_t(shotId));
	buf = nboPackUShort(buf, uint16_t(reason));
	send(MsgShotEnd, sizeof(msg), msg);
}

void					ServerLink::sendAlive(const float* pos,
												const float* forward)
{
	char msg[24];
	void* buf = msg;
	buf = nboPackVector(buf, pos);
	buf = nboPackVector(buf, forward);
	send(MsgAlive, sizeof(msg), msg);
}

void					ServerLink::sendTeleport(int from, int to)
{
	char msg[4];
	void* buf = msg;
	buf = nboPackUShort(buf, uint16_t(from));
	buf = nboPackUShort(buf, uint16_t(to));
	send(MsgTeleport, sizeof(msg), msg);
}

void					ServerLink::sendNewScore(int wins, int losses)
{
	char msg[4];
	void* buf = msg;
	buf = nboPackUShort(buf, uint16_t(wins));
	buf = nboPackUShort(buf, uint16_t(losses));
	send(MsgScore, sizeof(msg), msg);
}
