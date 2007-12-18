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

#ifndef _BUFFERED_NETWORK_MESSAGE_H_
#define _BUFFERED_NETWORK_MESSAGE_H_

#include "common.h"
#include "Singleton.h"
#include <list>
#include <deque>

class NetHandler;

class BufferedNetworkMessageManager;

class BufferedNetworkMessage
{
public:
    BufferedNetworkMessage();
    BufferedNetworkMessage( const BufferedNetworkMessage &msg );

    virtual ~BufferedNetworkMessage();

    // sending
    void send ( NetHandler *to, uint16_t messageCode );
    void broadcast ( uint16_t messageCode, bool toAdminClients = true );

    void packUByte( uint8_t val );
    void packShort( int16_t val );
    void packInt( int32_t val );
    void packUShort( uint16_t val );
    void packUInt( uint32_t val );
    void packFloat( float val );
    void packVector( const float* val );
    void packString( const char* val, int len );
    void packStdString( const std::string& str );

 /*   uint8_t unpackUByte( uint8_t val );
    int16_t unpackShort( int16_t val );
    int32_t unpackInt( int32_t val );
    uint16_t unpackUShort( uint16_t val );
    uint32_t unpackUInt( uint32_t val );
    float unpackFloat( float val );
    float* unpackVector( float* val );
    const std::string& unpackStdString( std::string& str ); */

    void addPackedData ( const char* d, size_t s );

    void clear ( void );
   // void reset ( void );
   // void flush ( void );

    size_t size ( void );
    char * buffer ( void ) {return data+4;}

    void setCode ( uint16_t c ) { code = c; }

protected:
  friend class BufferedNetworkMessageManager;
  bool process ( void );

  char* getWriteBuffer ( void );

  void checkData ( size_t s );

  char *data;
  size_t dataSize;
  size_t packedSize;

  uint16_t    code;
  NetHandler  *recipent;  // NULL if broadcast;
  bool	      toAdmins;
};

class NetworkMessageTransferCallback
{
public:
  virtual ~NetworkMessageTransferCallback(){};

  virtual size_t send ( NetHandler* /*handler*/, void * /*data*/, size_t /*size*/ ){return 0;}
  virtual size_t broadcast ( void */*data*/, size_t /*size*/, int/* mask*/, int /*code*/  ){return 0;}

  virtual size_t receive ( uint16_t /*&code*/, BufferedNetworkMessage */*message*/ ){return 0;}
};

class BufferedNetworkMessageManager : public Singleton<BufferedNetworkMessageManager>
{
public:
  BufferedNetworkMessage  *newMessage ( BufferedNetworkMessage *msgToCopy = NULL );

  size_t receiveMessages ( NetworkMessageTransferCallback *callback,  std::list<BufferedNetworkMessage*> &incomingMessages );

  void sendPendingMessages ( void );

  void purgeMessages ( NetHandler *handler );

  void setTransferCallback ( NetworkMessageTransferCallback *cb ){ transferCallback = cb;}
  NetworkMessageTransferCallback* getTransferCallback ( void ){return transferCallback;}

  void queMessage ( BufferedNetworkMessage *msg );

protected:
  friend class Singleton<BufferedNetworkMessageManager>;

  typedef std::list<BufferedNetworkMessage*> MessageList;
  MessageList pendingOutgingMesages;

  typedef std::deque<BufferedNetworkMessage*> MessageDeque;
  MessageDeque outgoingQue;

  std::list<BufferedNetworkMessage*> incomingMesages;

  NetworkMessageTransferCallback *transferCallback;
private:
  BufferedNetworkMessageManager();
  ~BufferedNetworkMessageManager();
};  

#define MSGMGR (BufferedNetworkMessageManager::instance())

#define NetMsg BufferedNetworkMessage*

#endif //_BUFFERED_NETWORK_MESSAGE_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
