/* bzflag
* Copyright (c) 1993-2018 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#pragma once

#include "common.h"
#include <string>
#include <memory>

class MessageBuffer
{
public:
    typedef std::shared_ptr<MessageBuffer> Ptr;

    MessageBuffer();
    ~MessageBuffer();

    void packUByte(uint8_t);
    void packShort(int16_t);
    void packInt(int32_t);
    void packUShort(uint16_t);
    void packUInt(uint32_t);
    void packFloat(float);
    void packVector(const float*);
    void packString(const void* val, int len);
    void packStdString(const std::string& str);

    void reset();

    size_t size();
    void* buffer();

    bool locked = false;

protected:
    unsigned char*   raw_buffer = nullptr;
    void*   write_ptr;
};

MessageBuffer::Ptr GetMessageBuffer();
void ReleaseMessageBuffer(MessageBuffer::Ptr buffer);


