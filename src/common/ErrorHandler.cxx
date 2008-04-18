/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// interface header
#include "ErrorHandler.h"

// system headers
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>

// local implementation headers
#include "bzfio.h"
#include "BundleMgr.h"
#include "Bundle.h"

static ErrorCallback	errorCallback = NULL;

ErrorCallback		setErrorCallback(ErrorCallback cb)
{
  ErrorCallback oldErrorCallback = errorCallback;
  errorCallback = cb;
  return oldErrorCallback;
}

void			printError(const std::string &fmt, const std::vector<std::string> *parms)
{
  std::string msg;
  Bundle *pBdl = BundleMgr::getCurrentBundle();
  if (!pBdl)
    return;

  if ((parms != NULL) && (parms->size() > 0))
    msg = pBdl->formatMessage(fmt, parms);
  else
    msg = pBdl->getLocalString(fmt);

  if (errorCallback) (*errorCallback)(msg.c_str());
#if defined(_WIN32)
  else { OutputDebugString(msg.c_str()); OutputDebugString("\n"); }
#else
  else std::cerr << msg << std::endl;
#endif
}

//
// special error handler.  shows a message box on Windows.
//

std::vector<FatalErrorCallback*> fatalCallbacks;

void addFatalErrorCallback ( FatalErrorCallback* callback )
{
  fatalCallbacks.push_back(callback);
}
void removeFatalErrorCallback ( FatalErrorCallback* callback )
{
  for ( size_t s = 0; s < fatalCallbacks.size(); s++ )
  {
    if ( callback == fatalCallbacks[s])
    {
      fatalCallbacks.erase(fatalCallbacks.begin()+s);
      return;
    }
  }
}

void			printFatalError(const char* fmt, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);

  for ( size_t s = 0; s < fatalCallbacks.size(); s++ )
  {
    if ( fatalCallbacks[s])
      fatalCallbacks[s]->error("BZFlag Error",buffer);
  }

  std::cerr << buffer << std::endl;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
