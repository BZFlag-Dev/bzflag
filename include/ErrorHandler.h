/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
#define BZF_ERROR_HANDLER_H

typedef void			(*ErrorCallback)(const char*);

ErrorCallback			setErrorCallback(ErrorCallback);
void					printError(const char* fmt, ...);

#endif // BZF_ERROR_HANDLER_H
