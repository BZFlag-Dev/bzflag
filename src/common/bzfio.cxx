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
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

static HANDLE consoleStdIn;
static HANDLE consoleStdOut;
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
    std::cout << buffer;
  }
}

void initDebug()
{
#if defined(_WIN32) && defined(_DEBUG)
  int conHandle;
  AllocConsole();
  consoleStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  conHandle = _open_osfhandle((long)consoleStdOut, _O_TEXT);
  FILE *fp = _fdopen( conHandle, "w" );
  *stdout = *fp;
  setvbuf( stdout, NULL, _IONBF, 0 );
  std::ios::sync_with_stdio();
#endif
}

void termDebug()
{
#if defined(_WIN32) && defined(_DEBUG)
  FreeConsole();
#endif
}
