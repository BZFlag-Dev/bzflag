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
}

BufferedNetworkMessage::BufferedNetworkMessage( const BufferedNetworkMessage &msg )
{
}

BufferedNetworkMessage::~BufferedNetworkMessage()
{
}

void BufferedNetworkMessage::send ( NetHandler *to, uint16_t messageCode )
{
}

void BufferedNetworkMessage::send ( int to, uint16_t messageCode )
{
}

void BufferedNetworkMessage::broadcast ( uint16_t messageCode, bool toAll )
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
}

size_t BufferedNetworkMessage::size ( void )
{
  return 0;
}

bool BufferedNetworkMessage::process ( void )
{
  return true;
}


void BufferedNetworkMessage::checkData ( size_t s )
{
}

//BufferedNetworkMessageManager
BufferedNetworkMessage* BufferedNetworkMessageManager::newMessage ( BufferedNetworkMessage */*msg*/ )
{
  return NULL;
}

void BufferedNetworkMessageManager::sendPendingMessages ( void )
{

}

void BufferedNetworkMessageManager::purgeMessages ( NetHandler *handler )
{

}

BufferedNetworkMessageManager::BufferedNetworkMessageManager()
{
}

BufferedNetworkMessageManager::~BufferedNetworkMessageManager()
{
 
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
