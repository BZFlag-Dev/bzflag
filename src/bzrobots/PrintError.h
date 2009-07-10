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

/*
 * dummy error handling function for bot dll
 */

#ifndef BZF_PRINT_ERROR_H
#define	BZF_PRINT_ERROR_H

// common header first
#include "common.h"

// system headers
#include <vector>
#include <string>

// local implementation headers
#include "common.h"

void			printError(const std::string &fmt, const std::vector<std::string> *parms = NULL);

#endif // BZF_PRINT_ERROR_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
