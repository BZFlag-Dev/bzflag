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

#include "FileManager.h"
#include "string"
#include "StateDatabase.h"
#include <ctype.h>
#include <fstream>

#include <sys/stat.h>
#ifndef _WIN32
#include <sys/types.h>
#else
#include <direct.h>
#endif

//
// FileManager
//

FileManager*			FileManager::mgr = NULL;

FileManager::FileManager()
{
  // do nothing
}

FileManager::~FileManager()
{
  mgr = NULL;
}

FileManager*			FileManager::getInstance()
{
  if (mgr == NULL)
    mgr = new FileManager;
  return mgr;
}

std::istream*			FileManager::createDataInStream(
						const std::string& filename,
						bool binary) const
{
  // choose open mode
  std::ios::openmode mode = std::ios::in;
  if (binary)
    mode |= std::ios::binary;

  const bool relative = !isAbsolute(filename);
  if (relative) {
    // try directory stored in DB
    if (BZDB->isSet("directory")) {
      std::ifstream* stream = new std::ifstream(catPath(BZDB->get("directory"),
						filename).c_str(), mode);
      if (stream && *stream)
        return stream;
      delete stream;
    }

    // try data directory
    {
      std::ifstream* stream = new std::ifstream(catPath("data", filename).c_str(), mode);
      if (stream && *stream)
        return stream;
      delete stream;
    }
  }

  // try current directory (or absolute path)
  {
    std::ifstream* stream = new std::ifstream(filename.c_str(), mode);
    if (stream && *stream)
      return stream;
    delete stream;
  }

  // try install directory
#if defined(INSTALL_DATA_DIR)
  if (relative) {
    std::ifstream* stream = new std::ifstream(catPath(INSTALL_DATA_DIR,
					      filename).c_str(), mode);
    if (stream && *stream)
      return stream;
    delete stream;
  }
#endif

  return NULL;
}

std::ostream*			FileManager::createDataOutStream(
						const std::string& filename,
						bool binary,
						bool truncate) const
{
  // choose open mode
  std::ios::openmode mode = std::ios::out;
  if (binary)
    mode |= std::ios::binary;
  if (truncate)
    mode |= std::ios::trunc;

  const bool relative = !isAbsolute(filename);
  if (relative) {
    // try directory stored in DB
    if (BZDB->isSet("directory")) {
      std::ofstream* stream = new std::ofstream(catPath(BZDB->get("directory"),
						filename).c_str(), mode);
      if (stream && *stream)
        return stream;
      delete stream;
      return NULL;
    }

    // try data directory
    {
      std::ofstream* stream = new std::ofstream(catPath("data", filename).c_str(), mode);
      if (stream && *stream)
        return stream;
      delete stream;
      return NULL;
    }
  } else {
    // try absolute path
    int i = 0;
#ifndef _WIN32
    // create all directories above the file
    while ((i = filename.find('/', i+1)) != -1) {
      struct stat statbuf;
      if (!(stat(filename.substr(0, i).c_str(), &statbuf) == 0 &&
	    (S_ISDIR(statbuf.st_mode)))) {
	mkdir(filename.substr(0, i).c_str(), 0777);
      }
    }
#else
    // create all directories above the file
    while ((i = filename.find('\\', i+1)) != -1) {
      struct stat statbuf;
      if (!(stat(filename.substr(0, i).c_str(), &statbuf) == 0 &&
	    (_S_IFDIR & statbuf.st_mode))) {
		_mkdir(filename.substr(0, i).c_str());
      }
    }

#endif
    std::ofstream* stream = new std::ofstream(filename.c_str(), mode);
    if (stream && *stream)
      return stream;
    delete stream;
    return NULL;
  }
}

bool				FileManager::isAbsolute(const std::string& path) const
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
#elif defined(macintosh)
#error FIXME -- what indicates an absolute path on mac?
#else
  if (path.c_str()[0] == '/')
    return true;
#endif

  return false;
}

std::string			FileManager::catPath(
						const std::string& a,
						const std::string& b) const
{
  // handle trivial cases
  if (a.empty())
    return b;
  if (b.empty())
    return a;

#if defined(_WIN32)
  std::string c = a;
  c += "\\";
  c += b;
  return c;
#elif defined(macintosh)
  std::string c = a;
  c += ':';
  c += b;
#else
  std::string c = a;
  c += "/";
  c += b;
  return c;
#endif
}
// ex: shiftwidth=2 tabstop=8
