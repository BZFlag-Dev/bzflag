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

#include "ErrorHandler.h"
#include <stdio.h>
#include <stdarg.h>
#include "bzfio.h"
#if defined(_WIN32)
#include <windows.h>
#endif
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
  else fprintf(stderr, "%s\n", msg.c_str());
#endif
}
// ex: shiftwidth=2 tabstop=8
