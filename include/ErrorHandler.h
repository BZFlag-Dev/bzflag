/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * error handling functions
 */

#ifndef BZF_ERROR_HANDLER_H
#define	BZF_ERROR_HANDLER_H

#ifdef _WIN32
#pragma warning( 4: 4786 )
#endif

#include <vector>
#include <string>
#include "Bundle.h"

typedef void		(*ErrorCallback)(const char*);

ErrorCallback		setErrorCallback(ErrorCallback);
void			printError(const std::string &fmt, const std::vector<std::string> *parms = NULL);

#endif // BZF_ERROR_HANDLER_H
// ex: shiftwidth=2 tabstop=8
