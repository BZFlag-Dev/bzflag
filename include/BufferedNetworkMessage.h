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

    uint8_t unpackUByte( void );
    int16_t unpackShort( void );
    int32_t unpackInt( void );
    uint16_t unpackUShort( void );
    uint32_t unpackUInt( void );
    float unpackFloat( void );
    float* unpackVector( float* val );
    const std::string& unpackStdString( std::string& str ); 

    void clear ( void );
    void reset ( void );

    size_t size ( void );
    char * buffer ( void ) {return data+4;}
    void addPackedData ( const char* d, size_t s );

    void setCode ( uint16_t c ) { code = c; }
    uint16_t getCode ( void ) { return code; }

protected:
  friend class BufferedNetworkMessageManager;
  bool process ( void );

  char* getWriteBuffer ( void );
  char* getReadBuffer ( void );

  void checkData ( size_t s );

  char *data;
  size_t dataSize;
  size_t packedSize;
  size_t readPoint;

  uint16_t    code;
  NetHandler  *recipent;  // NULL if broadcast;
  bool	      toAdmins;
};

class NetworkMessageTransferCallback
{
public:
  virtual ~NetworkMessageTransferCallback(){};

  virtual size_t send ( NetHandler* /*handler*/, void * /*data*/, size_t /*size*/ ){return 0;}
  virtual size_t broadcast ( void * /*data*/, size_t /*size*/, int/* mask*/, int /*code*/  ){return 0;}

  virtual size_t receive ( BufferedNetworkMessage * /*message*/ ){return 0;}
};

class BufferedNetworkMessageManager : public Singleton<BufferedNetworkMessageManager>
{
public:
  BufferedNetworkMessage  *newMessage ( BufferedNetworkMessage *msgToCopy = NULL );

  typedef std::list<BufferedNetworkMessage*> MessageList;

  size_t receiveMessages ( NetworkMessageTransferCallback *callback,  MessageList &incomingMessages );

  void update ( void );

  void sendPendingMessages ( void );
  void clearDeadIncomingMessages ( void );

  void purgeMessages ( NetHandler *handler );

  void setTransferCallback ( NetworkMessageTransferCallback *cb ){ transferCallback = cb;}
  NetworkMessageTransferCallback* getTransferCallback ( void ){return transferCallback;}

  void queueMessage ( BufferedNetworkMessage *msg );

protected:
  friend class Singleton<BufferedNetworkMessageManager>;

  MessageList pendingOutgoingMesages;

  typedef std::deque<BufferedNetworkMessage*> MessageDeque;
  MessageDeque outgoingQueue;

  MessageList incomingMesages;

  NetworkMessageTransferCallback *transferCallback;
private:
  BufferedNetworkMessageManager();
  ~BufferedNetworkMessageManager();
};  

#define MSGMGR (BufferedNetworkMessageManager::instance())

#define NetMsg BufferedNetworkMessage*

#endif //_BUFFERED_NETWORK_MESSAGE_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
