/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 * error handling functions
 */

#ifndef BZF_ERROR_HANDLER_H
#define	BZF_ERROR_HANDLER_H

// common header first
#include "common.h"

// system headers
#include <vector>
#include <string>

// local implementation headers
#include "common.h"
#include "Bundle.h"

typedef void		(*ErrorCallback)(const char*);

ErrorCallback		setErrorCallback(ErrorCallback);
void			printError(const std::string &fmt, const std::vector<std::string> *parms = NULL);
void			printFatalError(const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 1, 2)));

#endif // BZF_ERROR_HANDLER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
