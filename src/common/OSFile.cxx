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


#ifdef _MSC_VER
  #define strcasecmp stricmp
  #pragma warning( disable : 4786 )  // Disable warning message
#else
  #include <stddef.h>
#endif

#include <vector>
#include "OSFile.h"


void getUpperName ( char *name )
{
#ifdef    _WIN32
    strupr(name);
#else
    while(*name++)
	*name = toupper(*name);
#endif
}

typedef std::vector<std::string> OSFileNameList;

// these are the core file name conversions
// std pathnames are unix style
// if you are adding a platform
// this is one place you need to add conversions to/from your OS

// util functions
void osFileStdToOSDir ( char *dir )
{
#ifndef _WIN32
    return;
#else
    char *p = dir;

    while ( *p != NULL ) {
	if (*p == '/')
	    *p = '\\';

	p++;
    }
#endif//WIN32
}

void osFileOSToStdDir ( char *dir )
{
#ifndef _WIN32
    return;
#else
    char *p = dir;

    while ( *p != NULL ) {
	if (*p == '\\')
	    *p= '/';

	p++;
    }
#endif//WIN32
}

// windows specific make path
#ifdef _WIN32
void windowsMakePath(const char * path)
{
    // scan the folder path and make it
    char *pos = strchr (path,'\\');

    if (!pos) { // dono maybe the screwed up
	std::string str = "\\";
	str += path;
	mkdir(str.c_str());
	return;
    }

    while (pos) {
	std::string str = path;
	str.resize(pos-path);
	mkdir(str.c_str());
	pos = strchr (pos+1,'\\');
    }
}
#endif // windows


/// global make path
void osMakePath(const char * path)
{
#ifdef _WIN32
    windowsMakePath(path);
#endif

}


// global path stuff
std::string    stdBaseDir;
std::string    osBaseDir;

void setOSFileBaseDir ( const char *dir )
{
    stdBaseDir = dir;
    if (!dir)
	osBaseDir.empty();
    else {
	osBaseDir = stdBaseDir;
	osFileStdToOSDir((char*)osBaseDir.c_str());
    }
}

// OSFileClass
typedef struct OSFile::OSFileInfo
{
    OSFileInfo()
    {
	stdName = "";
    osName = "";
    title = "";
    useGlobalPath = true;
    fp = NULL;
    }

    OSFileInfo( std::string &_stdName, std::string &_osName, std::string &_title )
    {
	stdName = _stdName;
	osName = _osName;
	title = _title;
	useGlobalPath = true;
	fp = NULL;
    }

    const OSFileInfo& operator=( const OSFileInfo &xInfo )
    {
	stdName = xInfo.stdName;
	osName = xInfo.osName;
	title = xInfo.title;
	useGlobalPath = xInfo.useGlobalPath;
	fp = NULL;
    }

    std::string        stdName;
    std::string        osName;
    std::string        title;
    bool            useGlobalPath;
    FILE            *fp;
}OSFileInfo;

void OSFile::setUseGlobalPath ( bool use )
{
    info->useGlobalPath = use;
}

OSFile::OSFile ()
{
    info = new OSFileInfo;
}

OSFile::OSFile ( const char *name )
{
    info = new OSFileInfo;

    stdName(name);
}

OSFile::OSFile ( const OSFile &r)
{
    info = new OSFileInfo;
    info->fp = r.info->fp;
    info->osName = r.info->osName;
    info->stdName = r.info->stdName;
    info->title = r.info->title;
    info->useGlobalPath = r.info->useGlobalPath;

}

OSFile& OSFile::operator = (const OSFile &r)
{
    if (this != &r) {
	if (info)
	    delete (info);

	info = new OSFileInfo;
	info->fp = r.info->fp;
	info->osName = r.info->osName;
	info->stdName = r.info->stdName;
	info->title = r.info->title;
	info->useGlobalPath = r.info->useGlobalPath;
    }
    return *this;
}

OSFile::OSFile ( const char *name, const char *mode   )
{
    info = new OSFileInfo;
    info->fp = NULL;
    info->useGlobalPath = true;

    open(name,mode);
}

OSFile::~OSFile()
{
    close();
    if (info)
	delete (info);
}

bool OSFile::open ( const char *name, const char *mode  )
{
    close();

    stdName(name);

    return open(mode);
}

bool OSFile::open ( const char *mode )
{
    close();

    char    modeToUse[32];

    if (!mode)
	sprintf(modeToUse,"rb");
    else
	strcpy(modeToUse,mode);

    std::string    fileName;

    if (info->useGlobalPath)
	fileName = osBaseDir;
    fileName += info->osName;

    info->fp = fopen(fileName.c_str(),mode);

    // we may need to make the file path to the file, if we are writing then lets get on with it.
    if (!info->fp && strchr(mode,'w')) {
	osMakePath(fileName.c_str());
	info->fp = fopen(fileName.c_str(),mode);
    }

    return isOpen();
}

bool OSFile::close ( void )
{
    if (info->fp)
	fclose(info->fp);

    info->fp = NULL;

    return (!isOpen());
}

int OSFile::read ( void* data, int size, int count )
{
    if (!isOpen())
	return 0;

    return fread(data,size,count,info->fp);
}

unsigned char  OSFile::readChar ( void )
{
    if (!isOpen())
	return 0;

    char c = 0;

    if (fscanf(info->fp,"%c",&c) != 1)
	return 0;
    return c;
}

const char* OSFile::readLine ( void )
{
    static std::string line;

    line = "";
    char c = readChar();
    while (c != 0 && c != '/n' && c != 10) {
	line += c;
	c = readChar();
    }
    return line.c_str();
}

int OSFile::scanChar ( unsigned char *pChar )
{
    if (!pChar || !isOpen())
	return 0;

    return fscanf(info->fp,"%c",pChar);
}

const char* OSFile::scanStr ( )
{
    if (!isOpen())
	return 0;

    static char temp[1024] = {0};
    if (fscanf(info->fp,"%s",temp)!=1)
	return NULL;
    return temp;
}

int  OSFile::write ( const void* data, int size )
{
    if (!isOpen())
	return 0;

    return fwrite(data,size,1,info->fp);
}

void OSFile::flush ( void )
{
    fflush(info->fp);
}


int OSFile::seek ( teFilePos ePos, int iOffset )
{
    if (!isOpen())
	return 0;

    long iMode;
    switch(ePos) {
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

unsigned int OSFile::size ( void )
{
    if (!isOpen())
	return 0;

    unsigned int pos = ftell(info->fp);
    fseek(info->fp,0,SEEK_END);
    unsigned int len = ftell(info->fp);
    fseek(info->fp,pos, SEEK_SET);

    return len;
}

unsigned int OSFile::tell ( void )
{
    if (!isOpen())
	return 0;
    return ftell(info->fp);
}


void OSFile::stdName ( const char *name )
{
    info->stdName = name;
    info->osName = name;
    osFileStdToOSDir((char*)info->osName.c_str());
}

void OSFile::osName ( const char *name )
{
    info->stdName = name;
    info->osName = name;
    osFileOSToStdDir((char*)info->stdName.c_str());
}

const char* OSFile::getFileTitle ()
{
    if (info->stdName.size()== 0)
	return NULL;

    // yes I know this part is horible, put it in the pimple
    static std::string title;
    char    *path = strrchr(info->stdName.c_str(),'/');

    if (path)
	path++;
    else
	path = (char*)info->stdName.c_str();

    title = path;

    //FIXME - this is messy
    //FIXME - don't allow GetExtension to return NULL instead
    if (getExtension() == NULL) {
      title.resize(title.size() - 1);
    }
    else {
      title.resize(title.size() - strlen(getExtension()) - 1);
    }


    return title.c_str();
}

const char* OSFile::getExtension()
{
    if (info->stdName.size()== 0)
	return NULL;

    char    *pEnd = strrchr(info->stdName.c_str(),'.');

    if (pEnd)
	pEnd++;

    return pEnd;
}

const char* OSFile::getFullOSPath ( void )
{
    static std::string    szPath;

    szPath.empty();
    szPath = osBaseDir;
    szPath += info->osName;
    return szPath.c_str();
}

FILE* OSFile::getFile ( void )
{
    return info->fp;
}

const char* OSFile::getStdName ( void )
{
    return info->stdName.c_str();
}

const char* OSFile::getOSName ( void )
{
    return info->osName.c_str();
}

bool OSFile::isOpen ( void )
{
    return info->fp != NULL;
}

// OS Dir classes
typedef struct OSDir::OSDirInfo
{
    OSDirInfo()
    {
    baseStdDir = "";
    baseOSDir = "";
    namePos = 0;
    }

    OSDirInfo( const OSDirInfo &dInfo )
    {
    baseStdDir = dInfo.baseStdDir;
    baseOSDir = dInfo.baseOSDir;
    nameList = dInfo.nameList;
    namePos = dInfo.namePos;
    }

    std::string        baseStdDir;
    std::string        baseOSDir;
    OSFileNameList    nameList;
    unsigned int    namePos;
}OSDirInfo;

OSDir::OSDir()
{
    info = new OSDirInfo;
    info->namePos = -1;
}

OSDir::OSDir( const OSDir& r)
{
    info = new OSDirInfo( *r.info );
}

OSDir& OSDir::operator = (const OSDir& r)
{
    if (this != &r) {
	info->baseStdDir = r.info->baseStdDir;
	info->baseOSDir = r.info->baseOSDir;
	info->nameList = r.info->nameList;
	info->namePos = r.info->namePos;
    }

    return *this;
}


OSDir::OSDir( const char* szDirName )
{
    info = new OSDirInfo;
    setStdDir(szDirName);
    info->namePos = -1;
}

OSDir::~OSDir()
{
    if (info) {
	info->nameList.clear();
	delete(info);
    }
    info = NULL;
}

void OSDir::setStdDir ( const char* dirName )
{
    info->baseStdDir = dirName;
    info->baseOSDir = dirName;
    osFileStdToOSDir((char*)info->baseOSDir.c_str());
}

void OSDir::setOSDir ( const char* dirName )
{
    info->baseStdDir = dirName;
    info->baseOSDir = dirName;
    osFileOSToStdDir((char*)info->baseStdDir.c_str());
}

void OSDir::makeStdDir ( const char* dirName )
{
    setStdDir(dirName);
#ifdef _WIN32
    mkdir(info->baseOSDir.c_str());
#else
    mkdir(info->baseOSDir.c_str(), 0777);
#endif
}

void OSDir::makeOSDir ( const char* dirName )
{
    setOSDir(dirName);
#ifdef _WIN32
    mkdir(info->baseOSDir.c_str());
#else
    mkdir(info->baseOSDir.c_str(), 0777);
#endif
}

const char* OSDir::getStdName ( void )
{
    return info->baseStdDir.c_str();
}

const char* OSDir::getOSName ( void )
{
    return info->baseOSDir.c_str();
}

const char* OSDir::getFullOSPath ( void )
{
    static std::string    szFilePath;

    szFilePath.empty();
    szFilePath = osBaseDir;

    szFilePath += info->baseOSDir;
    return szFilePath.c_str();
}

bool OSDir::getNextFile ( OSFile &oFile, bool recursive )
{
    return getNextFile(oFile,NULL,recursive);
}

bool OSDir::getNextFile ( OSFile &oFile, const char* fileMask, bool recursive )
{
    std::string realMask = "*.*";
    if (fileMask)
	realMask = fileMask;

    getUpperName((char*)realMask.c_str());

    std::string theFileExt;
    if (info->namePos == -1) {
	info->nameList.clear();
	windowsAddFileStack(getFullOSPath(),realMask.c_str(),recursive);
	linuxAddFileStack(getFullOSPath(),realMask.c_str(),recursive);

	info->namePos = 0;
    }

    unsigned int size = info->nameList.size();
    if (info->namePos >= size) {
	info->namePos = -1;
	return false;
    }

    std::string    fileName = info->nameList[info->namePos];

    if (osBaseDir.size()>1) {
	std::string temp = &(fileName.c_str()[osBaseDir.size()]);
	fileName = temp;
    }

    oFile.osName(fileName.c_str());
    info->namePos++;
    return true;

    return true;
}

bool OSDir::windowsAddFileStack ( const char *szPathName, const char* fileMask, bool bRecursive   )
{
#ifdef _WIN32
    struct _finddata_t fileInfo;

    long        hFile;
    std::string searchstr;

    std::string FilePath;

    bool    bDone = false;

    searchstr = szPathName;
    searchstr += "\\";
    if (fileMask)
	searchstr += fileMask;
    else
	searchstr += "*.*";

    hFile = _findfirst(searchstr.c_str(),&fileInfo);

    if (hFile != -1) {
	while (!bDone) {
	    if ( (strlen(fileInfo.name) >0) && (strcmp(fileInfo.name,".") != 0) && (strcmp(fileInfo.name,"..") != 0)) {
		FilePath = szPathName;
		FilePath += "\\";
		FilePath += fileInfo.name;

		if ( (fileInfo.attrib & _A_SUBDIR ) && bRecursive)
		    windowsAddFileStack(FilePath.c_str(),fileMask,bRecursive);
		else if (!(fileInfo.attrib & _A_SUBDIR) )
		    info->nameList.push_back(FilePath);
	    }
	    if (_findnext(hFile,&fileInfo) == -1)
		bDone = true;
	}
    }
    return true;
#else
    return false;
#endif
}

// linux mask filter functions
// we don't need these for windows as it can do it right in findNextFile
#ifndef _WIN32
static int match_mask (const char *mask, const char *string)
{
    if (mask == NULL)
	return 0;

    if (string == NULL)
	return 0;

    if ((mask[0] == '*') &amp;&amp; (mask[1] == '\0'))
	return 1;                                    /* instant match */

    while (*mask != '\0') {
	if (*mask == '*') {
	    mask++;
	    switch (match_multi (&amp;mask, &amp;string)) {
		case +1:
		    return 1;
		case -1:
		    return 0;
	    }
	}
	else if (*string == '\0')
	    return 0;
	else if ((*mask == '?') || (*mask == *string)) {
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

static int match_multi (const char **mask, const char **string)
{
    const char *msk;
    const char *str;
    const char *msktop;
    const char *strtop;

    msk = *mask;
    str = *string;

    while ((*msk != '\0') &amp;&amp; (*msk == '*'))
	msk++;                      /* get rid of multiple '*'s */

    if (*msk == '\0')                /* '*' was last, auto-match */
	return +1;

    msktop = msk;
    strtop = str;

    while (*msk != '\0') {
	if (*msk == '*') {
	    *mask = msk;
	    *string = str;
	    return 0;                 /* matched this segment */
	}
	else if (*str == '\0')
	    return -1;                /* can't match */
	else {
	    if ((*msk == '?') || (*msk == *str)) {
		msk++;
		str++;
		if ((*msk == '\0') &amp;&amp; (*str != '\0'))    /* advanced check */
		{
		    str++;
		    strtop++;
		    str = strtop;
		    msk = msktop;
		}
	    }
	    else {
		str++;
		strtop++;
		str = strtop;
		msk = msktop;
	    }
	}
    }

    *mask = msk;
    *string = str;
    return +1;                                             /* full match */
}
#endif

bool OSDir::linuxAddFileStack (  const char *pathName, const char* fileMask, bool recursive  )
{
#ifdef _WIN32
    return false;//WIN32
#else
    DIR    *directory;
    dirent    *fileInfo;
    struct stat    statbuf;
    char    searchstr[1024];
    std::string    FilePath;

    strcpy(searchstr, szPathName);
    if (searchstr[strlen(searchstr) - 1] != '/')
	strcat(searchstr, "/");
    directory = opendir(searchstr);
    if (!directory)
	return false;

    // TODO: make it use the filemask
    while ((fileInfo = readdir(directory))) {
	if (!((strcmp(fileInfo-&gt;d_name, ".") == 0) || (strcmp(fileInfo-&gt;d_name, "..") == 0))) {
	    FilePath = searchstr;
	    FilePath += fileInfo-&gt;d_name;
	    GetUperName(fileInfo-&gt;d_name);

	    stat(FilePath.c_str(), &amp;statbuf);

	    if (S_ISDIR(statbuf.st_mode) &amp;&amp; bRecursive)
		LinuxAddFileStack(FilePath.c_str(),fileMask,bRecursive);
	    else if (match_mask (fileMask, fileInfo-&gt;d_name))
		info-&gt;nameList.push_back(FilePath);
	}
    }
    closedir(directory);
    return true;
#endif// !Win32
}
