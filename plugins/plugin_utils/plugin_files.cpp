/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include "plugin_files.h"
#include "plugin_utils.h"



#ifdef _WIN32
#define _DirDelim '\\'

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <io.h>
#include <direct.h>

bool WindowsAddFileStack ( const char *szPathName, const char* fileMask, bool bRecursive,std::vector<std::string> &list, bool justDirs = false )
{
  struct _finddata_t fileInfo;

  long		hFile;
  std::string searchstr;

  std::string FilePath;

  bool	bDone = false;

  searchstr = szPathName;
  searchstr += "\\";
  if (bRecursive)
    searchstr += "*.*";
  else if (fileMask)
    searchstr += fileMask;
  else
    searchstr += "*.*";

  std::string extenstionSearch;

  if ( fileMask && strchr(fileMask,'.'))
    extenstionSearch = strchr(fileMask,'.')+1;

  hFile = (long)_findfirst(searchstr.c_str(),&fileInfo);

  if (hFile != -1)
  {
    while (!bDone)
    {
      if ((strlen(fileInfo.name) >0) && (strcmp(fileInfo.name,".") != 0) && 
	(strcmp(fileInfo.name,"..") != 0))
      {
	FilePath = szPathName;
	//if (!(fileInfo.attrib & _A_SUBDIR ))
	  FilePath += "\\";
	FilePath += fileInfo.name;

	if (justDirs && (fileInfo.attrib & _A_SUBDIR ))	// we neever do just dirs recrusively
	  list.push_back(FilePath);
	else if (!justDirs)
	{
	  if ( (fileInfo.attrib & _A_SUBDIR ) && bRecursive)
	    WindowsAddFileStack(FilePath.c_str(),fileMask,bRecursive,list);
	  else if (!(fileInfo.attrib & _A_SUBDIR) )
	  {
	    if (bRecursive && fileMask)	// if we are recusive we need to check extension manualy, so we get dirs and stuf
	    {
	      if (strrchr(FilePath.c_str(),'.'))
	      {
		if ( stricmp(strrchr(FilePath.c_str(),'.')+1, extenstionSearch.c_str() ) == 0 )
		  list.push_back(FilePath);
	      }
	    }
	    else
	      list.push_back(FilePath);
	  }
	}
      }
      if (_findnext(hFile,&fileInfo) == -1)
	bDone = true;
    }
  }
  return true;
}
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

#define _DirDelim '/'
static int match_multi (const char **mask, const char **string)
{
  const char *msk;
  const char *str;
  const char *msktop;
  const char *strtop;

  msk = *mask;
  str = *string;

  while ((*msk != '\0') && (*msk == '*'))
    msk++;                      /* get rid of multiple '*'s */

  if (*msk == '\0')				/* '*' was last, auto-match */
    return +1;

  msktop = msk;
  strtop = str;

  while (*msk != '\0')
  {
    if (*msk == '*')
    {
      *mask = msk;
      *string = str;
      return 0;                 /* matched this segment */
    }
    else if (*str == '\0')
      return -1;                /* can't match */
    else
    {
      if ((*msk == '?') || (*msk == *str))
      {
	msk++;
	str++;
	if ((*msk == '\0') && (*str != '\0'))	/* advanced check */
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
  return +1;											 /* full match */
}

static int match_mask (const char *mask, const char *string)
{ 
  if (mask == NULL)
    return 0;

  if (string == NULL)
    return 0;

  if ((mask[0] == '*') && (mask[1] == '\0'))
    return 1;									/* instant match */

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

bool LinuxAddFileStack ( const char *szPathName, const char* fileMask, bool bRecursive, std::vector<std::string> &list, bool justDirs = false )
{
  DIR	*directory;
  dirent	*fileInfo;
  struct stat	statbuf;
  char	searchstr[1024];
  std::string	FilePath;

  strcpy(searchstr, szPathName);
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


      stat(FilePath.c_str(), &statbuf);

      if (justDirs && S_ISDIR(statbuf.st_mode))	// we never do just dirs recrusively
	list.push_back(FilePath);
      else if (!justDirs)
      {
	if (S_ISDIR(statbuf.st_mode) && bRecursive)
	  LinuxAddFileStack(FilePath.c_str(),fileMask,bRecursive, list);
	else if (match_mask (fileMask, fileInfo->d_name))
	  list.push_back(FilePath);
      }
    }
  }
  closedir(directory);
  return true;
}
#endif 

std::string convertPathToDelims ( const char* file )  // ensures all the delims are constant
{
  if (!file)
    return std::string();

  std::string delim;
  delim += _DirDelim;
  return replace_all(replace_all(file,"/",delim),"\\",delim);
}

std::vector<std::string> getFilesInDir ( const char* dir, const char* filter, bool recrusive )
{
  std::vector<std::string> list;
  if (!dir)
    return list;

  std::string realFilter = "*.*";
  if ( filter )
    realFilter = filter;

  std::string directory  = convertPathToDelims(dir);

#ifdef _WIN32
  WindowsAddFileStack (directory.c_str(), realFilter.c_str(),recrusive,list);
#else
  LinuxAddFileStack(directory.c_str(), realFilter.c_str(),recrusive,list);
#endif
  return list;
}

std::vector<std::string> getDirsInDir ( const char* dir)
{
  std::vector<std::string> list;
  if (!dir)
    return list;

  std::string directory  = convertPathToDelims(dir);

#ifdef _WIN32
  WindowsAddFileStack (directory.c_str(), "*.*",false,list,true);
#else
  LinuxAddFileStack (directory.c_str(), "*.*",false,list,true);
#endif
  return list;
}

std::string getPathForOS ( const char* file )
{
  return convertPathToDelims(file);
}

std::string getFileDir ( const char* file )
{
  std::string f = convertPathToDelims(file);
  
  char *p = strrchr(f.c_str(),_DirDelim);
  if (p)				     // it's ok to go one past, cus even if it's the end, that's the NULL char so we can set it to NULL again with out worry
    *(p+1) = 0;

  return std::string(f.c_str());
}

std::string getFileExtension ( const char* file )
{
  std::string f = convertPathToDelims(file);

  char *p = strrchr(f.c_str(),'.');
  if (!p)				   // it's ok to go one past, cus even if it's the end, that's the NULL char so we can set it to NULL again with out worry
    return std::string();

  return std::string(p+1);
}

std::string getFileTitle ( const char* file )
{
  std::string f = convertPathToDelims(file);

  std::string temp = f;

  char *p = strrchr(f.c_str(),_DirDelim);
  if (p)				  // it's ok to go one past, cus even if it's the end, that's the NULL char so we can set it to NULL again with out worry
    temp = p+1;

  p = strrchr(temp.c_str(),'.');
  if (p)				  // it's ok to go one past, cus even if it's the end, that's the NULL char so we can set it to NULL again with out worry
    *(p+1) = 0;

  return std::string(temp.c_str());
}

std::string getFileText ( const char* file )
{
  std::string text;
  if (!file)
    return text;

  FILE *fp = fopen(convertPathToDelims(file).c_str(),"rb");
  if (!fp)
    return text;

  fseek(fp,0,SEEK_END);
  unsigned int i  = (unsigned int)ftell(fp);
  fseek(fp,0,SEEK_SET);

  char *temp = (char*)malloc(i+1);
  fread(temp,i,1,fp);
  temp[i] = 0;
  text = temp;
  free(temp);
  fclose(fp);

  return replace_all(text,"\r",std::string());
}

std::vector<std::string> getFileTextLines ( const char* file )
{
  return tokenize(getFileText(file),"\n",0,false);
}

unsigned int getFileLen ( const char* file )
{
  if (!file)
    return 0;

  FILE *fp = fopen(convertPathToDelims(file).c_str(),"rb");
  if (!fp)
    return 0;

  fseek(fp,0,SEEK_END);
  unsigned int i  = (unsigned int)ftell(fp);
  fclose(fp);

  return i;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
