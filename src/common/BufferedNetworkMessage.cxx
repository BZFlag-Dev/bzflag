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
#include "Pack.h"
//#include "GameKeeper.h"
//#include "bzfs.h"
//#include "bzfsMessages.h"
#include "NetHandler.h"


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
  toAdmins = false;
}

BufferedNetworkMessage::BufferedNetworkMessage( const BufferedNetworkMessage &msg )
{
  packedSize = msg.packedSize;
  dataSize = msg.dataSize;
  data = (char*)malloc(dataSize);
  memcpy(data,msg.data,dataSize);
  code = msg.code;
  recipent = msg.recipent;
  toAdmins = msg.toAdmins;
}

BufferedNetworkMessage::~BufferedNetworkMessage()
{
  if (data)
    free(data);
}

void BufferedNetworkMessage::send ( NetHandler *to, uint16_t messageCode )
{
  recipent = to;
  code = messageCode;
}

/*void BufferedNetworkMessage::send ( int to, uint16_t messageCode )
{
  recipent = getPlayerNetHandler(to);
  code = messageCode;
} */

void BufferedNetworkMessage::broadcast ( uint16_t messageCode, bool toAdminClients )
{
  recipent = NULL;
  code = messageCode;
  toAdmins = toAdminClients;
}

void BufferedNetworkMessage::packUByte( uint8_t val )
{
  checkData(sizeof(uint8_t));
  nboPackUByte(getWriteBuffer(),val);
  packedSize += sizeof(uint8_t);
}

void BufferedNetworkMessage::packShort( int16_t val )
{
  checkData(sizeof(int16_t));
  nboPackShort(getWriteBuffer(),val);
  packedSize += sizeof(int16_t);
}

void BufferedNetworkMessage::packInt( int32_t val )
{
  checkData(sizeof(int32_t));
  nboPackInt(getWriteBuffer(),val);
  packedSize += sizeof(int32_t);
}

void BufferedNetworkMessage::packUShort( uint16_t val )
{
  checkData(sizeof(uint16_t));
  nboPackUShort(getWriteBuffer(),val);
  packedSize += sizeof(uint16_t);
}

void BufferedNetworkMessage::packUInt( uint32_t val )
{
  checkData(sizeof(uint32_t));
  nboPackUInt(getWriteBuffer(),val);
  packedSize += sizeof(uint32_t);
}

void BufferedNetworkMessage::packFloat( float val )
{
  checkData(sizeof(float));
  nboPackFloat(getWriteBuffer(),val);
  packedSize += sizeof(float);
}

void BufferedNetworkMessage::packVector( const float* val )
{
  checkData(sizeof(float)*3);
  nboPackVector(getWriteBuffer(),val);
  packedSize += sizeof(float)*3;
}

void BufferedNetworkMessage::packString( const char* val, int len )
{
  checkData(len);
  nboPackString(getWriteBuffer(),val,len);
  packedSize += len;
}

void BufferedNetworkMessage::packStdString( const std::string& str )
{
  checkData(str.size()+sizeof(uint32_t));
  nboPackStdString(getWriteBuffer(),str);
  packedSize += str.size()+sizeof(uint32_t);
}

void BufferedNetworkMessage::addPackedData ( const char* d, size_t s )
{
  checkData(s);
  memcpy(getWriteBuffer(),d,s);
  packedSize += s;
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
  NetworkMessageTransferCallback *transferCallback = MSGMGR.getTransferCallback();

  if (!transferCallback || !recipent && code == 0)
    return false;

  nboPackUShort(data, uint16_t(packedSize));
  nboPackUShort(data+sizeof(uint16_t), code);

  if (recipent)
  {
    return transferCallback->send(recipent, data, packedSize + 4) == packedSize+4;
  }
   
  // send message to everyone
  int mask = NetHandler::clientBZFlag;
  if (toAdmins)
    mask |= NetHandler::clientBZAdmin;
  transferCallback->broadcast(data, packedSize + 4, mask, code);

  return true;
}

void BufferedNetworkMessage::checkData ( size_t s )
{
  if ( packedSize + s > dataSize )
    data = reinterpret_cast<char*>(realloc(data, dataSize + ((int)ceil((s+4) / 256.0f)) * 256));
}

char* BufferedNetworkMessage::getWriteBuffer ( void )
{
  if (!data)
    return NULL;

  return data + 4 + packedSize;
}


//BufferedNetworkMessageManager
BufferedNetworkMessage* BufferedNetworkMessageManager::newMessage ( BufferedNetworkMessage* msgToCopy )
{
  BufferedNetworkMessage *msg = NULL;
  if (msgToCopy)
    msg = new BufferedNetworkMessage(*msgToCopy);
  else
    msg = new BufferedNetworkMessage;
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
  transferCallback = NULL;
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
