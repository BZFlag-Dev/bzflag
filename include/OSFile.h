/* 3dScreamers */
/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _OSFILE_H_
#define _OSFILE_H_

/* common header */
#include "common.h"

/* system headers */
#ifdef _WIN32
  #ifdef _MSC_VER
    #pragma warning(disable : 4786)  // Disable warning message
  #endif
  #define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
#undef NOMINMAX
#define NOMINMAX 1
  #include <windows.h>
  #include <io.h>
  #include <direct.h>
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <dirent.h>
  #include <ctype.h>
#endif

#include <string>
#include <vector>

#include <stdio.h>

typedef enum
{
  eFileStart,
  eCurentPos,
  eFileEnd
}teFilePos;

void setOSFileBaseDir(const std::string &dir);
void OSFileOSToStdDir(std::string &dir);

class OSFile
{
public:
  OSFile();
  OSFile(const OSFile &r);
  OSFile& operator = (const OSFile &r);

  OSFile(const std::string &szName);
  OSFile(const std::string &szName, const char *szMode);
  ~OSFile();

  bool open(const std::string &szName, const char *szMode);
  bool open(const char *szMode);
  bool close();

  void stdName(const std::string &szName);
  void osName(const std::string &szName);

  FILE* getFile();

  std::string getStdName();
  std::string getOSName();

  std::string getFileName();

  std::string getExtension();

  std::string getFullOSPath();

  std::string getOSFileDir();

  bool isOpen();

  int read(void* data, int size, int count = 1);
  unsigned char readChar();
  int scanChar(unsigned char *pChar);
  const char* scanStr();
  std::string readLine();
  int write(const void* data, int size);
  void flush();

  int seek(teFilePos ePos, int iOffset);
  unsigned int size();
  unsigned int tell();

  void setUseGlobalPath(bool use = false);
protected:
  class OSFileInfo;
  OSFileInfo    *info;
};


class OSDir
{
public:
  OSDir();
  OSDir(const OSDir &r);
  OSDir& operator = (const OSDir &r);
  OSDir(const std::string &DirName);
  ~OSDir();

  void setStdDir(const std::string &DirName);
  void setOSDir(const std::string &DirName);

  void makeStdDir(const std::string &DirName);
  void makeOSDir(const std::string &DirName);

  bool getNextFile(OSFile &oFile, bool bRecursive);
  bool getNextFile(OSFile &oFile, const char* fileMask, bool bRecursive);

  int getFileScanCount();

  std::string getStdName();
  std::string getOSName();
  std::string getFullOSPath();

  std::string getOSFileDir();

protected:
  class OSDirInfo;
  OSDirInfo    *info;

  bool windowsAddFileStack(std::string pathName, std::string fileMask, bool bRecursive);
  bool linuxAddFileStack(std::string pathName, std::string fileMask, bool bRecursive);
};


#endif//_OSFILE_H_
