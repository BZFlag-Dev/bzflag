/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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

/* local implementation headers */
#include "PlayerInfo.h"

enum RxStatus {
  ReadAll,
  ReadPart,
  ReadReset,
  ReadError,
  ReadDiscon
};

#ifdef DEBUG
#define NETWORK_STATS
#endif
#ifdef NETWORK_STATS
struct MessageCount {
  uint32_t count;
  uint16_t code;
  uint16_t maxSize;
};
// does not include MsgNull
#define MessageTypes 38
#endif

class NetHandler {
public:
  NetHandler(PlayerInfo *_info, const struct sockaddr_in &_clientAddr,
	  int _playerIndex, int _fd);
  ~NetHandler();

  static bool initNetwork(struct sockaddr_in addr);
  static int  getUdpSocket();
  static int  udpReceive(char *buffer, struct sockaddr *uaddr);
  static void fdSetUdp(fd_set *read_set, int &maxFile);
  static bool isUdpFdSet(fd_set *read_set);

  void        fdSet(fd_set *read_set, fd_set *write_set, int &maxFile);
  int         fdIsSet(fd_set *set);
  void        setUdpOut();
  bool        setUdpIn(struct sockaddr_in &_uaddr);
  int         pwrite(const void *b, int l);
  RxStatus    receive(size_t length);
  void       *getTcpBuffer();
  void        cleanTcp();
  int         pflush(fd_set *set);
  std::string reasonToKick();
#ifdef NETWORK_STATS
  void        countMessage(uint16_t code, int len, int direction);
  void        dumpMessageStats();
#endif
  bool        isMyUdpAddrPort(struct sockaddr_in &uaddr);
  void        UdpInfo();
  void        debugUdpRead(int n, struct sockaddr_in &_uaddr);
  void        getPlayerList(char *list); 
private:
  int  send(const void *buffer, size_t length);
  void udpSend(const void *b, size_t l);
  int  bufferedSend(const void *buffer, size_t length);
  static int                udpSocket;
  PlayerInfo               *info;
  struct sockaddr_in        uaddr;
  int                       playerIndex;
  // socket file descriptor
  int                       fd;

  // peer's network address
  Address                   peer;

  // input buffers
  // bytes read in current msg
  int tcplen;
  // current TCP msg
  char tcpmsg[MaxPacketLen];

  // output buffer
  int outmsgOffset;
  int outmsgSize;
  int outmsgCapacity;
  char *outmsg;

  // UDP connection
  bool udpin; // udp inbound up, player is sending us udp
  bool udpout; // udp outbound up, we can send udp

  bool                      toBeKicked;
  std::string               toBeKickedReason;

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
  struct MessageCount msg[2][MessageTypes];
#endif
};
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
