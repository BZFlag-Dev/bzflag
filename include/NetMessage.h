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

#ifndef __NET_MESSAGE_H__
#define __NET_MESSAGE_H__

#include "common.h"

// System headers
#include <list>
#include <deque>
#include <string>

// Common headers
#include "Singleton.h"
#include "vectors.h"

class NetHandler;


class NetMessage {
  public:
    typedef void (*SendFunc)      (NetHandler*, const void* data, size_t size);
    typedef void (*RecvFunc)      (NetMessage* netMsg);
    typedef void (*BroadcastFunc) (const void* data, size_t size, bool textClients);

  public:
    static void setSendFunc(SendFunc func)    { sendFunc = func; }
    static void setReceiveFunc(RecvFunc func) { recvFunc = func; }
    static void setBroadcastFunc(BroadcastFunc func) { broadcastFunc = func; }

  protected:
    static SendFunc sendFunc;
    static RecvFunc recvFunc;
    static BroadcastFunc broadcastFunc;

    static const size_t lenCodeOffset;
    static const size_t capacityStep;
    static const size_t capacityDefault;

  public:
    NetMessage();
    NetMessage(const NetMessage& msg);
    NetMessage& operator=(const NetMessage& msg);
    ~NetMessage();
  private:
    NetMessage(size_t size); // start with capacity = (size + lenCodeOffset)
    NetMessage(const void* fullData, size_t fullSize); // FIXME
  
  public:
    void clear();
    void resetRead() { readIndex = 0; }

    inline char*        getData()       { return data + lenCodeOffset; }
    inline const char*  getData() const { return data + lenCodeOffset; }

    inline size_t getSize()     const { return dataSize - lenCodeOffset; }
    inline size_t getFullSize() const { return dataSize; }

    inline size_t getCapacity() const { return capacity; }

  public: // sending
    void send(NetHandler* dst, uint16_t messageCode);
    void broadcast(uint16_t messageCode, bool toTextClients = true);

  public: // packing
    void packInt8(int8_t);
    void packInt16(int16_t);
    void packInt32(int32_t);
    void packInt64(int64_t);
    void packUInt8(uint8_t);
    void packUInt16(uint16_t);
    void packUInt32(uint32_t);
    void packUInt64(uint64_t);
    void packFloat(float);
    void packDouble(double);
    void packFVec2(const fvec2&);
    void packFVec3(const fvec3&);
    void packFVec4(const fvec4&);
    void packString(const char* data, int len);
    void packStdString(const std::string& str);

  public: // unpacking
    int8_t   unpackInt8();
    int16_t  unpackInt16();
    int32_t  unpackInt32();
    int64_t  unpackInt64();
    uint8_t  unpackUInt8();
    uint16_t unpackUInt16();
    uint32_t unpackUInt32();
    uint64_t unpackUInt64();
    float    unpackFloat();
    double   unpackDouble();
    fvec2    unpackFVec2();
    fvec3    unpackFVec3();
    fvec4    unpackFVec4();
    std::string unpackStdString();
    std::string unpackStdStringRaw();
    void unpackString(char* buf, size_t len);

  protected:
    void growCapacity(size_t s);
    void checkData(size_t s);

    char* getWriteBuffer();
    char* checkReadBuffer(size_t s);

  protected:
    char*  data;
    size_t dataSize;  // includes the 4 bytes for len+code
    size_t capacity;  // includes the 4 bytes for len+code
    size_t readIndex; // includes the 4 bytes for len+code
};


#endif //__NET_MESSAGE_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
