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

#include "common.h"

/* interface header */
#include "SharedObjectLoader.h"

/* system headers */
#include <sys/types.h>
#include <stdint.h>
#ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
#endif


bool SharedObjectLoader::load(std::string filename)
{

#ifdef _WIN32
  return false;
#else

  char *err;
  void *createSymbol;
  void *destroySymbol;

  int32_t createAddr32, destroyAddr32;
  int64_t createAddr64, destroyAddr64;

  if (filename.find('/') == std::string::npos)
    filename = "./" + filename;

  soHandle = dlopen(filename.c_str(), RTLD_LAZY);
  if (soHandle == NULL) {
    error = dlerror();
    return false;
  }

  dlerror(); // To clear error-var.
  createSymbol = dlsym(soHandle, "create");
  if ((err = dlerror())) {
    error = err;
    return false;
  }

  /* extract the address in a posixly-pleasing fugly way */
  switch (sizeof(createSymbol)) {
    case (sizeof(int32_t)):
      createAddr32 = (int32_t)((int64_t)createSymbol);
      createFunction = reinterpret_cast<createHandle>(createAddr32);
      break;
    case (sizeof(int64_t)):
      createAddr64 = (int64_t)createSymbol;
      createFunction = (createHandle)createAddr64;
      break;
    default:
      error = "Unknown pointer address size!\n";
      return false;
  }


  dlerror(); // To clear error-var.
  destroySymbol = dlsym(soHandle, "destroy");
  if ((err = dlerror())) {
    error = err;
    return false;
  }

  /* extract the address in a posixly-pleasing fugly way */
  switch (sizeof(createSymbol)) {
    case (sizeof(int32_t)):
      destroyAddr32 = (int32_t)((int64_t)destroySymbol);
      destroyFunction = reinterpret_cast<destroyHandle>(destroyAddr32);
      break;
    case (sizeof(int64_t)):
      destroyAddr64 = (int64_t)destroySymbol;
      destroyFunction = (destroyHandle)destroyAddr64;
      break;
    default:
      error = "Unknown pointer address size!\n";
      return false;
  }

  return true;
#endif /* _WIN32 */
}


SharedObjectLoader::~SharedObjectLoader()
{
#ifndef _WIN32
  dlclose(soHandle);
#endif /* _WIN32 */
}


BZAdvancedRobot *SharedObjectLoader::create(void)
{
#ifdef _WIN32
  return NULL;
#else
  return (*createFunction)();
#endif /* _WIN32 */
}


void SharedObjectLoader::destroy(BZAdvancedRobot *instance)
{
#ifdef _WIN32
  return;
#else
  (*destroyFunction)(instance);
#endif /* _WIN32 */
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
