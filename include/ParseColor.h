/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _PARSE_COLOR_H_
#define _PARSE_COLOR_H_


#include "common.h"

// system headers
#include <string>
#include <iostream>

// common headers
#include "vectors.h"


extern bool parseColorCString(const char* str,       fvec4& color);
extern bool parseColorString(const std::string& str, fvec4& color);
extern bool parseColorStream(std::istream& input,    fvec4& color);


#endif // _PARSE_COLOR_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
