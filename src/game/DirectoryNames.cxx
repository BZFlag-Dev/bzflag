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

/* interface header */
#include "DirectoryNames.h"

/* implementation system headers */
#ifndef _WIN32
#  include <stdlib.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <pwd.h>
#else   // _WIN32
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <stdio.h>
#  include <direct.h>
#  include <shlobj.h>
#endif  // _WIN32
#if defined(__APPLE__)
#  include <CoreServices/CoreServices.h>
#endif


// NOTE: terminate all strings with '/' or '\\'

static std::string		setupString(std::string dir)
{
  std::string name = getConfigDirName();
  name += dir;
  name += DirectorySeparator;
  return name;
}

std::string		getConfigDirName()
{
#if defined(_WIN32)
  std::string name("C:");
  char dir[MAX_PATH];
  ITEMIDLIST* idl;
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
    if (SHGetPathFromIDList(idl, dir)) {
      struct stat statbuf;
      if (stat(dir, &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) != 0)
	name = dir;
    }

    IMalloc* shalloc;
    if (SUCCEEDED(SHGetMalloc(&shalloc))) {
      shalloc->Free(idl);
      shalloc->Release();
    }
  }

  // yes your suposed to have the "my" in front of it. I know it's silly, but it's the MS way.
  name += "\\My BZFlag Files\\";
  
  return name;

#elif defined(__APPLE__)
  std::string name;
  ::FSRef libraryFolder;
  ::OSErr err;
  err = ::FSFindFolder(::kUserDomain, ::kApplicationSupportFolderType, true, &libraryFolder);
  if(err == ::noErr) {
    char buff[1024];
    err = ::FSRefMakePath(&libraryFolder, (UInt8*)buff, sizeof(buff));
    if(err == ::noErr) {
      std::strcat(buff, "/BZFlag/");
      name = buff;
    }
  }
  return name;
#else
  std::string name;
  struct passwd *pwent = getpwuid(getuid());
  if (pwent && pwent->pw_dir) {
    name += std::string(pwent->pw_dir);
    name += "/";
  }
  name += ".bzf/";

  return name;
#endif
}


extern std::string		getCacheDirName()
{
  std::string name = getConfigDirName();
  name += "cache";
#if !defined (_WIN32) && !defined (__APPLE__)
  // add in hostname on UNIX
  if (getenv("HOST")) {
    name += ".";
    name += getenv("HOST");
  }
#endif
  name += DirectorySeparator;
  return name;
}


extern std::string		getRecordDirName()
{
  return setupString("recordings");
}


extern std::string		getScreenShotDirName()
{
  return setupString("screenshots");
}


extern std::string		getWorldDirName()
{
  return setupString("worlds");
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
