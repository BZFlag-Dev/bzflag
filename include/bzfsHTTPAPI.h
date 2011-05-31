/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// a base class for plugins that want to do HTTP
#ifndef _BZFS_HTTP_H_
#define _PLUGIN_HTTP_H_

#include "bzfsAPI.h"

typedef enum
{
  eHTTPUnknown = 0,
  eHTTPGet,
  eHTTPHead,
  eHTTPPost,
  eHTTPPut,
  eHTTPDelete,
  eHTTPTrace,
  eHTTPOptions,
  eHTTPConnect
}bzhttp_eRequestType;

class BZF_API bzhttp_Request
{
public:

protected:
  class Data;
  Data  *data;
};

class BZF_API bzhttp_Responce
{
public:

protected:
  class Data;
  Data  *data;
};

class BZF_API bzhttp_VDir
{
public:
  virtual ~bzhttp_VDir(){};
  virtual const char* Name() = 0;
};

BZF_API bool bzhttp_RegisterVDir (bz_Plugin* plugin, bzhttp_VDir *vdir );
BZF_API bool bzhttp_RemoveVDir (bz_Plugin* plugin, bzhttp_VDir *vdir );
BZF_API bool bzhttp_RemoveAllVdirs (bz_Plugin* plugin );

#endif //_BZFS_HTTP_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
