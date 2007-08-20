/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "SharedObjectLoader.h"
#include "dlfcn.h"

bool SharedObjectLoader::load(std::string filename)
{
  char *err;

  if (filename.find('/') == std::string::npos)
    filename = "./" + filename;

  soHandle = dlopen(filename.c_str(), RTLD_LAZY);
  if (soHandle == NULL)
  {
    error = dlerror();
    return false;
  }

  dlerror(); // To clear error-var.
  createFunction = (createHandle)dlsym(soHandle, "create");
  if ((err = dlerror()))
  {
    error = err;
    return false;
  }

  dlerror(); // To clear error-var.
  destroyFunction = (destroyHandle)dlsym(soHandle, "destroy");
  if ((err = dlerror()))
  {
    error = err;
    return false;
  }

  return true;
}
SharedObjectLoader::~SharedObjectLoader()
{
  dlclose(soHandle);
}
BZAdvancedRobot *SharedObjectLoader::create(void)
{
  return (*createFunction)();
}
void SharedObjectLoader::destroy(BZAdvancedRobot *instance)
{
  (*destroyFunction)(instance);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
