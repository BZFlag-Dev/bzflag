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

/*
 * dummy error handling function for bot dll
 */

#ifndef __BZF_PRINT_ERROR_H__
#define __BZF_PRINT_ERROR_H__

// common header first
#include "common.h"

// system headers
#include <vector>
#include <string>

// local implementation headers
#include "common.h"

void printError(const std::string& fmt, const std::vector<std::string> *parms = NULL);

#endif // __BZF_PRINT_ERROR_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
