/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
#include "BzfString.h"
#include "StateDatabase.h"
#include <ctype.h>
#include <fstream>
typedef std::ifstream ifstream;
typedef std::ofstream ofstream;

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

istream*				FileManager::createDataInStream(
								const BzfString& filename,
								bool binary) const
{
	// choose open mode
	int mode = std::ios::in;
	if (binary)
		mode |= std::ios::binary;

	const bool relative = !isAbsolute(filename);
	if (relative) {
		// try directory stored in DB
		if (BZDB->isSet("directory")) {
			ifstream* stream = new ifstream(catPath(BZDB->get("directory"),
								filename).c_str(), mode);
			if (stream && *stream)
				return stream;
			delete stream;
		}

		// try data directory
		{
			ifstream* stream = new ifstream(catPath("data", filename).c_str(), mode);
			if (stream && *stream)
				return stream;
			delete stream;
		}
	}

	// try current directory (or absolute path)
	{
		ifstream* stream = new ifstream(filename.c_str(), mode);
		if (stream && *stream)
			return stream;
		delete stream;
	}

	// try install directory
#if defined(INSTALL_DATA_DIR)
	if (relative) {
		ifstream* stream = new ifstream(catPath(INSTALL_DATA_DIR,
								filename).c_str(), mode);
		if (stream && *stream)
			return stream;
		delete stream;
	}
#endif

	return NULL;
}

ostream*				FileManager::createDataOutStream(
								const BzfString& filename,
								bool binary,
								bool truncate) const
{
	// choose open mode
	int mode = std::ios::out;
	if (binary)
		mode |= std::ios::binary;
	if (truncate)
		mode |= std::ios::trunc;

	const bool relative = !isAbsolute(filename);
	if (relative) {
		// try directory stored in DB
		if (BZDB->isSet("directory")) {
			ofstream* stream = new ofstream(catPath(BZDB->get("directory"),
								filename).c_str(), mode);
			if (stream && *stream)
				return stream;
			delete stream;
			return NULL;
		}

		// try data directory
		{
			ofstream* stream = new ofstream(catPath("data", filename).c_str(), mode);
			if (stream && *stream)
				return stream;
			delete stream;
			return NULL;
		}
	}

	// try absolute path
	else {
		ofstream* stream = new ofstream(filename.c_str(), mode);
		if (stream && *stream)
			return stream;
		delete stream;
		return NULL;
	}
}

bool					FileManager::isAbsolute(const BzfString& path) const
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

BzfString				FileManager::catPath(
								const BzfString& a,
								const BzfString& b) const
{
	// handle trivial cases
	if (a.empty())
		return b;
	if (b.empty())
		return a;

#if defined(_WIN32)
	BzfString c = a;
	c += "\\";
	c += b;
	return c;
#elif defined(macintosh)
#error FIXME -- what is the mac path separator?
#else
	BzfString c = a;
	c += "/";
	c += b;
	return c;
#endif
}
