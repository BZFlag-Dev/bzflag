/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __NETHANDLER_H__
#define __NETHANDLER_H__

/* common header */
#include "common.h"

/* system headers */
#include <string>
#include <map>
#include <list>

/* common interface headers */
#include "PlayerInfo.h"
#include "Address.h"
#include "AresHandler.h"

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

class NetworkDataLogCallback
{
public:
  virtual ~NetworkDataLogCallback(){};

  virtual void networkDataLog ( bool send, bool udp, const unsigned char *data, unsigned int size ) = 0;
};

void addNetworkLogCallback(NetworkDataLogCallback * cb );
void removeNetworkLogCallback(NetworkDataLogCallback * cb );

void setNoDelay(int fd);

/** This class is a client that connects to a BZFlag client and has
    functions for sending and receiving messages.
*/
class NetHandler {
public:
  /** A default constructor.
      It needs:
      a socket address to address subsequent message at user,
      the file descriptor for the TCP connection with the user.
  */
  NetHandler(const struct sockaddr_in &_clientAddr, int _fd);
  /** The default destructor
      free all internal resources, and close the tcp connection
  */
  ~NetHandler();

  /** Class-Wide initialization
      Should be called before any other operation on any clas instance
      InitHandlers needs the addr structure filled to point to the local port
      needed for udp communications
  */
  static bool initHandlers(struct sockaddr_in addr);

  /// General function to support the select statement
  static void setFd(fd_set *read_set, fd_set *write_set, int &maxFile);
  static bool isUdpFdSet(fd_set *read_set);
  bool	isFdSet(fd_set *set);

  /// Supporting DNS Asynchronous resolver
  static void checkDNS(fd_set *read_set, fd_set *write_set);

  /// return the opened socket, usable from all other network internal client
  static int  getUdpSocket();

  /**
      udpReceive will try to get the next udp message received

      return the playerIndex if found, -1 when no player had an open udp
      connection or -2 when a Ping Code Request has been detected.

      buffer is the received message

      uaddr is the identifier of the remote address
  */
  static int  udpReceive(char *buffer, struct sockaddr_in *uaddr,
			 NetHandler **netHandler);

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
  RxStatus    tcpReceive(bool doCodes = true);
  void       *getTcpBuffer();
  unsigned int getTcpReadSize ( void  );

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
  static NetHandler *whoIsAtIP(const std::string& IP);
  in_addr	getIPAddress();
  const char*	getHostname();
  bool	  reverseDNSDone();

  static const int     clientNone    = 0;
  static const int     clientBZAdmin = 1;
  static const int     clientBZFlag  = 2;
  void	  setClientKind(int kind);
  int	   getClientKind();

  /// Notify that the channel is going to be close.
  /// In the meantime any pwrite call will do nothing.
  /// Cannot be undone.
  void		closing();

  void	  setUDPin(struct sockaddr_in *uaddr);

  static void setCurrentTime(TimeKeeper tm);

  bool isMyUdpAddrPort(struct sockaddr_in uaddr, bool checkPort);

  static std::list<NetHandler*> netConnections;

  int  send(const void *buffer, size_t length);
 /// Send data for transmission on the tcp channel.
  ///   in case channel is not ready to accept other data, it just buffer it
  /// Return 0 if all went ok
  ///       -1 if got an error
  ///       -2 if tcp buffer is going to be too much big
  int  bufferedSend(const void *buffer, size_t length);

  RxStatus    receive(size_t length);

  void flushData ( void ){tcplen = 0;}

  int getFD ( void ) {return fd;}

private:
  void udpSend(const void *b, size_t l);

#ifdef NETWORK_STATS
  void	countMessage(uint16_t code, int len, int direction);
  void	dumpMessageStats();
#endif
  AresHandler	   ares;

  /// On win32, a socket is typedef UINT_PTR SOCKET;
  /// Hopefully int will be ok
  static int		udpSocket;
  struct sockaddr_in	uaddr;
  /// socket file descriptor
  int			fd;

  int clientType;

  /// peer's network address
  Address peer;
  /* peer->getDotNotation returns a temp variable that is not safe
   * to pass around.  This variable lets us keep a copy in allocated
   * memory for as long as we need to */
  std::string dotNotation;

  /// input buffers
  /// current TCP msg
  char tcpmsg[MaxPacketLen];
  /// bytes read in current msg
  int tcplen;
  /// current UDP msg
  static char	       udpmsg[MaxPacketLen];
  static int		udpLen;
  /// bytes read in current msg
  static int		udpRead;
  static struct sockaddr_in lastUDPRxaddr;

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
  static TimeKeeper now;

#ifdef NETWORK_STATS

  bool     messageExchanged;
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

class NewNetworkConnectionCallback
{
public:
  virtual ~NewNetworkConnectionCallback(){};

  virtual bool accept ( NetHandler *handler, int connectionID ) = 0;
};


class NetworkDataPendingCallback
{
public:
    virtual ~NetworkDataPendingCallback(){};
    virtual bool pending ( NetHandler *handler, int connectionID, bool tcp ) = 0;
    virtual bool disconected ( NetHandler *handler, int connectionID ) = 0;
};

class NetListener
{
public:
  NetListener();
  ~NetListener();

  bool listen ( Address serverAddress, unsigned short port );

  bool close ( NetHandler *handler );
  bool close ( int connectionID );

  int update ( float waitTime );

  void processConnections ( void );

  void addNewConnectionCallback ( NewNetworkConnectionCallback *handler );
  void removeNewConnectionCallback  ( NewNetworkConnectionCallback *handler );

  void addDataPendingCallback( NetworkDataPendingCallback *handler );
  void removeDataPendingCallback( NetworkDataPendingCallback *handler );

protected:
  int	  listenSocket;

  int	  toRead;

  int	  maxFileDescriptors;
  fd_set  read_set;
  fd_set  write_set;

  std::map<int,NetHandler*> handlers;

  std::vector<NewNetworkConnectionCallback*>	newConnectionCallbacks;
  std::vector<NetworkDataPendingCallback*>	dataPendingCallbacks;

  void accept ( void );
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
