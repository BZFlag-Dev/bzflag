/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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

#ifndef	BZF_PACK_H
#define	BZF_PACK_H

#include "common.h"
#include <string>

void* nboPackUByte(void*, uint8_t);
void* nboPackShort(void*, int16_t);
void* nboPackInt(void*, int32_t);
void* nboPackUShort(void*, uint16_t);
void* nboPackUInt(void*, uint32_t);
void* nboPackFloat(void*, float);
void* nboPackVector(void*, const float*);
void* nboPackString(void*, const void*, int len);
void* nboPackStdString(void*, const std::string& str);

void* nboUnpackUByte(void*, uint8_t&);
void* nboUnpackShort(void*, int16_t&);
void* nboUnpackInt(void*, int32_t&);
void* nboUnpackUShort(void*, uint16_t&);
void* nboUnpackUInt(void*, uint32_t&);
void* nboUnpackFloat(void*, float&);
void* nboUnpackVector(void*, float*);
void* nboUnpackString(void*, void*, int len);
void* nboUnpackStdString(void*, std::string& str);

unsigned int nboStdStringPackSize(const std::string& str);

#endif // BZF_PACK_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
