/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Functions to write various data types packed into a buffer
 * in network byte order, or to read various types out of a
 * buffer.
 *
 * floats may be converted to some platform independent form before
 * packing and converted from that form when unpacking;  precision
 * may be lost.
 */

#ifndef BZF_PACK_H
#define BZF_PACK_H

#include "common.h"

inline void*			nboPackUByte(void* buf, uint8_t u8)
{
	*((uint8_t *)buf) = u8;
	return ((uint8_t *)buf) + 1;
}

void*					nboPackShort(void*, int16_t);
void*					nboPackInt(void*, int32_t);
void*					nboPackUShort(void*, uint16_t);
void*					nboPackUInt(void*, uint32_t);
void*					nboPackFloat(void*, float);
void*					nboPackVector(void*, const float*);
void*					nboPackString(void*, const void*, int len);

inline void*			nboUnpackUByte(void* buf, uint8_t& u8)
{
	u8 = *((uint8_t *)buf);
	return ((uint8_t *)buf) + 1;
}

void*					nboUnpackShort(void*, int16_t&);
void*					nboUnpackInt(void*, int32_t&);
void*					nboUnpackUShort(void*, uint16_t&);
void*					nboUnpackUInt(void*, uint32_t&);
void*					nboUnpackFloat(void*, float&);
void*					nboUnpackVector(void*, float*);
void*					nboUnpackString(void*, void*, int len);

#endif // BZF_PACK_H
