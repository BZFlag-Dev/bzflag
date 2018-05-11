/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <CoreFoundation/CoreFoundation.h>

// if -directory is not used, this function is used to get the default path
// to the data directory which is located in the same directory as the
// application bundle
char *GetMacOSXDataPath(void)
{
  ::CFBundleRef	appBundle		= NULL;
  ::CFURLRef	resourceURL		= NULL;
  char *		string			= NULL;
  static char	basePath[2048]	= "<undefined resource path>";

  if ((appBundle = ::CFBundleGetMainBundle()) == NULL
      || (resourceURL = ::CFBundleCopyResourcesDirectoryURL(appBundle)) == NULL) {
    return NULL;
  }
  if (!::CFURLGetFileSystemRepresentation(resourceURL,
					  true, reinterpret_cast<UInt8 *>(basePath), sizeof(basePath))) {
    string = NULL;
    fprintf(stderr, "data path was not found\n");
  } else {
    string = basePath;
    fprintf(stderr, "data path is \"%s\"\n", string);
  }
  ::CFRelease(resourceURL);
  return string;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
