/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// a series of utilitys for bzfs plugins to use.
#ifndef _PLUGIN_FILES_H_
#define _PLUGIN_FILES_H_

#include "bzfsAPI.h"

std::vector<std::string> getFilesInDir ( const char* dir, const char* filter = "*.*", bool recursive = false );
inline std::vector<std::string> getFilesInDir ( const std::string &dir, const char* filter = "*.*", bool recursive = false ){return getFilesInDir(dir.c_str(),filter,recursive);}

std::vector<std::string> getDirsInDir ( const char* dir);
inline std::vector<std::string> getDirsInDir ( const std::string &dir ){return getDirsInDir(dir.c_str());}

std::string getFileDir ( const char* file );
inline std::string getFileDir ( const std::string &file ){return getFileDir(file.c_str());}

std::string getPathForOS ( const char* file );
inline std::string getPathForOS ( const std::string &file ){return getPathForOS(file.c_str());}

std::string getFileExtension ( const char* file );
inline std::string getFileExtension ( const std::string &file ){return getFileExtension(file.c_str());}

std::string getFileTitle ( const char* file );
inline std::string getFileTitle ( const std::string &file ){return getFileTitle(file.c_str());}

std::string getFileText ( const char* file );
inline std::string getFileText ( const std::string &file ){return getFileText(file.c_str());}

std::vector<std::string> getFileTextLines ( const char* file );
inline std::vector<std::string> getFileTextLines ( const std::string &file ){return getFileTextLines(file.c_str());}

unsigned int getFileLen ( const char* file );
inline unsigned int getFileLen ( std::string &file ){return getFileLen(file.c_str());}

std::string concatPaths ( const char* path1, const char* path2 );
inline std::string concatPaths ( const std::string &p1, const std::string &p2 ){return concatPaths(p1.c_str(),p2.c_str());}

bool fileExists ( const char *path );
inline bool fileExists ( const std::string &p1 ){ return fileExists(p1.c_str());}

#endif //_PLUGIN_FILES_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
