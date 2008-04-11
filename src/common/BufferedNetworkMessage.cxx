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

// Interface header
#include "BufferedNetworkMessage.h"

// System headers
#include <algorithm>
#include <errno.h>

// Common headers
#include "Pack.h"
#include "NetHandler.h"


// initialize the singleton
template <>
BufferedNetworkMessageManager* Singleton<BufferedNetworkMessageManager>::_instance = (BufferedNetworkMessageManager*)0;


BufferedNetworkMessage::BufferedNetworkMessage()
{
  dataSize = 4;
  data = NULL;
  packedSize = 0;
  readPoint = 0;

  checkData(0);

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

  MSGMGR.queueMessage(this);
}

void BufferedNetworkMessage::broadcast ( uint16_t messageCode, bool toAdminClients )
{
  recipent = NULL;
  code = messageCode;
  toAdmins = toAdminClients;
  MSGMGR.queueMessage(this);
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

void BufferedNetworkMessage::packDouble( double val )
{
  checkData(sizeof(double));
  nboPackDouble(getWriteBuffer(),val);
  packedSize += sizeof(double);
}

void BufferedNetworkMessage::packFloatVector( const float* val )
{
  checkData(sizeof(float)*3);
  nboPackFloatVector(getWriteBuffer(),val);
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

uint8_t BufferedNetworkMessage::unpackUByte( void )
{
  uint8_t v = 0;
  char *p = getReadBuffer();
  if (p)  
    packedSize += (char*)(nboUnpackUByte(p,v))-p;
  return v;
}

int16_t BufferedNetworkMessage::unpackShort( void )
{
  int16_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackShort(p,v))-p;
  return v;
}

int32_t BufferedNetworkMessage::unpackInt( void )
{
  int32_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackInt(p,v))-p;
  return v;
}

uint16_t BufferedNetworkMessage::unpackUShort( void )
{
  uint16_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUShort(p,v))-p;
  return v;
}

uint32_t BufferedNetworkMessage::unpackUInt( void )
{
  uint32_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUInt(p,v))-p;
  return v;
}

float BufferedNetworkMessage::unpackFloat( void )
{
  float v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackFloat(p,v))-p;
  return v;
}

double BufferedNetworkMessage::unpackDouble( void )
{
  double v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackDouble(p,v))-p;
  return v;
}

float* BufferedNetworkMessage::unpackFloatVector( float* val )
{
  memset(val,0,sizeof(float)*3);
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackFloatVector(p,val))-p;
  return val;
}

const std::string& BufferedNetworkMessage::unpackStdString( std::string& str )
{
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackStdString(p,str))-p;
  return str;
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

  dataSize = 4;
  data = NULL;
  packedSize = 0;
  recipent = NULL;
  readPoint = 0;
  code = 0;
}

void BufferedNetworkMessage::reset ( void )
{
  readPoint = 0;
}

size_t BufferedNetworkMessage::size ( void )
{
  return packedSize;
}

bool BufferedNetworkMessage::process ( void )
{
  if (!data)
    checkData(0);

  NetworkMessageTransferCallback *transferCallback = MSGMGR.getTransferCallback();

  if (!transferCallback || !recipent && code == 0)
    return false;

  nboPackUShort(data, uint16_t(packedSize));
  nboPackUShort(data+sizeof(uint16_t), code);

  if (recipent)
  {
    return transferCallback->send(recipent, data, packedSize+4) == packedSize+4;
  }
   
  // send message to everyone
  int mask = NetHandler::clientBZFlag;
  if (toAdmins)
    mask |= NetHandler::clientBZAdmin;
  transferCallback->broadcast(data, packedSize+4, mask, code);

  return true;
}

void BufferedNetworkMessage::checkData ( size_t s )
{
  if (!data || (packedSize + s + 4 > dataSize)) {
    dataSize = dataSize + ((int)ceil(s / 256.0f)) * 256;
    data = reinterpret_cast<char*>(realloc(data, dataSize));
  }
  if (!data) { // hmm, couldn't realloc - out of memory?
    logDebugMessage(1, "Error - out of memory reallocating buffered network message; size %d, error %d.", dataSize, errno);
    exit(2);
  }
}

char* BufferedNetworkMessage::getWriteBuffer ( void )
{
  if (!data)
    return NULL;

  return data + 4 + packedSize;
}

char* BufferedNetworkMessage::getReadBuffer ( void )
{
  if (!data)
    return NULL;

  if (readPoint >= packedSize)
    return NULL;

  return data + 4 + readPoint;
}



//BufferedNetworkMessageManager
BufferedNetworkMessage* BufferedNetworkMessageManager::newMessage ( BufferedNetworkMessage* msgToCopy )
{
  BufferedNetworkMessage *msg = NULL;
  if (msgToCopy)
    msg = new BufferedNetworkMessage(*msgToCopy);
  else
    msg = new BufferedNetworkMessage;
  pendingOutgoingMessages.push_back(msg);
  return msg;
}

size_t BufferedNetworkMessageManager::receiveMessages ( NetworkMessageTransferCallback *callback,  std::list<BufferedNetworkMessage*> &incomingMessages )
{
  BufferedNetworkMessage * msg = new BufferedNetworkMessage;

  while (callback->receive(msg))
  {
    incomingMessages.push_back(msg);
    msg = new BufferedNetworkMessage;
  }
  // clean up the extra at the end
  delete msg;

  return incomingMessages.size();
}

void BufferedNetworkMessageManager::update ( void )
{
  if (transferCallback)
    sendPendingMessages();
}

void BufferedNetworkMessageManager::sendPendingMessages ( void )
{
  while ( outgoingQueue.size() )
  {
    MessageDeque::iterator itr = outgoingQueue.begin();
    if (*itr)
    {
      (*itr)->process();
      delete(*itr);
    }
    outgoingQueue.pop_front();
  }
}

void BufferedNetworkMessageManager::queueMessage ( BufferedNetworkMessage *msg )
{
  MessageList::iterator itr = std::find(pendingOutgoingMessages.begin(),pendingOutgoingMessages.end(),msg);
  if ( itr != pendingOutgoingMessages.end() )
    pendingOutgoingMessages.erase(itr);

  outgoingQueue.push_back(msg);
}


void BufferedNetworkMessageManager::purgeMessages ( NetHandler *handler )
{
  MessageList::iterator itr = pendingOutgoingMessages.begin();
  while ( itr != pendingOutgoingMessages.end() )
  {
    if ((*itr) && (handler == (*itr)->recipent))  // just kill the message and data, it'll be pulled from the list on the next update pass
    {
      delete(*itr);
      pendingOutgoingMessages.erase(itr);
      itr = pendingOutgoingMessages.begin();
    }
    else
      itr++;
  }

  MessageDeque::iterator qItr = outgoingQueue.begin();
  while (qItr != outgoingQueue.end())
  {
    if ((*qItr) && (handler == (*qItr)->recipent))  // just kill the message and data, it'll be pulled from the list on the next update pass
    {
      delete(*qItr);
      outgoingQueue.erase(qItr);
      qItr = outgoingQueue.begin();
    }
    else
      qItr++;
  }
}

BufferedNetworkMessageManager::BufferedNetworkMessageManager()
{
  transferCallback = NULL;
}

BufferedNetworkMessageManager::~BufferedNetworkMessageManager()
{
  MessageList::iterator itr = pendingOutgoingMessages.begin();
  while ( itr != pendingOutgoingMessages.end() )
  {
      delete(*itr);
      itr++;
  }

  MessageDeque::iterator qItr = outgoingQueue.begin();
  while (qItr != outgoingQueue.end())
  {
      delete(*qItr);
      qItr++;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
