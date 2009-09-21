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

// interface header
#include "FileManager.h"

// system headers
#include <string>
#include <ctype.h>
#include <fstream>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/types.h>
#else
#include <direct.h>
#endif

// local implementation headers
#include "StateDatabase.h"

//
// FileManager
//

// initialize the singleton
template <>
FileManager* Singleton<FileManager>::_instance = (FileManager*)NULL;


FileManager::FileManager()
{
  // do nothing
}

FileManager::~FileManager()
{
}


static bool isDirectory(const std::string& path)
{
  struct stat statbuf;
#ifndef WIN32
  if (stat(path.c_str(), &statbuf) == -1) {
    return false;
  }
  return S_ISDIR(statbuf.st_mode);
#else
  std::string dir = path;
  while (dir.find_last_of('\\') == (dir.size() - 1)) {
    dir.resize(dir.size() - 1); // strip trailing '\'s
  }
  if (_stat(dir.c_str(), &statbuf) == -1) {
    return false;
  }
  return ((statbuf.st_mode & _S_IFDIR) != 0);
#endif
}


static std::ifstream* getInStream(const std::string& path,
                                 std::ios::openmode mode)
{
  if (isDirectory(path)) {
    return NULL;
  }  
  std::ifstream* stream = new std::ifstream(path.c_str(), mode);
  if (stream && *stream) {
    return stream; // ready for reading
  }
  delete stream;
  return NULL;
}


std::istream* FileManager::createDataInStream(const std::string& filename,
                                              bool binary) const
{
  // choose open mode
  std::ios::openmode mode = std::ios::in;
  if (binary) {
    mode |= std::ios::binary;
  }

  std::ifstream* stream = NULL;

  const bool relative = !isAbsolute(filename);

  if (relative) {
    // try directory stored in DB
    if (BZDB.isSet("directory")) {
      stream = getInStream(catPath(BZDB.get("directory"), filename), mode);
      if (stream) {
        return stream;
      }
    }

    // try data directory
    stream = getInStream(catPath("data", filename), mode);
    if (stream) {
      return stream;
    }
  }

  // try current directory (or absolute path)
  stream = getInStream(filename, mode);
  if (stream) {
    return stream;
  }

  // try install directory
#if defined(BZFLAG_DATA)
  if (relative) {
    stream = getInStream(catPath(BZFLAG_DATA, filename), mode);
    if (stream) {
      return stream;
    }
  }
#endif

  return NULL;
}


std::ostream* FileManager::createDataOutStream(const std::string& filename,
					       bool binary, bool truncate) const
{
  // choose open mode
  std::ios::openmode mode = std::ios::out;
  if (binary)   { mode |= std::ios::binary; }
  if (truncate) { mode |= std::ios::trunc;  }

  const bool relative = !isAbsolute(filename);
  if (relative) {
    // try directory stored in DB
    if (BZDB.isSet("directory")) {
      const std::string path = catPath(BZDB.get("directory"), filename);
      std::ofstream* stream = new std::ofstream(path.c_str(), mode);
      if (stream && *stream) {
	return stream;
      }
      delete stream;
    }

    // try data directory
    {
      const std::string path = catPath("data", filename);
      std::ofstream* stream = new std::ofstream(path.c_str(), mode);
      if (stream && *stream) {
	return stream;
      }
      delete stream;
    }
  }
  else {
    // try absolute path
    int successMkdir = 0;
    int i = 0;
#ifndef _WIN32
    // create all directories above the file
    while ((i = filename.find('/', i+1)) != -1) {
      const std::string subDir = filename.substr(0, i);
      if (!isDirectory(subDir)) {
	successMkdir = mkdir(subDir.c_str(), 0755);
	if (successMkdir != 0) {
	  perror("Unable to make directory");
	  return NULL;
	}
      }
    }
#else
    // create all directories above the file
    i = 2; // don't stat on a drive, it will fail
    while ((i = filename.find('\\', i + 1)) != -1) {
      const std::string subDir = filename.substr(0, i);
      if (!isDirectory(subDir)) {
	successMkdir = _mkdir(subDir.c_str());
	/*if (successMkdir != 0)
	{
	  perror("Unable to make directory");
	  return NULL;
	} */
      }
    }
#endif

    std::ofstream* stream = new std::ofstream(filename.c_str(), mode);
    if (stream) {
      return stream;
    }

    delete stream;
  }

  return NULL;
}


bool FileManager::isAbsolute(const std::string& path) const
{
  if (path.empty())
    return false;

#if defined(_WIN32)
  const char* cpath = path.c_str();
  if (cpath[0] == '\\' || cpath[0] == '/')
    return true;
  if (path.size() >= 3 && isalpha(cpath[0]) && cpath[1] == ':' &&
      (cpath[2] == '\\' || cpath[2] == '/'))
    return true;
#else
  if (path.c_str()[0] == '/')
    return true;
#endif

  return false;
}


std::string FileManager::catPath(const std::string& a,
                                 const std::string& b) const
{
  // handle trivial cases
  if (a.empty())
    return b;
  if (b.empty())
    return a;

  std::string c = a;
#if defined(_WIN32)
  c += "\\";
#else
  c += "/";
#endif
  c += b;
  return c;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
