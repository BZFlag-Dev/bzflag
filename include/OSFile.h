/* 3dScreamers */
/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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

#ifdef _WIN32
  #ifdef _MSC_VER
    #pragma warning( disable : 4786 )  // Disable warning message
  #endif
  #define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
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

void setOSFileBaseDir ( const char *szDir );

class OSFile
{
public:
	OSFile ();
	OSFile ( const OSFile &r);
	OSFile& operator = (const OSFile &r);

	OSFile ( const char *szName );
	OSFile ( const char *szName, const char *szMode );
	~OSFile();

	bool open ( const char *szName, const char *szMode );
	bool open ( const char *szMode );
	bool close ( void );

	void stdName ( const char *szName );
	void osName ( const char *szName );

	FILE* getFile ( void );

	const char* getStdName ( void );
	const char* getOSName ( void );

	const char* getFileName ( void );

	const char* getExtension ( void );
	
	const char* getFullOSPath ( void );

	const char* getOSFileDir ( void );

	bool isOpen ( void );

	int read ( void* data, int size, int count = 1 );
	unsigned char readChar ( void );
	int scanChar ( unsigned char *pChar );
	const char* scanStr ( void );
	const char* readLine ( void );
	int write ( const void* data, int size );
	void flush ( void );

	int seek ( teFilePos ePos, int iOffset );
	unsigned int size ( void );
	unsigned int tell ( void );

	void setUseGlobalPath ( bool use = false );
protected:
	class OSFileInfo;
	OSFileInfo		*info;
};


class OSDir
{
public:
	OSDir();
	OSDir( const OSDir &r);
	OSDir& operator = (const OSDir &r);
	OSDir( const char* szDirName );
	~OSDir();

	void setStdDir ( const char* szDirName );
	void setOSDir ( const char* szDirName );

	void makeStdDir ( const char* szDirName );
	void makeOSDir ( const char* szDirName );

	bool getNextFile ( OSFile &oFile, bool bRecursive );
	bool getNextFile ( OSFile &oFile, const char* fileMask, bool bRecursive );

	int getFileScanCount ( void );

	const char* getStdName ( void );
	const char* getOSName ( void );
	const char* getFullOSPath ( void );

	const char* getOSFileDir ( void );

protected:
	class OSDirInfo;
	OSDirInfo		*info;

	bool windowsAddFileStack(std::string pathName, std::string fileMask, bool bRecursive );
	bool linuxAddFileStack(std::string pathName, std::string fileMask, bool bRecursive);
};


#endif//_OSFILE_H_
