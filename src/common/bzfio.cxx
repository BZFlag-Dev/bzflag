/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _WIN32
#include "ATLbase.h"
#endif

#include "bzfio.h"
#include "common.h"

void formatDebug(const char* fmt, ...)
{
  if (debugLevel >= 1) {
    char buffer[8192];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 8192, fmt, args);
    va_end(args);
    #if defined(_WIN32)
      #if defined(_DEBUG)
        ATLTRACE2(atlTraceUser, ATL_TRACE_LEVEL, buffer);
      #endif
    #else
      std::cout << buffer;
    #endif
  }
}
