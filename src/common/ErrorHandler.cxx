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

#include "ErrorHandler.h"
#include "BzfString.h"
#include <stdio.h>
#if defined(_WIN32)
#include <windows.h>
#endif

static ErrorCallback		errorCallback = NULL;

ErrorCallback			setErrorCallback(ErrorCallback cb)
{
	ErrorCallback oldErrorCallback = errorCallback;
	errorCallback = cb;
	return oldErrorCallback;
}

void					printError(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	BzfString buffer = BzfString::vformat(fmt, args);
	va_end(args);

	if (errorCallback) {
		(*errorCallback)(buffer.c_str());
	}
#if defined(_WIN32)
	else {
		OutputDebugString(buffer.c_str());
		OutputDebugString("\n");
	}
#else
	else {
		fprintf(stderr, "%s\n", buffer.c_str());
	}
#endif
}
