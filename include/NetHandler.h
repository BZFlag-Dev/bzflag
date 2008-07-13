/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include <set>

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

  virtual void networkDataLog ( bool send, bool udp, const unsigned char *data, unsigned int size, void *param = NULL ) = 0;
};

void addNetworkLogCallback(NetworkDataLogCallback * cb );
void removeNetworkLogCallback(NetworkDataLogCallback * cb );

void setNoDelay(SOCKET fd);

class NetHandler;
/**
 * NetHandlerCloseCallback is an abstract base class for user classes
 * to derive from. When registered with a NetHandler, the callback()
 * method will be invoked when the NetHandler is inactivated.
 */
class NetHandlerCloseCallback
{
public:
  virtual ~NetHandlerCloseCallback() {}

  virtual void callback(NetHandler*, char const* reason) = 0;

  /**
   * Get rid of the callback when it is no longer needed.
   *
   * If an application dynamically allocates these objects, this
   * method should free the memory. The default implementation does
   * nothing.
   */
  virtual void dispose() {};
};

/**
 * The NetHandler provides an abstraction over socket writes and
 * mostly reads. Each NetHandler represents the network connection
 * (both TCP and UDP) between two endpoints (e.g., bzfs server and
 * bzflag client).
 *
 * Users of the NetHandler class request messages. For no particular
 * reason, precedence is given to datagram (UDP) messages.
 *
 * As an implementation detail, each NetHandler contains a buffer for
 * data received over TCP and a separate buffer for UDP datagrams.
 */
class NetHandler {
  friend class NetListener;
public:
  /**
   * Static Area
   *    Things here fall into the following broad categories:
   *	-  UDP (only one UDP socket)
   *    -  Iterators: operate over all NetHandlers
   *    -  DNS: asynchronous DNS queries
   */

  /** Class-Wide initialization
      Should be called before any other operation on any class instance
      InitHandlers needs the addr structure filled to point to the local port
      needed for udp communications
  */
  static bool initHandlers(struct sockaddr_in addr);

  // Iterators......
  /**
   * Fills the passed fd_sets with the file descriptors associated
   * with all the existing NetHandlers and updates maxFile
   * appropriately
   */
  static void setFd(fd_set *read_set, fd_set *write_set, int &maxFile);

// TODO: These public accessors to the UDP socket need to go
// away. However, until the Ping class gets modified to work with
// (rather than against) the NetHandler, it has to stay.
  // UDP......
  static bool isUdpFdSet(fd_set *read_set);
// private:
  /// return the opened socket, usable from all other network internal client
  static SOCKET  getUdpSocket();
public:
  /**
   * udpReceive will try to get the next udp message received
   *
   * @return  return the playerIndex if found, -1 when no player had an open udp
   * connection or -2 when a Ping Code Request has been detected.
   *  
   * @param  buffer is the received message
   *
   * @param  uaddr_ is the identifier of the remote address
   *
   * @param netHandler is the NetHandler associated with this address
   */
  static int  udpReceive(char *buffer, struct sockaddr_in *uaddr_,
			 NetHandler*& netHandler);

  /// Request if there is any buffered udp messages waiting to be sent
  static bool	anyUDPPending() {return pendingUDP;};

  /// Send all buffered UDP messages, if any
  static void	flushAllUDP();



  // DNS...
  /// Supporting DNS Asynchronous resolver
  static void checkDNS(fd_set *read_set, fd_set *write_set);

  static NetHandler* whoIsAtIP(const std::string& IP);

  static const int     clientNone    = 0;
  static const int     clientBZAdmin = 1;
  static const int     clientBZFlag  = 2;

  static void setCurrentTime(TimeKeeper tm);

  typedef std::list<NetHandler*> NetConnections;
  static NetConnections netConnections;


  /** Constructor
   * This constructor is for servers that are listening for connections
   * @param _clientAddr address of the client
   * @param _fd bound TCP socket
   **/
  NetHandler(const struct sockaddr_in &_clientAddr, SOCKET _fd);

  /** Constructor
   * This constructor is for clients that will connect to a server
   * @param serverAddr address of the server to connect to
   **/
  NetHandler(const struct sockaddr_in& serverAddr);

  /** Destructor
   *  free all internal resources, and close the tcp connection
   */
  ~NetHandler();

  /**
   * This allows an application to attach "extra" info to the bound
   * connection, to be used in an application specific manner. The
   * application is responsible for managing the memory associated
   * with the extra info, perhaps through a NetHandlerCloseCallback.
   */
  void setExtraInfo(void* info);
  void* getExtraInfo() const;

  void addCloseCallback(NetHandlerCloseCallback*);
  void removeCloseCallback(NetHandlerCloseCallback*);

  /**
   * Determine if data is available from any source (UDP/TCP)
   */
  bool hasData() const;

  enum Status { NoMsg, GoodMsg, EMsgTooLarge };
  typedef Status (*FramingFunction)(char* src, size_t src_len, char* dst, size_t& dst_len);
  /**
   * getMsg() fills the passed buffer with the next available message
   * in the buffer(s).
   *
   * @result Returns the status of the operation. If a full message
   * could not be provided (possible network latency) then NoMsg is
   * returned. If the message will not fit in the passed buffer,
   * EMsgTooLarge is returned and the message is discarded. GoodMsg is
   * returned on success.
   *
   * @param framer is a function that detects message boundaries. This
   * allows for a variety of protocols to use this generic interface
   * layer.
   *
   * @param buf an allocated buffer large enough to hold the largest
   * message
   *
   * @param len on input, the size of the buffer. On output, the size
   * of the message
   *
   * @param isUDP returns whether the message was received from UDP or
   * not. This is of questionable utility, but is present to minimize
   * the impact on existing logic. Eventually, all application logic
   * that depends on whether a message was from TCP or UDP needs to be
   * rethought.
   */
  Status getMsg(FramingFunction framer, char* buf, size_t& len, bool& isUDP);

  /**
   * Send data for transmission on the tcp channel.
   *
   * If channel is not ready to accept other data, it just buffer it
   *
   * @return 0 if all went ok
   *        -1 if got an error
   *        -2 if tcp buffer is going to be too much big
   *
   * @param buffer the buffer to send
   *
   * @param length how many bytes to send
   */
  int  bufferedSend(const void *buffer, size_t length);

  /**
   * Check if this connection is in the passed fd_set
   *
   * @param set an fd_set presumably populated from a call to select()
   */
  bool	isFdSet(fd_set *set);

  /// Send all buffered UDP messages, if any
  void		flushUDP();

  int		pwrite(const void *b, int l);
  int		pflush();
  void		getPlayerList(char* list, size_t listSize);
  const char*	getTargetIP();
  int		sizeOfIP();
  void*		packAdminInfo(void *buf);
  in_addr	getIPAddress();
  const char*	getHostname();
  bool		reverseDNSDone();

  void		setClientKind(int kind);
  int		getClientKind();

  /// Notify that the channel is going to be close.
  /// In the meantime any pwrite call will do nothing.
  /// Cannot be undone.
  ///
  /// @param reason a text reason why this connection is being closed
  void		closing(char const* reason);

  void	  setUDPin(struct sockaddr_in *uaddr_);

  bool isMyUdpAddrPort(struct sockaddr_in uaddr_, bool checkPort);

  int  send(const void *buffer, size_t length);

  RxStatus    receive(size_t length);

  SOCKET getFD ( void ) {return fd;}

protected:
  /// I don't know if we really want to subclass this or not, but just
  /// in case, these should be accessible?

  /**
   * Read available data from the bound TCP socket into the message
   * buffer
   */
  void tcpRead();

private:
  void udpSend(const void *b, size_t l);

  typedef std::list<NetHandlerCloseCallback*> CloseCallbacks;
  CloseCallbacks closeCallbacks;

#ifdef NETWORK_STATS
  void	countMessage(uint16_t code, int len, int direction);
  void	dumpMessageStats();
#endif
  AresHandler	   ares;

  /// On win32, a socket is typedef UINT_PTR SOCKET;
  static SOCKET		udpSocket;
  /// current UDP msg
  static char	        udpmsg[MaxPacketLen];
  static int		udpLen;
  /// bytes read in current msg
  static int		udpRead;
  static struct sockaddr_in lastUDPRxaddr;
  static bool pendingUDP;

  struct sockaddr_in	uaddr;
  /// socket file descriptor
  SOCKET		fd;

  int clientType;

  /// peer's network address
  Address peer;
  /* peer->getDotNotation returns a temp variable that is not safe
   * to pass around.  This variable lets us keep a copy in allocated
   * memory for as long as we need to */
  std::string dotNotation;

  /// input buffers
  /// current UDP msg
  char	        udpmsg2[MaxPacketLen];
  size_t	udpLen2;
  /// current TCP msg
  char tcpmsg[MaxPacketLen];
  size_t	tcplen;

  /// Closing flag
  bool closed;

  /// output buffer
  int outmsgOffset;
  int outmsgSize;
  int outmsgCapacity;
  char* outmsg;

  char udpOutputBuffer[MaxPacketLen];
  int udpOutputLen;

  /// UDP connection
  bool udpin; // udp inbound up, player is sending us udp
  bool udpout; // udp outbound up, we can send udp

  // FIXME... this is protocol and doesn't belong here?
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

  /// Extra info that an application can attach to this connection
  void*		extraInfo;
};

/**
 * NewNetworkConnectionCallback is a the base class for user-defined
 * socket connection management.
 */
class NewNetworkConnectionCallback
{
public:
  virtual ~NewNetworkConnectionCallback(){};

  /**
   * perform application-specific initialization of new connections.
   *
   * @param handler encapsulates the network connection.
   * @param connectionID perhaps unused (arbitrary?) connection identifier
   */
  virtual bool accept ( NetHandler* handler, int connectionID ) = 0;
};


class NetworkDataPendingCallback
{
public:
  virtual ~NetworkDataPendingCallback(){};

  // When this NetHandler has data available to read
  virtual bool pendingRead( NetHandler* handler ) = 0;

  // When this NetHandler has data available to write
  virtual bool pendingWrite( NetHandler* handler ) = 0;

  // When no action is required for this NetHandler
  virtual bool idle( NetHandler* handler ) = 0;
};

class NetworkUDPReadCallback
{
public:
  virtual ~NetworkUDPReadCallback() {}

  virtual void callback() = 0;
};

class NetListener
{
public:
  NetListener();
  ~NetListener();

  // Create a socket an listen on the specified port. In case of a
  // bind error, use an ephemeral port if 'fallback' is true
  bool listen ( Address serverAddress, unsigned short port, bool fallback=false );

  // Return the address of the bound socket, or 0 if the socket isn't
  // bound. Useful if 'fallback' was enabled on listen()
  sockaddr_in address();

  // Terminate this listener
  void shutdown();
private:
  bool close ( NetHandler *handler );
  bool close ( int connectionID );
public:

  int update ( float waitTime );

  void process ( void );

  /**
   * NewNetworkConectionCallback is invoked when a remote client is
   * accepted as a network connection.
   **/
  void addNewConnectionCallback ( NewNetworkConnectionCallback *handler );
  void removeNewConnectionCallback  ( NewNetworkConnectionCallback *handler );

  /**
   * NetworkDataPendingCallback is invoked for existing connections
   * whenever the network is able to communicate, or the idle() method
   * is invoked if it isn't
   **/
  void addDataPendingCallback( NetworkDataPendingCallback *handler );
  void removeDataPendingCallback( NetworkDataPendingCallback *handler );

  /**
   * NetworkUDPReadCallback is invoked when there is data available
   * from the UDP socket associated with a network client
   **/
  void addUDPReadCallback( NetworkUDPReadCallback* handler );
  void removeUDPReadCallback( NetworkUDPReadCallback* handler );

protected:
  SOCKET  listenSocket;

  int	  toRead;

  int	  maxFileDescriptors;
  fd_set  read_set;
  fd_set  write_set;

  typedef std::set<NewNetworkConnectionCallback*> NewNetworkConnectionCallbacks;
  typedef std::set<NetworkDataPendingCallback*>   DataPendingCallbacks;
  typedef std::set<NetworkUDPReadCallback*>	  UdpReadCallbacks;

  NewNetworkConnectionCallbacks		newConnectionCallbacks;
  DataPendingCallbacks			dataPendingCallbacks;
  UdpReadCallbacks			udpReadCallbacks;

  void accept ( void );
  void fail( char const* reason );
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
