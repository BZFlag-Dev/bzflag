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

#ifdef _MSC_VER
  #pragma warning( disable : 4786 )  // Disable warning message
#endif

// BZFlag common header
#include "common.h"

// interface header
#include "OSFile.h"

// system headers
#include <vector>
#include <string> //std::string
#include <algorithm>
#include <string.h> //c-style string
#include <stdio.h>
#ifndef _MSC_VER
  #include <stddef.h>
#endif

// local implementation headers
#include "TextUtils.h"

typedef std::vector<std::string> fileNameList;

// These are the core file name conversions
// std pathnames are unix style
// If you are adding a platform
// this is one place you need to add conversions to/from your OS

// utility functions
void OSFileStdToOSDir(std::string &dir)
{
  if (dir=="") return;
#ifndef _WIN32
  return;
#else
  std::replace(dir.begin(), dir.end(), '/', '\\');
#endif//WIN32
}

void OSFileOSToStdDir(std::string &dir)
{
//#ifndef _WIN32
//  return;
//#else
  std::replace(dir.begin(), dir.end(), '\\', '/');
//#endif//WIN32
}

// Windows specific make path
#ifdef _WIN32
void windowsMakePath(const std::string &path)
{
  std::string::size_type pos = 0;
  std::string::size_type delimitpos;

  do
  {
    delimitpos = path.find('\\', pos);
    std::string nextdir = path.substr(pos, delimitpos);
    mkdir(nextdir.c_str());
    pos = delimitpos + 1;
  } while (delimitpos != std::string::npos);
}
#endif // windows


/// global make path
void osMakePath(const std::string &path)
{
  if (path=="") return;
#ifdef _WIN32
  windowsMakePath(path);
#endif
}


// global path stuff
std::string  stdBaseDir;
std::string  osBaseDir;

void setOSFileBaseDir(const std::string &dir)
{
  stdBaseDir = dir;
  osBaseDir = dir;
  OSFileStdToOSDir(osBaseDir);
}

// OSFileClass
class OSFile::OSFileInfo
{
  public:
    OSFileInfo();
    OSFileInfo(const OSFile::OSFileInfo &r);
    std::string    stdName;
    std::string    osName;
    std::string    title;
    std::string    osPath;
    bool      useGlobalPath;
    FILE      *fp;
};

OSFile::OSFileInfo::OSFileInfo()
{
  fp = NULL;
  useGlobalPath = true;
}

OSFile::OSFileInfo::OSFileInfo(const OSFile::OSFileInfo &r)
{
  // never copy the file
  fp = NULL;
  osName = r.osName;
  stdName = r.stdName;
  title =  r.title;
  useGlobalPath = r.useGlobalPath;
}

void OSFile::setUseGlobalPath(bool use)
{
  info->useGlobalPath = use;
}

OSFile::OSFile()
{
  info = new OSFileInfo;
}

OSFile::OSFile(const std::string &name)
{
  info = new OSFileInfo;
  stdName(name);
}

OSFile::OSFile(const OSFile &r)
{
  info = new OSFileInfo(*r.info);
}

OSFile& OSFile::operator = (const OSFile &r)
{
  if (this == &r)
    return *this;

  if (info)
    delete (info);

  info = new OSFileInfo(*r.info);
  return *this;
}

OSFile::OSFile(const std::string &name, const char *mode)
{
  info = new OSFileInfo;
  stdName(name);
  open(name.c_str(), mode);
}

OSFile::~OSFile()
{
  close();
   delete (info);
}

bool OSFile::open(const std::string &name, const char *mode)
{
  close();
  stdName(name);
  return open(mode);
}

bool OSFile::open(const char *mode)
{
  close();

  char  modeToUse[32];

  if (!mode)
    sprintf(modeToUse, "rb");
  else
    strcpy(modeToUse, mode);

  std::string fileName;

  if (info->useGlobalPath)
    fileName = osBaseDir;
  fileName += info->osName;

  info->fp = fopen(fileName.c_str(),mode);

  // we may need to make the file path to the file, if we are writing then lets get on with it.
  if (!info->fp && strchr(mode,'w'))
  {
    osMakePath(fileName);
    info->fp = fopen(fileName.c_str(),mode);
  }

  return isOpen();
}

bool OSFile::close()
{
  if (info->fp)
    fclose(info->fp);

  info->fp = NULL;

  return (!isOpen());
}

int OSFile::read(void* data, int _size, int count)
{
  if (!isOpen())
    return 0;

  return (int)fread(data, _size, count, info->fp);
}

unsigned char OSFile::readChar()
{
  if (!isOpen())
    return 0;

  int c = getc(info->fp);
  if (c!=EOF)
    return c;
  else
    return 0;
}

std::string OSFile::readLine()
{
  std::string line;
  // reserve initial space for about a typical line length to speed up things
  line.reserve(20);

  char c = readChar();
  while (c != 0 && c != '\n' && c != 10)
  {
    line += c;
    c = readChar();
  }
  return line;
}

int OSFile::scanChar(unsigned char *pChar)
{
  if (!pChar || !isOpen())
    return 0;

  return fscanf(info->fp,"%c",pChar);
}

const char* OSFile::scanStr()
{
  if (!isOpen())
    return 0;

  static char temp[1024] = {0};
  if (fscanf(info->fp,"%s",temp)!=1)
    return NULL;
  return temp;
}

int OSFile::write(const void* data, int _size)
{
  if (!isOpen())
    return 0;

  return (int)fwrite(data, _size, 1, info->fp);
}

void OSFile::flush()
{
  fflush(info->fp);
}


int OSFile::seek(teFilePos ePos, int iOffset)
{
  if (!isOpen())
    return 0;

  long iMode;
  switch(ePos)
  {
    case eFileStart:
      iMode = SEEK_SET;
      break;

    case eFileEnd:
      iMode = SEEK_END ;
      break;

    case eCurentPos:
    default:
      iMode = SEEK_CUR ;
      break;
  }

  return fseek(info->fp,iOffset,iMode);
}

unsigned int OSFile::size()
{
  if (!isOpen())
    return 0;

  unsigned int pos = ftell(info->fp);
  fseek(info->fp,0,SEEK_END);
  unsigned int len = ftell(info->fp);
  fseek(info->fp,pos, SEEK_SET);

  return len;
}

unsigned int OSFile::tell()
{
  if (!isOpen())
    return 0;
  return ftell(info->fp);
}


void OSFile::stdName(const std::string &name)
{
  info->stdName = name;
  info->osName = name;
  OSFileStdToOSDir(info->osName);
}

void OSFile::osName(const std::string &name)
{
  info->stdName = name;
  info->osName = name;
  OSFileOSToStdDir(info->stdName);
}

std::string OSFile::getFileName()
{
  std::string::size_type lastslash = info->stdName.rfind('/');
  info->title = lastslash==std::string::npos ?
    info->stdName : info->stdName.substr(lastslash+1);

  // strip extension
  info->title.erase(info->title.rfind('.'));

  return info->title;
}

// this CAN return npos, cus the file may not have an extenstion, if it just happens to end in a '.' then well, your really weird Mr. File.
std::string OSFile::getExtension()
{
  std::string::size_type dot = info->stdName.rfind('.');
  return dot==std::string::npos ? "" : info->stdName.substr(dot+1);
}

std::string OSFile::getOSFileDir()
{
  std::string::size_type lastslash = info->stdName.rfind('/');
  info->osPath = lastslash==std::string::npos ?
    info->stdName : info->stdName.substr(0, lastslash);
  OSFileStdToOSDir(info->osPath);

  return info->osPath;
}

std::string OSFile::getFullOSPath()
{
  std::string path = osBaseDir;
  path += info->osName;
  return path;
}

FILE* OSFile::getFile()
{
  return info->fp;
}

std::string OSFile::getStdName()
{
  return info->stdName;
}

std::string OSFile::getOSName()
{
  return info->osName;
}

bool OSFile::isOpen()
{
  return info->fp != NULL;
}

// OS Dir classes
class OSDir::OSDirInfo
{
public:
  OSDirInfo();
  OSDirInfo(const OSDir::OSDirInfo &r);

  std::string    baseStdDir;
  std::string    baseOSDir;
  fileNameList    nameList;
  int      namePos;
};

OSDir::OSDirInfo::OSDirInfo()
{
  namePos = -1;
}

OSDir::OSDirInfo::OSDirInfo(const OSDir::OSDirInfo &r)
{
  baseStdDir = r.baseStdDir;
  baseOSDir = r.baseOSDir;
  nameList = r.nameList;
  namePos = r.namePos;
}

OSDir::OSDir()
{
  info = new OSDirInfo;
}

OSDir::OSDir(const OSDir& r)
{
  info = new OSDirInfo(*r.info);
}

OSDir& OSDir::operator = (const OSDir& r)
{
  if (this == &r)
    return *this;

  delete info;
  info = new OSDirInfo(*r.info);

  return *this;
}

OSDir::OSDir(const std::string &DirName)
{
  info = new OSDirInfo;
  setStdDir(DirName);
}

OSDir::~OSDir()
{
  if (info)
  {
    info->nameList.clear();
    delete(info);
  }
}

void OSDir::setStdDir(const std::string &DirName)
{
  info->baseStdDir = DirName;
  info->baseOSDir = DirName;
  OSFileStdToOSDir(info->baseOSDir);
}

void OSDir::setOSDir(const std::string &DirName)
{
  info->baseStdDir = DirName;
  info->baseOSDir = DirName;
  OSFileOSToStdDir(info->baseStdDir);
}

void OSDir::makeStdDir(const std::string &DirName)
{
  setStdDir(DirName);
#ifdef _WIN32
  mkdir(info->baseOSDir.c_str());
#else
  mkdir(info->baseOSDir.c_str(), 0777);
#endif
}

void OSDir::makeOSDir(const std::string &DirName)
{
  setOSDir(DirName);
#ifdef _WIN32
  mkdir(info->baseOSDir.c_str());
#else
  mkdir(info->baseOSDir.c_str(), 0777);
#endif
}

std::string OSDir::getStdName()
{
  return info->baseStdDir;
}

std::string  OSDir::getOSName()
{
  return info->baseOSDir.c_str();
}

std::string OSDir::getFullOSPath()
{
  std::string  FilePath = osBaseDir;
  FilePath += info->baseOSDir;
  return FilePath;
}

bool OSDir::getNextFile(OSFile &oFile, bool bRecursive)
{
  return getNextFile(oFile, NULL, bRecursive);
}

int OSDir::getFileScanCount()
{
  if (info)
    return (int)info->nameList.size();
  else
    return -1;
}

bool OSDir::getNextFile(OSFile &oFile, const char* fileMask, bool bRecursive)
{
#ifdef _WIN32
  std::string realMask = "*.*";  //FIXME -- could this also be just '*' ?
#else
  std::string realMask = "*";
#endif
  if (fileMask)
    realMask = fileMask;

  realMask = TextUtils::toupper(realMask);

  std::string theFileExt;
  if (info->namePos == -1)
  {
    info->nameList.clear();
  //FIXME -- just do the #ifdef'ing here?
    windowsAddFileStack(getFullOSPath(), realMask, bRecursive);
    linuxAddFileStack(getFullOSPath(), realMask, bRecursive);

    info->namePos = 0;
  }

  int size = info->nameList.size();
  if (info->namePos >= size)
  {
    info->namePos = -1;
    return false;
  }

  std::string fileName = info->nameList[info->namePos];

  if (osBaseDir.size()>1)
  {
    std::string temp = &(fileName.c_str()[osBaseDir.size()]);
    fileName = temp;
  }

  oFile.osName(fileName);
  info->namePos++;

  return true;
}

bool OSDir::windowsAddFileStack(std::string pathName, std::string fileMask, bool bRecursive)
{
#ifdef _WIN32
  struct _finddata_t fileInfo;

  long    hFile;
  std::string searchstr;

  std::string FilePath;

  bool  bDone = false;

  searchstr = pathName;
  searchstr += "\\";
  if (fileMask.size() > 0)
    searchstr += fileMask;
  else
    searchstr += "*.*";

  hFile = (long)_findfirst(searchstr.c_str(), &fileInfo);

  if (hFile != -1)
  {
    while (!bDone)
    {
      if ((strlen(fileInfo.name) >0) && (strcmp(fileInfo.name,".") != 0) &&
    (strcmp(fileInfo.name,"..") != 0))
      {
	FilePath = pathName;
	FilePath += "\\";
	FilePath += fileInfo.name;

	if ((fileInfo.attrib & _A_SUBDIR) && bRecursive)
	  windowsAddFileStack(FilePath,fileMask,bRecursive);
	else if (!(fileInfo.attrib & _A_SUBDIR))
	  info->nameList.push_back(FilePath);
      }
      if (_findnext(hFile,&fileInfo) == -1)
	bDone = true;
    }
  }
  return true;
#else
  // quell warnings
  if (!bRecursive) {
    fileMask.size();
    pathName.size();
  }
  return false;
#endif
}

// linux mask filter functions
// we don't need these for windows as it can do it right in findNextFile
#ifndef _WIN32
static int match_multi(const char **mask, const char **string)
{
  const char *msk;
  const char *str;
  const char *msktop;
  const char *strtop;

  msk = *mask;
  str = *string;

  while ((*msk != '\0') && (*msk == '*'))
    msk++;	  /* get rid of multiple '*'s */

  if (*msk == '\0')	/* '*' was last, auto-match */
    return +1;

  msktop = msk;
  strtop = str;

  while (*msk != '\0')
  {
    if (*msk == '*')
    {
      *mask = msk;
      *string = str;
      return 0;     /* matched this segment */
    }
    else if (*str == '\0')
      return -1;    /* can't match */
    else
    {
      if ((*msk == '?') || (*msk == *str))
      {
	msk++;
	str++;
	if ((*msk == '\0') && (*str != '\0'))  /* advanced check */
	{
	  str++;
	  strtop++;
	  str = strtop;
	  msk = msktop;
	}
      }
      else
      {
	str++;
	strtop++;
	str = strtop;
	msk = msktop;
      }
    }
  }

  *mask = msk;
  *string = str;
  return +1;		       /* full match */
}

static int match_mask (const char *mask, const char *string)
{
  if (mask == NULL)
    return 0;

  if (string == NULL)
    return 0;

  if ((mask[0] == '*') && (mask[1] == '\0'))
    return 1;		  /* instant match */

  while (*mask != '\0')
  {
    if (*mask == '*')
    {
      mask++;
      switch (match_multi (&mask, &string))
      {
	case +1:
	  return 1;
	case -1:
	  return 0;
      }
    }
    else if (*string == '\0')
      return 0;
    else if ((*mask == '?') || (*mask == *string))
    {
      mask++;
      string++;
    }
    else
      return 0;
  }

  if (*string == '\0')
    return 1;
  else
    return 0;
}
#endif

bool OSDir::linuxAddFileStack(std::string pathName, std::string fileMask, bool bRecursive)
{
#ifdef _WIN32
  // quell warnings
  if (!bRecursive) {
    fileMask.size();
    pathName.size();
  }
  return false;
#else
  DIR  *directory;
  dirent  *fileInfo;
  struct stat  statbuf;
  char   searchstr[1024];
  std::string  FilePath;

  strcpy(searchstr, pathName.c_str());
  if (searchstr[strlen(searchstr) - 1] != '/')
    strcat(searchstr, "/");
  directory = opendir(searchstr);
  if (!directory)
    return false;

  // TODO: make it use the filemask
  while ((fileInfo = readdir(directory)))
  {
    if (!((strcmp(fileInfo->d_name, ".") == 0) || (strcmp(fileInfo->d_name, "..") == 0)))
    {
      FilePath = searchstr;
      FilePath += fileInfo->d_name;
      strcpy(fileInfo->d_name, TextUtils::toupper(fileInfo->d_name).c_str());

      stat(FilePath.c_str(), &statbuf);

      if (S_ISDIR(statbuf.st_mode) && bRecursive)
	linuxAddFileStack(FilePath,fileMask,bRecursive);
      else if (match_mask(fileMask.c_str(), fileInfo->d_name))
	info->nameList.push_back(FilePath);
    }
  }
  closedir(directory);
  return true;
#endif// !Win32
}
