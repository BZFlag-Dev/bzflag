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

void setOSFileBaseDir ( const char *dir );

class OSFile
{
public:
    OSFile ();
    OSFile ( const OSFile &r);
    OSFile& operator = (const OSFile &r);

    OSFile ( const char *name );
    OSFile ( const char *name, const char *mode );
    ~OSFile();

    bool open ( const char *name, const char *mode );
    bool open ( const char *mode );
    bool close ();

    void stdName ( const char *name );
    void osName ( const char *name );

    FILE* getFile ();

    const char* getStdName ();
    const char* getOSName ();

    const char* getFileTitle ();

    const char* getExtension ();

    const char* getFullOSPath ();

    bool isOpen ();

    int read ( void* data, int size, int count = 1 );
    unsigned char readChar ();
    int scanChar ( unsigned char *pChar );
    const char* scanStr ();
    const char* readLine ();
    int write ( const void* data, int size );
    void flush ();

    int seek ( teFilePos ePos, int iOffset );
    unsigned int size ();
    unsigned int tell ();

    void setUseGlobalPath ( bool use = false );
protected:
    typedef struct OSFileInfo;
    OSFileInfo        *info;
};


class OSDir
{
public:
    OSDir();
    OSDir( const OSDir &r);
    OSDir& operator = (const OSDir &r);
    OSDir( const char* dirName );
    ~OSDir();

    void setStdDir ( const char* dirName );
    void setOSDir ( const char* dirName );

    void makeStdDir ( const char* dirName );
    void makeOSDir ( const char* dirName );

    bool getNextFile ( OSFile &oFile, bool recursive );
    bool getNextFile ( OSFile &oFile, const char* fileMask, bool recursive );

    const char* getStdName ();
    const char* getOSName ();
    const char* getFullOSPath ();

protected:
    struct OSDirInfo;
    OSDirInfo        *info;

    bool windowsAddFileStack ( const char *pathName, const char* fileMask , bool recursive );
    bool linuxAddFileStack( const char *pathName, const char* fileMask , bool recursive);
};


#endif//_OSFILE_H_
