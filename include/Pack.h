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
#include "vectors.h"


extern void* nboPackInt8      (void*, int8_t);
extern void* nboPackInt16     (void*, int16_t);
extern void* nboPackInt32     (void*, int32_t);
extern void* nboPackInt64     (void*, int64_t);
extern void* nboPackUInt8     (void*, uint8_t);
extern void* nboPackUInt16    (void*, uint16_t);
extern void* nboPackUInt32    (void*, uint32_t);
extern void* nboPackUInt64    (void*, uint64_t);
extern void* nboPackFloat     (void*, float);
extern void* nboPackDouble    (void*, double);
extern void* nboPackFVec2     (void*, const fvec2&);
extern void* nboPackFVec3     (void*, const fvec3&);
extern void* nboPackFVec4     (void*, const fvec4&);
extern void* nboPackString    (void*, const void*, int len);
extern void* nboPackStdString (void*, const std::string& str);

extern void* nboUnpackInt8         (void*, int8_t&);
extern void* nboUnpackInt16        (void*, int16_t&);
extern void* nboUnpackInt32        (void*, int32_t&);
extern void* nboUnpackInt64        (void*, int64_t&);
extern void* nboUnpackUInt8        (void*, uint8_t&);
extern void* nboUnpackUInt16       (void*, uint16_t&);
extern void* nboUnpackUInt32       (void*, uint32_t&);
extern void* nboUnpackUInt64       (void*, uint64_t&);
extern void* nboUnpackFloat        (void*, float&);
extern void* nboUnpackDouble       (void*, double&);
extern void* nboUnpackFVec2        (void*, fvec2&);
extern void* nboUnpackFVec3        (void*, fvec3&);
extern void* nboUnpackFVec4        (void*, fvec4&);
extern void* nboUnpackString       (void*, void*, int len);
extern void* nboUnpackStdString    (void*, std::string& str);
extern void* nboUnpackStdStringRaw (void*, std::string& str);

extern unsigned int nboStdStringPackSize(const std::string& str);

// buffer overrun checking
extern void nboUseErrorChecking(bool checking);
extern bool nboGetBufferError();
extern void nboClearBufferError();
extern void nboSetBufferLength(unsigned int length);
extern unsigned int nboGetBufferLength();


#endif // BZF_PACK_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
