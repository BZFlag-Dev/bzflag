/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __NETHANDLER_H__
#define __NETHANDLER_H__

/* common header */
#include "common.h"

/* system headers */
#include <string>
#include <map>

/* common interface headers */
#include "PlayerInfo.h"
#include "Address.h"
#include "AdnsHandler.h"

enum RxStatus {
  ReadAll,
  ReadPart,
  ReadHuge,
  ReadReset,
  ReadError,
  ReadDiscon
};

const int maxHandlers = LastRealPlayer;

#ifdef DEBUG
#define NETWORK_STATS
#endif
#ifdef NETWORK_STATS
struct MessageCount {
  uint32_t count;
  uint16_t maxSize;
};
#endif

/** This class is a client that connects to a BZFlag client and has
    functions for sending and receiving messages.
*/
class NetHandler {
public:
  /** A default constructor.
      It needs a pointer to the Player basic Info,
      a socket address to address subsequent message at user,
      a player Index, a unique pointer to a player
      the file descriptor for the TCP connection with the user.
  */
  NetHandler(PlayerInfo *_info, const struct sockaddr_in &_clientAddr,
	     int _playerIndex, int _fd);
  /** The default destructor
      free all internal resources, and close the tcp connection
  */
  ~NetHandler();

  /** Class-Wide initialization and destruction
      Should be called before any other operation on any clas instance
      InitHandlers needs the addr structure filled to point to the local port
      needed for udp communications
  */
  static bool initHandlers(struct sockaddr_in addr);
  static void destroyHandlers();

  /// General function to support the select statement
  static void setFd(fd_set *read_set, fd_set *write_set, int &maxFile);
  static bool isUdpFdSet(fd_set *read_set);
  bool	isFdSet(fd_set *set);

  /// return the opened socket, usable from all other network internal client
  static int  getUdpSocket();

  /**
      udpReceive will try to get the next udp message received

      return the playerIndex if found, -1 when no player had an open udp
      connection or -2 when a Ping Code Request has been detected.

      buffer is the received message

      uaddr is the identifier of the remote address

      udpLinkRequest report if the received message is a valid udpLinkRequest
  */
  static int  udpReceive(char *buffer, struct sockaddr_in *uaddr,
			 bool &udpLinkRequest);

  /**
     tcpReceive try to get a message from the tcp connection
     the message can be accessed by using the getTcpBuffer methods
     result value indicates:
     ReadAll    : was successfully received a full message
     ReadPart   : only part of a message has been retrieved
     ReadHuge   : length of the message is too long
     ReadReset  : a reset of the connection has been detected
     ReadError  : Error detected on the tcp connection
     ReadDiscon : Peer has closed the connection
  */
  RxStatus    tcpReceive();
  void       *getTcpBuffer();

  /// Request if there is any buffered udp messages waiting to be sent
  static bool	anyUDPPending() {return pendingUDP;};

  /// Send all buffered UDP messages, if any
  void		flushUDP();
  static void	flushAllUDP();

  int		pwrite(const void *b, int l);
  int		pflush(fd_set *set);
  std::string	reasonToKick();
  void		getPlayerList(char *list);
  const char*	getTargetIP();
  int		sizeOfIP();
  void*		packAdminInfo(void *buf);
  static int	whoIsAtIP(const std::string& IP);
  in_addr	getIPAddress();
  const char*	getHostname();

  /// Notify that the channel is going to be close.
  /// In the meantime any pwrite call will do nothing.
  /// Cannot be undone.
  void		closing();

private:
  int  send(const void *buffer, size_t length);
  void udpSend(const void *b, size_t l);
  int  bufferedSend(const void *buffer, size_t length);
  bool isMyUdpAddrPort(struct sockaddr_in uaddr);
  RxStatus    receive(size_t length);
#ifdef NETWORK_STATS
  void	countMessage(uint16_t code, int len, int direction);
  void	dumpMessageStats();
#endif
#ifdef HAVE_ADNS_H
  AdnsHandler *adns;
#endif

  /// On win32, a socket is typedef UINT_PTR SOCKET;
  /// Hopefully int will be ok
  static int		udpSocket;
  static NetHandler*	netPlayer[maxHandlers];
  PlayerInfo*		info;
  struct sockaddr_in	uaddr;
  int			playerIndex;
  /// socket file descriptor
  int			fd;

  /// peer's network address
  Address peer;

  /// input buffers
  /// current TCP msg
  char tcpmsg[MaxPacketLen];
  /// bytes read in current msg
  int tcplen;

  /// Closing flag
  bool closed;

  /// output buffer
  int outmsgOffset;
  int outmsgSize;
  int outmsgCapacity;
  char* outmsg;

  char udpOutputBuffer[MaxPacketLen];
  int udpOutputLen;
  static bool pendingUDP;

  /// UDP connection
  bool udpin; // udp inbound up, player is sending us udp
  bool udpout; // udp outbound up, we can send udp

  bool toBeKicked;
  std::string toBeKickedReason;

  // time accepted
  TimeKeeper time;
#ifdef NETWORK_STATS
  // message stats bloat
  TimeKeeper perSecondTime[2];
  uint32_t perSecondCurrentBytes[2];
  uint32_t perSecondMaxBytes[2];
  uint32_t perSecondCurrentMsg[2];
  uint32_t perSecondMaxMsg[2];
  uint32_t msgBytes[2];

  typedef std::map<const uint16_t, struct MessageCount> MessageCountMap;
  MessageCountMap msg[2];
#endif
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
