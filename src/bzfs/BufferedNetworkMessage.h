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

#include "NetHandler.h"
#include <list>

class BufferedNetworkMessageManager;

class BufferedNetworkMessage
{
public:
    BufferedNetworkMessage();
    BufferedNetworkMessage( const BufferedNetworkMessage &msg );

    virtual ~BufferedNetworkMessage();

    void send ( NetHandler *to, uint16_t messageCode );
    void send ( int to, uint16_t messageCode );
    void broadcast ( uint16_t messageCode );

    void packUByte( uint8_t val );
    void packShort( int16_t val );
    void packInt( int32_t val );
    void packUShort( uint16_t val );
    void packUInt( uint32_t val );
    void packFloat( float val );
    void packVector( const float* val );
    void packString( const char* val, int len );
    void packStdString( const std::string& str );

    void addPackedData ( const char* d, size_t s );

    void clear ( void );

    size_t size ( void );

protected:
  friend BufferedNetworkMessageManager;
  bool process ( void );

  void allocateInitialData ( void );
  void growData ( size_t s );

  void checkData ( size_t s );

  char *data;
  size_t dataSize;
  size_t packedSize;

  uint16_t    code;
  NetHandler  *recipent;  // NULL if broadcast;
};

class BufferedNetworkMessageManager
{
public:
  BufferedNetworkMessage  *newMessage ( void );

  void sendPendingMessages ( void );

  void purgeMessages ( NetHandler *handler );

  BufferedNetworkMessageManager& instance ( void )
  {
    static BufferedNetworkMessageManager mgr;
    return mgr;
  }

protected:
  std::list<BufferedNetworkMessage*> messages;

private:
  BufferedNetworkMessageManager();
  ~BufferedNetworkMessageManager();
};  

#define MSGMGR BufferedNetworkMessageManager

#endif //_BUFFERED_NETWORK_MESSAGE_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
