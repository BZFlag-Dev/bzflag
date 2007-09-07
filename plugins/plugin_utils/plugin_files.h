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

// a series of utilitys for bzfs plugins to use.
#ifndef _PLUGIN_FILES_H_
#define _PLUGIN_FILES_H_

#include <string>
#include <vector>
#include "bzfsAPI.h"

std::vector<std::string> getFilesInDir ( const char* dir, const char* filter = "*.*", bool recursive = false );
std::vector<std::string> getFilesInDir ( const std::string &dir, const char* filter = "*.*", bool recursive = false ){return getFilesInDir(dir.c_str(),filter,recursive);}

std::string getFileDir ( const char* file );
std::string getFileDir ( const std::string &file ){return getFileDir(file.c_str());}

std::string getPathForOS ( const char* file );
std::string getPathForOS ( const std::string &file ){return getPathForOS(file.c_str());}

std::string getFileExtension ( const char* file );
std::string getFileExtension ( const std::string &file ){return getFileExtension(file.c_str());}

std::string getFileTitle ( const char* file );
std::string getFileTitle ( const std::string &file ){return getFileTitle(file.c_str());}

std::string getFileText ( const char* file );
std::string getFileText ( const std::string &file ){return getFileText(file.c_str());}

std::vector<std::string> getFileTextLines ( const char* file );
std::vector<std::string> getFileTextLines ( const std::string &file ){return getFileTextLines(file.c_str());}

unsigned int getFileLen ( const char* file );
unsigned int getFileLen ( std::string &file ){return getFileLen(file.c_str());}


#endif //_PLUGIN_FILES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
