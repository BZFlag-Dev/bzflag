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

#include "common.h"

// interface header
#include "Clipboard.h"

// system headers
#include <string>


#if defined(_WIN32)

std::string getClipboard()
{
  std::string data;
  OpenClipboard(NULL);
  const void* p = GetClipboardData(CF_TEXT);
  if (p != NULL) {
    data = (char*)p;
  }
  CloseClipboard();
  return data;
}

#elif defined(__APPLE__)

std::string getClipboard()
{
  return ""; // FIXME -- not implemented
}

#elif defined(HAVE_SDL)

#include "bzfSDL.h"
std::string getClipboard()
{
  std::string data;
  // only works with the cut-buffer method (xterm)
  // (and not with the more recent selections method)
  SDL_SysWMinfo sdlinfo;
  SDL_VERSION(&sdlinfo.version);
  if (SDL_GetWMInfo(&sdlinfo)) {
    sdlinfo.info.x11.lock_func();
    Display* display = sdlinfo.info.x11.display;
    int count = 0;
    char* msg = XFetchBytes(display, &count);
    if ((msg != NULL) && (count > 0)) {
      data.append((char*)msg, count);
    }
    XFree(msg);
    sdlinfo.info.x11.unlock_func();
  }
  return data;
}

#else

std::string getClipboard()
{
  return ""; // FIXME -- not implemented
}

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
