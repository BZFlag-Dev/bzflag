/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__RCLINK_H__
#define	__RCLINK_H__

#include "common.h"

/* system interface headers */
#include <ostream>
#include <sstream>

/* common interface headers */
#include "global.h"
#include "Address.h"
#include "TimeKeeper.h"

/* local interface headers */
#include "RCMessage.h"


#define RC_LINK_RECVBUFLEN 100000
#define RC_LINK_SENDBUFLEN 100000
#define RC_LINK_MAXARGS 50
#define RC_LINK_OVERFLOW_MSG "\nerror Connection Stalled.  RC stopped reading data!\n"

#define PROTOCOL_DEBUG


/**
 * Remote Control Link: Encapsulates communication between backend and
 * frontend.  (This is the generic base-functionality)
 */
class RCLink {
public:
  typedef enum {
    Disconnected,
    SocketError,
    Listening,
    Connecting,
    Connected
  } State;

  RCLink(std::ostream *logger);
  virtual ~RCLink();

  bool connect(const char *host, int port);
  void startListening(int port);
  virtual bool tryAccept();
  virtual State getDisconnectedState() = 0;

  virtual bool parseCommand(char *cmdline) = 0;
  int updateParse(int maxlines = 0);
  int updateWrite();
  int updateRead();
  void detachAgents();
  bool waitForData();
  State getStatus() const { return status; }
  const std::string &getError() const { return error; }

  virtual bool send(const char *message);
  virtual bool sendf(const char *format, ...) BZ_ATTR_23;

  template<class C>
  bool send(const RCMessage<C> *message)
  {
    return sendf("%s\n", message->asString().c_str());
  }
  template<class C>
  bool send(const RCMessage<C> &message)
  {
    return send(&message);
  }

protected:
  bool isFrontEnd;

  State status;
  int listenfd, connfd;
  char recvbuf[RC_LINK_RECVBUFLEN];
  char sendbuf[RC_LINK_SENDBUFLEN];
  int recv_amount, send_amount;
  bool input_toolong, output_overflow;
  std::string error;
  bool vsendf(const char *format, va_list ap);

private:
  std::ostream *specificLogger;
};

// Cheap ass packet system for local only transfers

class CLocalTransferPacket
{
public:
  unsigned int size;
  char *data;

  CLocalTransferPacket(unsigned int _size = 0, const char* _data = NULL)
  {
    size = _size;
    if (_data) {
      data = (char*)malloc(size);
      memcpy(data,_data,size);
    } else {
    data = NULL;
    }
  }

  ~CLocalTransferPacket()
  {
    if (data)
      free(data);
  }

  CLocalTransferPacket(const CLocalTransferPacket& r)
  {
    size = r.size;
    if (r.data) {
      data = (char*)malloc(size);
      memcpy(data,r.data,size);
    } else {
      data = NULL;
    }
  }
};

extern std::vector<CLocalTransferPacket> messagesToFront,messagesToBack;
extern bool fakeNetConnect;

void fakenetConnectFrontToBack ( void );
void fakenetDisconect ( void );

void fakeNetResetFrontEnd ( void );
void fakeNetResetBackEnd ( void );

void fakeNetSendToFrontEnd( unsigned int s, const char *d );
void fakeNetSendToBackEnd( unsigned int s, const char *d );

unsigned int fakeNetPendingFrontEnd( void );
unsigned int fakeNetPendingBackEnd( void );

unsigned int fakeNetNextPacketSizeFrontEnd( void );
unsigned int fakeNetNextPacketSizeBackEnd( void );

char* fakeNetNextPacketDataFrontEnd( void );
char* fakeNetNextPacketDataBackEnd( void );

void fakeNetPopPendingPacketFrontEnd( void );
void fakeNetPopPendingPacketBackEnd( void );


#else
class RCLink;
class CLocalTransferPacket;
#endif /* __RCLINK_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
