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

#include "ErrorHandler.h"
#include <stdio.h>
#include <stdarg.h>
#include "bzfio.h"
#if defined(_WIN32)
#include <windows.h>
#endif

static ErrorCallback	errorCallback = NULL;

ErrorCallback		setErrorCallback(ErrorCallback cb)
{
  ErrorCallback oldErrorCallback = errorCallback;
  errorCallback = cb;
  return oldErrorCallback;
}

void			printError(const char* fmt, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  if (errorCallback) (*errorCallback)(buffer);
#if defined(_WIN32)
  else { OutputDebugString(buffer); OutputDebugString("\n"); }
#else
  else fprintf(stderr, "%s\n", buffer);
#endif
}
// ex: shiftwidth=2 tabstop=8
