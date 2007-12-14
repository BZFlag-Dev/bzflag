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

#include "BufferedNetworkMessage.h"

// initialize the singleton
template <>
BufferedNetworkMessageManager* Singleton<BufferedNetworkMessageManager>::_instance = (BufferedNetworkMessageManager*)0;


BufferedNetworkMessage::BufferedNetworkMessage()
{
  data = NULL;
  dataSize = 0;
  packedSize = 0;

  code = 0;
  recipent = NULL;
}

BufferedNetworkMessage::BufferedNetworkMessage( const BufferedNetworkMessage &msg )
{
  packedSize = msg.packedSize;
  dataSize = msg.dataSize;
  data = (char*)malloc(dataSize);
  memcpy(data,msg.data,dataSize);
  code = msg.code;
  recipent = msg.recipent;
}

BufferedNetworkMessage::~BufferedNetworkMessage()
{
  if (data)
    free(data);
}

void BufferedNetworkMessage::send ( NetHandler *to, uint16_t messageCode )
{
}

void BufferedNetworkMessage::send ( int to, uint16_t messageCode )
{
}

void BufferedNetworkMessage::broadcast ( uint16_t messageCode )
{
}

void BufferedNetworkMessage::packUByte( uint8_t val )
{
}

void BufferedNetworkMessage::packShort( int16_t val )
{
}

void BufferedNetworkMessage::packInt( int32_t val )
{
}

void BufferedNetworkMessage::packUShort( uint16_t val )
{
}

void BufferedNetworkMessage::packUInt( uint32_t val )
{
}

void BufferedNetworkMessage::packFloat( float val )
{
}

void BufferedNetworkMessage::packVector( const float* val )
{
}

void BufferedNetworkMessage::packString( const char* val, int len )
{
}

void BufferedNetworkMessage::packStdString( const std::string& str )
{
}

void BufferedNetworkMessage::addPackedData ( const char* d, size_t s )
{
}

void BufferedNetworkMessage::clear ( void )
{
  if (data)
    free(data);

  data = NULL;
  dataSize = 0;
  packedSize = 0;
  recipent = NULL;
  code = 0;
}

size_t BufferedNetworkMessage::size ( void )
{
  return packedSize;
}

bool BufferedNetworkMessage::process ( void )
{
  return true;
}

void BufferedNetworkMessage::allocateInitialData ( void )
{
  if (data)
    free(data);

  packedSize = 0;
  dataSize = 256;
  data = (char*)malloc(256);
}

void BufferedNetworkMessage::growData ( size_t s )
{
  char *p = (char*)malloc(dataSize + s);
  memcpy(p,data,dataSize);
  dataSize += s;
  free(data);
  data = p;
}

void BufferedNetworkMessage::checkData ( size_t s )
{
  if ( !data || dataSize == 0 )
    allocateInitialData();

  if ( packedSize + s > dataSize )
    growData(s);
}


//BufferedNetworkMessageManager
BufferedNetworkMessage* BufferedNetworkMessageManager::newMessage ( void )
{
  BufferedNetworkMessage *msg = new BufferedNetworkMessage;
  messages.push_back(msg);
  return msg;
}

void BufferedNetworkMessageManager::sendPendingMessages ( void )
{
  std::list<BufferedNetworkMessage*>::iterator itr = messages.begin();
  while ( itr != messages.end() )
  {
    if (*itr)
    {
      (*itr)->process();
      delete(*itr);
    }
    itr++;
  }

  messages.clear();
}

void BufferedNetworkMessageManager::purgeMessages ( NetHandler *handler )
{
  std::list<BufferedNetworkMessage*>::iterator itr = messages.begin();
  while ( itr != messages.end() )
  {
    if (handler == (*itr)->recipent)  // just kill the message and data, it'll be pulled from the list on the next update pass
    {
      delete(*itr);
      *itr = NULL;
    }
    itr++;
  }
}

BufferedNetworkMessageManager::BufferedNetworkMessageManager()
{
}

BufferedNetworkMessageManager::~BufferedNetworkMessageManager()
{
  std::list<BufferedNetworkMessage*>::iterator itr = messages.begin();
  while ( itr != messages.end() )
  {
    if (*itr)
      delete(*itr);
    itr++;
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
