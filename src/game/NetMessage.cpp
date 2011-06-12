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

// interface header
#include "NetMessage.h"

// system headers
#include <assert.h>
#include <string.h>

// common headers
#include "common/bzfio.h"
#include "net/Pack.h"


NetMessage::SendFunc      NetMessage::sendFunc      = NULL;
NetMessage::BroadcastFunc NetMessage::broadcastFunc = NULL;


const size_t NetMessage::lenCodeOffset = 2 * sizeof(uint16_t);
const size_t NetMessage::capacityStep  = 256;
const size_t NetMessage::capacityDefault = NetMessage::capacityStep;


//============================================================================//

uint16_t NetRecvMsg::getCode() const {
  uint16_t code;
  nboUnpackUInt16((uint16_t*)data + 1, code);
  return code;
}


//============================================================================//

NetMessage::NetMessage()
  : data(NULL) {
  clear(); // setup the initial capacity
}


NetMessage::NetMessage(size_t size) {
  capacity = size + lenCodeOffset;;
  dataSize = capacity;
  data = (char*) malloc(dataSize);
  memset(data, 0, lenCodeOffset);
  readIndex = lenCodeOffset;
}


NetMessage::NetMessage(const void* fullData, size_t fullSize) {
  assert(fullSize >= lenCodeOffset);
  assert(*((uint16_t*)fullData) == (fullSize - lenCodeOffset));

  dataSize = fullSize;
  capacity = fullSize;
  data = (char*) malloc(dataSize);
  memcpy(data, fullData, dataSize);
  readIndex = lenCodeOffset;
}


NetMessage::NetMessage(uint16_t len, uint16_t code, const void* msgData) {
  dataSize = lenCodeOffset + len;
  capacity = dataSize;
  data = (char*) malloc(dataSize);

  uint16_t* u16ptr = (uint16_t*) data;
  nboPackUInt16(u16ptr + 0, len);
  nboPackUInt16(u16ptr + 1, code);
  memcpy(data + lenCodeOffset, msgData, len);

  readIndex = lenCodeOffset;
}


NetMessage::NetMessage(const NetMessage& msg)
  : dataSize(msg.dataSize)
  , capacity(msg.capacity)
  , readIndex(msg.readIndex) {
  data = (char*) malloc(dataSize);
  memcpy(data, msg.data, dataSize);
}


NetMessage& NetMessage::operator=(const NetMessage& msg) {
  free(data);

  dataSize  = msg.dataSize;
  capacity  = msg.capacity;
  readIndex = msg.readIndex;

  data = (char*) malloc(dataSize);
  memcpy(data, msg.data, dataSize);

  return *this;
}


NetMessage::~NetMessage() {
  free(data);
}


//============================================================================//

void NetMessage::clear() {
  free(data);
  capacity = capacityDefault;
  data = (char*) malloc(capacity);
  memset(data, 0, lenCodeOffset);
  dataSize  = lenCodeOffset;
  readIndex = lenCodeOffset;
}


void NetMessage::growCapacity(size_t s) {
  if (s < capacity) {
    return;
  }

  size_t newCapacity = capacity;
  do {
    newCapacity += capacityStep;
  }
  while (newCapacity < s);

  capacity = newCapacity;

  data = (char*) realloc(data, capacity);
  if (data == NULL) {
    debugf(0,
           "ERROR: NetMessage::growCapacity(%i) out-of-memory\n", (int) s);
    exit(2);
  }
}


void NetMessage::checkData(size_t s) {
  growCapacity(capacity + s);
}


inline char* NetMessage::getWriteBuffer() {
  return data + dataSize;
}


inline char* NetMessage::checkReadBuffer(size_t s) {
  if ((readIndex + s) > dataSize) {
    return NULL;
  }
  return data + readIndex;
}


//============================================================================//

void NetMessage::send(NetHandler* dst, uint16_t messageCode) {
  if (!sendFunc) {
    debugf(0, "NetMessage::send()  sendFunc == NULL\n");
  }

  if (!dst) {
    return;
  }

  // setup the length & code
  nboPackUInt16(data, uint16_t(dataSize - lenCodeOffset));
  nboPackUInt16(data + sizeof(uint16_t), messageCode);

  sendFunc(dst, data, dataSize);
}


void NetMessage::broadcast(uint16_t messageCode, bool toTextClients) {
  if (!broadcastFunc) {
    debugf(0, "NetMessage::broadcast()  broadcastFunc == NULL\n");
  }

  // setup the length & code
  nboPackUInt16(data, uint16_t(dataSize - lenCodeOffset));
  nboPackUInt16(data + sizeof(uint16_t), messageCode);

  broadcastFunc(data, dataSize, toTextClients);
}


//============================================================================//
//
//  Packing routines
//

void NetMessage::packInt8(int8_t val) {
  checkData(sizeof(int8_t));
  nboPackInt8(getWriteBuffer(), val);
  dataSize += sizeof(int8_t);
}


void NetMessage::packInt16(int16_t val) {
  checkData(sizeof(int16_t));
  nboPackInt16(getWriteBuffer(), val);
  dataSize += sizeof(int16_t);
}


void NetMessage::packInt32(int32_t val) {
  checkData(sizeof(int32_t));
  nboPackInt32(getWriteBuffer(), val);
  dataSize += sizeof(int32_t);
}


void NetMessage::packInt64(int64_t val) {
  checkData(sizeof(int64_t));
  nboPackInt64(getWriteBuffer(), val);
  dataSize += sizeof(int64_t);
}


void NetMessage::packUInt8(uint8_t val) {
  checkData(sizeof(uint8_t));
  nboPackUInt8(getWriteBuffer(), val);
  dataSize += sizeof(uint8_t);
}


void NetMessage::packUInt16(uint16_t val) {
  checkData(sizeof(uint16_t));
  nboPackUInt16(getWriteBuffer(), val);
  dataSize += sizeof(uint16_t);
}


void NetMessage::packUInt32(uint32_t val) {
  checkData(sizeof(uint32_t));
  nboPackUInt32(getWriteBuffer(), val);
  dataSize += sizeof(uint32_t);
}


void NetMessage::packUInt64(uint64_t val) {
  checkData(sizeof(uint64_t));
  nboPackUInt64(getWriteBuffer(), val);
  dataSize += sizeof(uint64_t);
}


void NetMessage::packFloat(float val) {
  checkData(sizeof(float));
  nboPackFloat(getWriteBuffer(), val);
  dataSize += sizeof(float);
}


void NetMessage::packDouble(double val) {
  checkData(sizeof(double));
  nboPackDouble(getWriteBuffer(), val);
  dataSize += sizeof(double);
}


void NetMessage::packFVec2(const fvec2& val) {
  checkData(sizeof(fvec2));
  nboPackFVec2(getWriteBuffer(), val);
  dataSize += sizeof(fvec2);
}


void NetMessage::packFVec3(const fvec3& val) {
  checkData(sizeof(fvec3));
  nboPackFVec3(getWriteBuffer(), val);
  dataSize += sizeof(fvec3);
}


void NetMessage::packFVec4(const fvec4& val) {
  checkData(sizeof(fvec4));
  nboPackFVec4(getWriteBuffer(), val);
  dataSize += sizeof(fvec4);
}


void NetMessage::packString(const char* val, int len) {
  checkData(len);
  nboPackString(getWriteBuffer(), val, len);
  dataSize += len;
}


void NetMessage::packStdString(const std::string& str) {
  const size_t packSize = nboStdStringPackSize(str);
  checkData(packSize);
  nboPackStdString(getWriteBuffer(), str);
  dataSize += packSize;
}


//============================================================================//
//
//  Unpacking routines
//

int8_t NetMessage::unpackInt8() {
  int8_t v = 0;
  char* p = checkReadBuffer(sizeof(int8_t));
  if (p) {
    nboUnpackInt8(p, v);
    readIndex += sizeof(int8_t);
  }
  return v;
}


int16_t NetMessage::unpackInt16() {
  int16_t v = 0;
  char* p = checkReadBuffer(sizeof(int16_t));
  if (p) {
    nboUnpackInt16(p, v);
    readIndex += sizeof(int16_t);
  }
  return v;
}


int32_t NetMessage::unpackInt32() {
  int32_t v = 0;
  char* p = checkReadBuffer(sizeof(int32_t));
  if (p) {
    nboUnpackInt32(p, v);
    readIndex += sizeof(int32_t);
  }
  return v;
}


int64_t NetMessage::unpackInt64() {
  int64_t v = 0;
  char* p = checkReadBuffer(sizeof(int64_t));
  if (p) {
    nboUnpackInt64(p, v);
    readIndex += sizeof(int64_t);
  }
  return v;
}


uint8_t NetMessage::unpackUInt8() {
  uint8_t v = 0;
  char* p = checkReadBuffer(sizeof(uint8_t));
  if (p) {
    nboUnpackUInt8(p, v);
    readIndex += sizeof(uint8_t);
  }
  return v;
}


uint16_t NetMessage::unpackUInt16() {
  uint16_t v = 0;
  char* p = checkReadBuffer(sizeof(uint16_t));
  if (p) {
    nboUnpackUInt16(p, v);
    readIndex += sizeof(uint16_t);
  }
  return v;
}


uint32_t NetMessage::unpackUInt32() {
  uint32_t v = 0;
  char* p = checkReadBuffer(sizeof(uint32_t));
  if (p) {
    nboUnpackUInt32(p, v);
    readIndex += sizeof(uint32_t);
  }
  return v;
}


uint64_t NetMessage::unpackUInt64() {
  uint64_t v = 0;
  char* p = checkReadBuffer(sizeof(uint64_t));
  if (p) {
    nboUnpackUInt64(p, v);
    readIndex += sizeof(uint64_t);
  }
  return v;
}


float NetMessage::unpackFloat() {
  float v = 0.0f;
  char* p = checkReadBuffer(sizeof(float));
  if (p) {
    nboUnpackFloat(p, v);
    readIndex += sizeof(float);
  }
  return v;
}


double NetMessage::unpackDouble() {
  double v = 0.0;
  char* p = checkReadBuffer(sizeof(double));
  if (p) {
    nboUnpackDouble(p, v);
    readIndex += sizeof(double);
  }
  return v;
}


fvec2 NetMessage::unpackFVec2() {
  fvec2 v(0.0f, 0.0f);
  char* p = checkReadBuffer(sizeof(fvec2));
  if (p) {
    nboUnpackFVec2(p, v);
    readIndex += sizeof(fvec2);
  }
  return v;
}


fvec3 NetMessage::unpackFVec3() {
  fvec3 v(0.0f, 0.0f, 0.0f);
  char* p = checkReadBuffer(sizeof(fvec3));
  if (p) {
    nboUnpackFVec3(p, v);
    readIndex += sizeof(fvec3);
  }
  return v;
}


fvec4 NetMessage::unpackFVec4() {
  fvec4 v(0.0f, 0.0f, 0.0f, 0.0f);
  char* p = checkReadBuffer(sizeof(fvec4));
  if (p) {
    nboUnpackFVec4(p, v);
    readIndex += sizeof(fvec4);
  }
  return v;
}


std::string NetMessage::unpackStdString() {
  std::string v;
  const uint32_t strSize = unpackUInt32();
  char* p = checkReadBuffer(strSize);
  if (p) {
    char* buffer = new char[strSize + 1];
    nboUnpackString(p, buffer, strSize);
    buffer[strSize] = 0;
    v = buffer;
    delete[] buffer;
    readIndex += strSize;
  }
  return v;
}


std::string NetMessage::unpackStdStringRaw() {
  std::string v;
  const uint32_t strSize = unpackUInt32();
  char* p = checkReadBuffer(strSize);
  if (p) {
    char* buffer = new char[strSize];
    nboUnpackString(p, buffer, strSize);
    v.assign(buffer, strSize);
    delete[] buffer;
    readIndex += strSize;
  }
  return v;
}


void NetMessage::unpackString(char* buf, size_t len) {
  char* p = checkReadBuffer(len);
  if (p) {
    memcpy(buf, p, len);
    readIndex += len;
  }
  else {
    memset(buf, 0, len);
  }
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
