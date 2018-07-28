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

#include "MessageBuffers.h"
#include "Protocol.h"
#include "Pack.h"

#include <string.h>
#include <vector>

MessageBuffer::MessageBuffer()
{
    raw_buffer = new unsigned char[MaxPacketLen];
    reset();
}

MessageBuffer::~MessageBuffer()
{
    if (raw_buffer != nullptr)
        delete[] raw_buffer;
}

void MessageBuffer::reset()
{
    write_ptr = &(raw_buffer[2 * sizeof(uint16_t)]);
}

size_t MessageBuffer::size()
{
    return (size_t)(((unsigned char*)write_ptr - raw_buffer) - (2 * sizeof(uint16_t)));
}

void* MessageBuffer::buffer()
{
    return raw_buffer;
}

void* MessageBuffer::current_buffer()
{
    return write_ptr;
}

void* MessageBuffer::record_buffer()
{
    return (char*)raw_buffer + 4;;
}


void MessageBuffer::push_repack(size_t offset)
{
    saved_ptr = write_ptr;
    write_ptr = ((unsigned char*)raw_buffer) + offset + 4;
}

void MessageBuffer::pop_offset()
{
    write_ptr = saved_ptr;
    saved_ptr = nullptr;
}


void MessageBuffer::legacyPack(void* newEndPtr)
{
    write_ptr = newEndPtr;
}

void MessageBuffer::packBuffer(const void* data, size_t len)
{
    memcpy(write_ptr, data, len);
    write_ptr = (char*)write_ptr + len;
}

void MessageBuffer::packUByte(uint8_t val)
{
    write_ptr = nboPackUByte(write_ptr, val);
}

void MessageBuffer::packShort(int16_t val)
{
    write_ptr = nboPackShort(write_ptr, val);
}

void MessageBuffer::packInt(int32_t val)
{
    write_ptr = nboPackInt(write_ptr, val);
}

void MessageBuffer::packUShort(uint16_t val)
{
    write_ptr = nboPackUShort(write_ptr, val);
}

void MessageBuffer::packUInt(uint32_t val)
{
    write_ptr = nboPackUInt(write_ptr, val);
}

void MessageBuffer::packFloat(float val)
{
    write_ptr = nboPackFloat(write_ptr, val);
}

void MessageBuffer::packVector(const float* val)
{
    write_ptr = nboPackVector(write_ptr, val);
}

void MessageBuffer::packString(const void* val, int len)
{
    write_ptr = nboPackString(write_ptr, val, len);
}

void MessageBuffer::packStdString(const std::string& val)
{
    write_ptr = nboPackStdString(write_ptr, val);
}

std::vector<MessageBuffer::Ptr> BufferPool;

void moreBuffers()
{
    for (int i = 0; i < 5; i++)
        BufferPool.push_back(std::make_shared<MessageBuffer>());
}

MessageBuffer::Ptr checkOutBuffer()
{
    for (size_t i = 0; i < BufferPool.size(); i++)
    {
        if (!BufferPool[i]->locked)
        {
            BufferPool[i]->locked = true;
            return BufferPool[i];
        }
    }

    return nullptr;
}

MessageBuffer::Ptr GetMessageBuffer()
{
    auto buffer = checkOutBuffer();
    if (buffer == nullptr)
    {
        moreBuffers();
        buffer = checkOutBuffer();
    }
    return buffer;
}

void ReleaseMessageBuffer(MessageBuffer::Ptr buffer)
{
    if (buffer != nullptr)
    {
        buffer->reset();
        buffer->locked = false;
    }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
