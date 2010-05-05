/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#ifdef DEBUG
#include "MsgStrings.h"
#endif


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
  recipient = NULL;
  toAdmins = false;
}

BufferedNetworkMessage::BufferedNetworkMessage(const BufferedNetworkMessage &msg)
{
  packedSize = msg.packedSize;
  dataSize = msg.dataSize;
  data = (char*)malloc(dataSize);
  memcpy(data, msg.data, dataSize);
  code = msg.code;
  recipient = msg.recipient;
  toAdmins = msg.toAdmins;
}

BufferedNetworkMessage::~BufferedNetworkMessage()
{
  if (data)
    free(data);
}

void BufferedNetworkMessage::send (NetHandler *to, uint16_t messageCode)
{
  recipient = to;
  code = messageCode;

  MSGMGR.queueMessage(this);
}

void BufferedNetworkMessage::broadcast (uint16_t messageCode, bool toAdminClients)
{
  recipient = NULL;
  code = messageCode;
  toAdmins = toAdminClients;
  MSGMGR.queueMessage(this);
}

void BufferedNetworkMessage::packUInt8(uint8_t val)
{
  checkData(sizeof(uint8_t));
  nboPackUInt8(getWriteBuffer(), val);
  packedSize += sizeof(uint8_t);
}

void BufferedNetworkMessage::packInt16(int16_t val)
{
  checkData(sizeof(int16_t));
  nboPackInt16(getWriteBuffer(), val);
  packedSize += sizeof(int16_t);
}

void BufferedNetworkMessage::packInt32(int32_t val)
{
  checkData(sizeof(int32_t));
  nboPackInt32(getWriteBuffer(), val);
  packedSize += sizeof(int32_t);
}

void BufferedNetworkMessage::packInt64(int64_t val)
{
  checkData(sizeof(int64_t));
  nboPackInt64(getWriteBuffer(), val);
  packedSize += sizeof(int64_t);
}

void BufferedNetworkMessage::packUInt16(uint16_t val)
{
  checkData(sizeof(uint16_t));
  nboPackUInt16(getWriteBuffer(), val);
  packedSize += sizeof(uint16_t);
}

void BufferedNetworkMessage::packUInt32(uint32_t val)
{
  checkData(sizeof(uint32_t));
  nboPackUInt32(getWriteBuffer(), val);
  packedSize += sizeof(uint32_t);
}

void BufferedNetworkMessage::packUInt64(uint64_t val)
{
  checkData(sizeof(uint64_t));
  nboPackUInt64(getWriteBuffer(), val);
  packedSize += sizeof(uint64_t);
}

void BufferedNetworkMessage::packFloat(float val)
{
  checkData(sizeof(float));
  nboPackFloat(getWriteBuffer(), val);
  packedSize += sizeof(float);
}

void BufferedNetworkMessage::packDouble(double val)
{
  checkData(sizeof(double));
  nboPackDouble(getWriteBuffer(), val);
  packedSize += sizeof(double);
}

void BufferedNetworkMessage::packFVec2(const fvec2& val)
{
  checkData(sizeof(fvec2));
  nboPackFVec2(getWriteBuffer(), val);
  packedSize += sizeof(fvec2);
}

void BufferedNetworkMessage::packFVec3(const fvec3& val)
{
  checkData(sizeof(fvec3));
  nboPackFVec3(getWriteBuffer(), val);
  packedSize += sizeof(fvec3);
}

void BufferedNetworkMessage::packFVec4(const fvec4& val)
{
  checkData(sizeof(fvec4));
  nboPackFVec4(getWriteBuffer(), val);
  packedSize += sizeof(fvec4);
}

void BufferedNetworkMessage::packString(const char* val, int len)
{
  checkData(len);
  nboPackString(getWriteBuffer(), val, len);
  packedSize += len;
}

void BufferedNetworkMessage::packStdString(const std::string& str)
{
  checkData(str.size()+sizeof(uint32_t));
  nboPackStdString(getWriteBuffer(), str);
  packedSize += str.size()+sizeof(uint32_t);
}

uint8_t BufferedNetworkMessage::unpackUInt8(void)
{
  uint8_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUInt8(p, v)) - p;
  return v;
}

int16_t BufferedNetworkMessage::unpackInt16(void)
{
  int16_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackInt16(p, v)) - p;
  return v;
}

int32_t BufferedNetworkMessage::unpackInt32(void)
{
  int32_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackInt32(p, v)) - p;
  return v;
}

int64_t BufferedNetworkMessage::unpackInt64(void)
{
  int64_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackInt64(p, v)) - p;
  return v;
}

uint16_t BufferedNetworkMessage::unpackUInt16(void)
{
  uint16_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUInt16(p, v)) - p;
  return v;
}

uint32_t BufferedNetworkMessage::unpackUInt32(void)
{
  uint32_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUInt32(p, v)) - p;
  return v;
}

uint64_t BufferedNetworkMessage::unpackUInt64(void)
{
  uint64_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUInt64(p, v)) - p;
  return v;
}

float BufferedNetworkMessage::unpackFloat(void)
{
  float v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackFloat(p, v)) - p;
  return v;
}

double BufferedNetworkMessage::unpackDouble(void)
{
  double v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackDouble(p, v)) - p;
  return v;
}

fvec2& BufferedNetworkMessage::unpackFVec2(fvec2& val)
{
  memset(val, 0, sizeof(fvec2));
  char *p = getReadBuffer();
  if (p) {
    packedSize += (char*)(nboUnpackFVec2(p, val)) - p;
  }
  return val;
}

fvec3& BufferedNetworkMessage::unpackFVec3(fvec3& val)
{
  memset(val, 0, sizeof(fvec3));
  char *p = getReadBuffer();
  if (p) {
    packedSize += (char*)(nboUnpackFVec3(p, val)) - p;
  }
  return val;
}

fvec4& BufferedNetworkMessage::unpackFVec4(fvec4& val)
{
  memset(val, 0, sizeof(fvec4));
  char *p = getReadBuffer();
  if (p) {
    packedSize += (char*)(nboUnpackFVec4(p, val)) - p;
  }
  return val;
}

const std::string& BufferedNetworkMessage::unpackStdString(std::string& str)
{
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackStdString(p, str)) - p;
  return str;
}

void BufferedNetworkMessage::addPackedData (const char* d, size_t s)
{
  checkData(s);
  memcpy(getWriteBuffer(), d, s);
  packedSize += s;
}

void BufferedNetworkMessage::clear (void)
{
  if (data)
    free(data);

  dataSize = 4;
  data = NULL;
  packedSize = 0;
  recipient = NULL;
  readPoint = 0;
  code = 0;
}

void BufferedNetworkMessage::reset (void)
{
  readPoint = 0;
}

size_t BufferedNetworkMessage::size (void)
{
  return packedSize;
}

bool BufferedNetworkMessage::process (void)
{
  if (!data)
    checkData(0);

  NetworkMessageTransferCallback *transferCallback = MSGMGR.getTransferCallback();

  if (!transferCallback || (!recipient && code == 0))
    return false;

  nboPackUInt16(data, uint16_t(packedSize));
  nboPackUInt16(data+sizeof(uint16_t), code);

  if (recipient) {
    return transferCallback->send(recipient, data, packedSize+4) == packedSize+4;
  }

  // send message to everyone
  int mask = NetHandler::clientBZFlag;
  if (toAdmins)
    mask |= NetHandler::clientBZAdmin;
  transferCallback->broadcast(data, packedSize+4, mask, code);

  return true;
}

void BufferedNetworkMessage::checkData (size_t s)
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

char* BufferedNetworkMessage::getWriteBuffer (void)
{
  if (!data)
    return NULL;

  return data + 4 + packedSize;
}

char* BufferedNetworkMessage::getReadBuffer (void)
{
  if (!data)
    return NULL;

  if (readPoint >= packedSize)
    return NULL;

  return data + 4 + readPoint;
}



//BufferedNetworkMessageManager
size_t BufferedNetworkMessageManager::receiveMessages (NetworkMessageTransferCallback *callback,  std::list<BufferedNetworkMessage*> &incomingMessages)
{
  BufferedNetworkMessage * msg = new BufferedNetworkMessage;

  while (callback->receive(msg)) {
    incomingMessages.push_back(msg);
    msg = new BufferedNetworkMessage;
  }
  // clean up the extra at the end
  delete msg;

  return incomingMessages.size();
}

void BufferedNetworkMessageManager::update (void)
{
  if (transferCallback)
    sendPendingMessages();
}

void BufferedNetworkMessageManager::sendPendingMessages (void)
{
  MessageDeque leftovers;

  std::map<NetHandler*, int> sentHandlers;

  while (outgoingQueue.size()) {
    MessageDeque::iterator itr = outgoingQueue.begin();
    if (itr != outgoingQueue.end()) {
      BufferedNetworkMessage *msg = *itr;

      if (msg) {
	bool send = true;

	if (sentHandlers.find(msg->recipient) != sentHandlers.end()) {
	  if (sentHandlers[msg->recipient] > 2) // don't spam em
	    send = false;
	}

	if (send && msg->process()) {
	  if (msg->recipient) {
	    if (sentHandlers.find(msg->recipient) == sentHandlers.end())
	      sentHandlers[msg->recipient] = 1;
	    else
	      sentHandlers[msg->recipient]++;
	  }
	  delete(msg);
	} else {
	  leftovers.push_back(msg);
	}

	outgoingQueue.pop_front();
      }
    }
  }
  outgoingQueue = leftovers; // do the leftover next time
}

void BufferedNetworkMessageManager::queueMessage (BufferedNetworkMessage *msg)
{
  MessageList::iterator itr = std::find(pendingOutgoingMessages.begin(), pendingOutgoingMessages.end(), msg);
  if (itr != pendingOutgoingMessages.end()) {
    pendingOutgoingMessages.erase(itr);
  }

#ifdef DEBUG
  if (msg->code != MsgPlayerUpdateSmall) {
    const MsgStringList msgList = MsgStrings::msgFromClient(msg->packedSize,
      msg->code, msg->data);
    logDebugMessage(5, "%s to %s at %f\n", msgList[0].text.c_str(),
      msg->recipient ? msg->recipient->getTargetIP() : "all",
      BzTime::getCurrent().getSeconds());
  }
#endif
  outgoingQueue.push_back(msg);
}

void BufferedNetworkMessageManager::purgeMessages (NetHandler *handler)
{
  MessageList::iterator itr = pendingOutgoingMessages.begin();
  while (itr != pendingOutgoingMessages.end()) {
    if ((*itr) && (handler == (*itr)->recipient)) {
      // just kill the message and data, it'll be pulled from the list on the next update pass
      delete(*itr);
      pendingOutgoingMessages.erase(itr);
      itr = pendingOutgoingMessages.begin();
    } else {
      itr++;
    }
  }

  MessageDeque::iterator qItr = outgoingQueue.begin();
  while (qItr != outgoingQueue.end()) {
    if ((*qItr) && (handler == (*qItr)->recipient)) {
      // just kill the message and data, it'll be pulled from the list on the next update pass
      delete(*qItr);
      outgoingQueue.erase(qItr);
      qItr = outgoingQueue.begin();
    } else {
      qItr++;
    }
  }
}

void BufferedNetworkMessageManager::flushMessages (NetHandler *handler)
{
  MessageDeque::iterator qItr = outgoingQueue.begin();
  while (qItr != outgoingQueue.end()) {
    if ((*qItr) && (handler == (*qItr)->recipient)) {
      // process the message and data for that specific nethandler
      (*qItr)->process();
      delete(*qItr);
      outgoingQueue.erase(qItr);
      qItr = outgoingQueue.begin();
    } else {
      qItr++;
    }
  }
}

BufferedNetworkMessageManager::BufferedNetworkMessageManager()
{
  transferCallback = NULL;
}

BufferedNetworkMessageManager::~BufferedNetworkMessageManager()
{
  MessageList::iterator itr = pendingOutgoingMessages.begin();
  while (itr != pendingOutgoingMessages.end()) {
    delete(*itr);
    itr++;
  }

  MessageDeque::iterator qItr = outgoingQueue.begin();
  while (qItr != outgoingQueue.end()) {
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
