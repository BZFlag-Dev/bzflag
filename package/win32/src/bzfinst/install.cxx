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

/*
 * installs files, etc. using the compressed database as a source
 */

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "install.h"
#include "Uncompressor.h"
#include "undo.h"

extern int doMessages();
extern "C" {
extern byte_t database[];
}

/* to do:
 *	code to add DLL
 */

//
// registry key search
//

static HKEY		findKey(const char* name)
{
    char tmp[1024];

    const char* head = name;
    const char* tail = head;

    // open base key
    HKEY key;
    while (*tail && *tail != '\\')
	tail++;
    strncpy(tmp, head, tail - head);
    tmp[tail - head] = '\0';
    if (strcmp(tmp, "HKCR") == 0)
	key = HKEY_CLASSES_ROOT;
    else if (strcmp(tmp, "HKCU") == 0)
	key = HKEY_CURRENT_USER;
    else if (strcmp(tmp, "HKLM") == 0)
	key = HKEY_LOCAL_MACHINE;
    else if (strcmp(tmp, "HKUS") == 0)
	key = HKEY_USERS;
    else
	return NULL;

    // open subkeys all the way down
    while (*tail) {
	// find next component of name
	head = ++tail;
	if (!*tail) break;
	while (*tail && *tail != '\\')
	    tail++;

	// make name
	strncpy(tmp, head, tail - head);
	tmp[tail - head] = '\0';

	// open key
	HKEY subkey;
	LONG hr = RegOpenKeyEx(key, tmp, 0, KEY_ALL_ACCESS, &subkey);
	RegCloseKey(key);
	if (hr != ERROR_SUCCESS)
	    return NULL;

	// descend
	key = subkey;
    }

    return key;
}

//
// undo items
//

class UndoNewFile : public UndoItem {
  public:
    UndoNewFile(const char* name);
    ~UndoNewFile() { delete[] d_name; }

    void		execute();

  private:
    char*		d_name;
};

UndoNewFile::UndoNewFile(const char* name)
{
    d_name = new char[strlen(name) + 1];
    strcpy(d_name, name);
}

void			UndoNewFile::execute()
{
    unlink(d_name);
}

class UndoNewDirectory : public UndoItem {
  public:
    UndoNewDirectory(const char* name);
    ~UndoNewDirectory() { delete[] d_name; }

    void		execute();

  private:
    char*		d_name;
};

UndoNewDirectory::UndoNewDirectory(const char* name)
{
    d_name = new char[strlen(name) + 1];
    strcpy(d_name, name);
}

void			UndoNewDirectory::execute()
{
    rmdir(d_name);
}

class UndoNewRegKey : public UndoItem {
  public:
    UndoNewRegKey(const char* parent, const char* name);
    ~UndoNewRegKey() { delete[] d_parent; delete[] d_name; }

    void		execute();

  private:
    char*		d_parent;
    char*		d_name;
};

UndoNewRegKey::UndoNewRegKey(const char* parent, const char* name)
{
    d_parent = new char[strlen(parent) + 1];
    strcpy(d_parent, parent);
    d_name = new char[strlen(name) + 1];
    strcpy(d_name, name);
}

void			UndoNewRegKey::execute()
{
    HKEY key = findKey(d_parent);
    if (key) {
	RegDeleteKey(key, d_name);
	RegCloseKey(key);
    }
}


class UndoNewRegValue : public UndoItem {
  public:
    UndoNewRegValue(const char* parent, const char* name,
				DWORD type, const void* value, DWORD size);
    ~UndoNewRegValue();

    void		execute();

  private:
    char*		d_parent;
    char*		d_name;
    char*		d_value;
    DWORD		d_type;
    DWORD		d_size;
};

UndoNewRegValue::UndoNewRegValue(const char* parent, const char* name,
				DWORD type, const void* value, DWORD size) :
				d_value(NULL), d_type(type), d_size(size)
{
    d_parent = new char[strlen(parent) + 1];
    strcpy(d_parent, parent);
    d_name = new char[strlen(name) + 1];
    strcpy(d_name, name);
    if (value) {
	d_value = new char[size];
	memcpy(d_value, value, size);
    }
}

UndoNewRegValue::~UndoNewRegValue()
{
    delete[] d_parent;
    delete[] d_name;
    delete[] d_value;
}

void			UndoNewRegValue::execute()
{
    HKEY key = findKey(d_parent);
    if (key) {
	RegDeleteValue(key, d_name);
	if (d_value)
	    RegSetValueEx(key, d_name, 0, d_type, (const BYTE*)d_value, d_size);
	RegCloseKey(key);
    }
}


//
// utilities
//

static BOOL		setDirectory(const char* dir, ErrorCB errorCB)
{
    if (chdir(dir) < 0) {
	if (errorCB) {
	    char buffer[1024];
	    sprintf(buffer, "Can't change to directory:%s\n", dir);
	    (*errorCB)(buffer);
	}
	return FALSE;
    }

    return TRUE;
}

static BOOL		makeDirectory(const char* dir,
				UndoList& undoList, ErrorCB errorCB)
{
    if (setDirectory(dir, NULL))
	return TRUE;

    char tmpdir[_MAX_PATH];
    const char *src = dir;
    char *dst = tmpdir;
    *dst++ = *src++;	// copy `X:\'
    *dst++ = *src++;
    while (*src) {
	// append next component
	*dst++ = *src++;
	while (*src != '\\' && *src != '\0')
	    *dst++ = *src++;
	*dst = '\0';

	struct stat statbuf;
	if (stat(tmpdir, &statbuf) < 0) {
	    if (errno != ENOENT) {
		char buffer[1024];
		sprintf(buffer, "Can't create directory:%s\n", tmpdir);
		(*errorCB)(buffer);
		return FALSE;
	    }
	    if (mkdir(tmpdir) < 0) {
		char buffer[1024];
		sprintf(buffer, "Can't create directory:%s\n", tmpdir);
		(*errorCB)(buffer);
		return FALSE;
	    }
	    undoList.append(new UndoNewDirectory(tmpdir));
	}
	else if (!(statbuf.st_mode & _S_IFDIR)) {
	    char buffer[1024];
	    sprintf(buffer, "Path is not a directory:%s\n", tmpdir);
	    (*errorCB)(buffer);
	    return FALSE;
	}
    }

    return setDirectory(dir, errorCB);
}

int			installFile(const char* path, Uncompressor& stream,
				UndoList& undoList, int& totalSpace,
				ErrorCB errorCB,
				SetMeterCB fileCB, SetMeterCB totalCB)
{
    (*fileCB)(0);

    DWORD n = stream.getUInt32();
    if (n < 0)
	return 0;

    // remove file
    if (unlink(path) < 0 && errno == EACCES) {
	char buffer[2048];
	sprintf(buffer, "Can't remove read-only file:\n%s", path);
	(*errorCB)(buffer);
	return -1;
    }

    // create file
    int fd = open(path, _O_CREAT | _O_TRUNC | _O_RDWR |
			_O_BINARY | _O_SEQUENTIAL, 0666);
    if (fd < 0) {
	char buffer[2048];
	sprintf(buffer, "Can't create file:\n%s", path);
	(*errorCB)(buffer);
	return -1;
    }

    // undo file creation
    undoList.append(new UndoNewFile(path));

    // copy block by block
    const DWORD requiredSpace = (DWORD)getRequiredSpace();
    const DWORD size = n;
    unsigned char block[4096];
    while (n > 0) {
	int bytes = n;
	if (bytes > sizeof(block))
	    bytes = sizeof(block);
	if (stream.getData(block, bytes) != bytes) {
	    close(fd);
	    return 0;
	}
	errno = 0;
	if (write(fd, block, bytes) != bytes) {
	    char buffer[2048];
	    sprintf(buffer, (errno == ENOSPC) ?
				"Disk full writing:\n%s" :
				"Can't write file:\n%s", path);
	    close(fd);
	    (*errorCB)(buffer);
	    return -1;
	}
	n -= bytes;
	totalSpace += bytes;
	(*fileCB)(100 * (size - n) / size);
	(*totalCB)(100 * totalSpace / requiredSpace);

	// send some windows messages
	if (doMessages())
	    break;
    }

    (*fileCB)(100);
    close(fd);
    return 1;
}

static BOOL		replaceTags(char* args, int maxLen,
				const char* cwd,
				const char* instDir,
				const char* sysDir,
				const char* winDir)
{
    int len = 0;
    const char* scan = args;
    while (*scan) {
	if (*scan == '%') {
	    switch (scan[1]) {
	      default:
		len += 2;
		break;

	      case 'c':
		len += strlen(cwd);
		break;

	      case 'i':
		len += strlen(instDir);
		break;

	      case 's':
		len += strlen(sysDir);
		break;

	      case 'w':
		len += strlen(winDir);
		break;
	    }
	    scan += 2;
	}
	else {
	    len++;
	    scan++;
	}
    }

    // check transformed length
    if (len >= maxLen)
	return FALSE;

    // copy original
    char* src = strdup(args);

    // transform
    char* dst = args;
    for (scan = src; *scan; scan++) {
	if (*scan == '%') {
	    switch (scan[1]) {
	      default:
		*dst++ = scan[0];
		*dst++ = scan[1];
		break;

	      case 'c':
		strcpy(dst, cwd);
		dst += strlen(cwd);
		break;

	      case 'i':
		strcpy(dst, instDir);
		dst += strlen(instDir);
		break;

	      case 's':
		strcpy(dst, sysDir);
		dst += strlen(sysDir);
		break;

	      case 'w':
		strcpy(dst, winDir);
		dst += strlen(winDir);
		break;
	    }
	    scan++;
	}
	else {
	    *dst++ = *scan;
	}
    }
    *dst = '\0';

    // cleanup
    free(src);
    return TRUE;
}

static BOOL		addDesktopLink(const char* linkname,
				UndoList& undoList,
				const char* name,
				const char* args,
				const char* workdir)
{
    HRESULT hres;
    char winDir[_MAX_PATH];
    static const char* desktop = "\\Desktop\\";
    static const char* linkext = ".lnk";

    // get user profile directory
    const char* profile = getenv("USERPROFILE");
    if (!profile) {
	GetWindowsDirectory(winDir, sizeof(winDir));
	profile = winDir;
    }
    if (!profile)
	return FALSE;

    // construct link pathname
    char* linkPath = new char[strlen(profile) +
				strlen(desktop) +
				strlen(linkname) +
				strlen(linkext) + 1];
    sprintf(linkPath, "%s%s%s%s", profile, desktop, linkname, linkext);

    IShellLink* shellLink;
    hres = CoCreateInstance(CLSID_ShellLink, NULL,
				CLSCTX_INPROC_SERVER,
				IID_IShellLink, (LPVOID*)&shellLink);
    if (SUCCEEDED(hres)) {
	shellLink->SetPath(name);
	shellLink->SetArguments(args);
	shellLink->SetWorkingDirectory(workdir);

	IPersistFile* persistFile;
	hres = shellLink->QueryInterface(IID_IPersistFile,
				      (LPVOID*)&persistFile);

	if (SUCCEEDED(hres)) {
	    WORD wideLinkPath[MAX_PATH];
	    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
				linkPath, -1, wideLinkPath, MAX_PATH);

	    hres = persistFile->Save(wideLinkPath, TRUE);
	    persistFile->Release();
	}

	shellLink->Release();
    }

    if (SUCCEEDED(hres)) {
	undoList.append(new UndoNewFile(linkPath));
    }

    delete[] linkPath;
    return TRUE;
}

// if allowRegistryFailure is TRUE then ignore failure to
// write registry.  since we only use the registry to save
// uninstall info, we're safe in ignoring failures (the
// uninstaller just won't be available from Add/Remove
// programs).  some users complained that they couldn't
// install because they didn't have permission to edit the
// registry and there was no way to override the registry
// changing.
//
// FIXME -- must do more sophisticated handling when we
// use the registry for stuff that actually matters.
static BOOL allowRegistryFailure = TRUE;

static BOOL		addRegistryKey(const char* parent, const char* name,
				UndoList& undoList)
{
    HKEY key, subkey;

    // open parent
    key = findKey(parent);
    if (!key) return allowRegistryFailure;

    DWORD disp;
    LONG hr = RegCreateKeyEx(key, name, 0, REG_NONE, 0,
				KEY_ALL_ACCESS, NULL, &subkey,
				&disp);
    RegCloseKey(key);
    if (hr != ERROR_SUCCESS)
	return allowRegistryFailure;

    if (disp != REG_OPENED_EXISTING_KEY)
	undoList.append(new UndoNewRegKey(parent, name));
    RegCloseKey(subkey);
    return TRUE;
}

static BOOL		addRegistryDWord(const char* parent,
				const char* name, DWORD value,
				UndoList& undoList)
{
    // open parent
    HKEY key = findKey(parent);
    if (!key) return allowRegistryFailure;

    // get the current value
    BYTE* data = NULL;
    DWORD type, size = 0;
    LONG hr = RegQueryValueEx(key, name, 0, &type, data, &size);
    UndoItem* undo = NULL;
    if (hr == ERROR_MORE_DATA) {
	data = new BYTE[size];
	hr = RegQueryValueEx(key, name, 0, &type, data, &size);
	if (hr != ERROR_SUCCESS) {
	    delete[] data;
	    RegCloseKey(key);
	    return allowRegistryFailure;
	}
	undo = new UndoNewRegValue(parent, name, type, data, size);
	delete[] data;
    }
    else {
	undo = new UndoNewRegValue(parent, name, 0, NULL, 0);
    }

    hr = RegSetValueEx(key, name, 0, REG_DWORD,
				(const BYTE*)&value, sizeof(value));
    RegCloseKey(key);
    if (hr != ERROR_SUCCESS) {
	delete undo;
	return allowRegistryFailure;
    }

    if (undo)
	undoList.append(undo);
    return TRUE;
}

static BOOL		addRegistryString(const char* parent,
				const char* name, const char* string,
				UndoList& undoList)
{
    // open parent
    HKEY key = findKey(parent);
    if (!key) return allowRegistryFailure;

    // get the current value
    BYTE* data = NULL;
    DWORD type, size = 0;
    LONG hr = RegQueryValueEx(key, name, 0, &type, data, &size);
    UndoItem* undo = NULL;
    if (hr == ERROR_MORE_DATA) {
	data = new BYTE[size];
	hr = RegQueryValueEx(key, name, 0, &type, data, &size);
	if (hr != ERROR_SUCCESS) {
	    delete[] data;
	    RegCloseKey(key);
	    return allowRegistryFailure;
	}
	undo = new UndoNewRegValue(parent, name, type, data, size);
	delete[] data;
    }

    hr = RegSetValueEx(key, name, 0, REG_SZ,
				(const BYTE*)string, strlen(string));
    RegCloseKey(key);
    if (hr != ERROR_SUCCESS) {
	delete undo;
	return allowRegistryFailure;
    }

    if (undo)
	undoList.append(undo);
    return TRUE;
}

//
// installation stuff
//

int			getRequiredSpace()
{
    return (int)(((uint32_t)database[3] << 24) +
		((uint32_t)database[2] << 16) +
		((uint32_t)database[1] << 8)  +
		(uint32_t)database[0]);
}

int			install(const char* instDir,
				ErrorCB errorCB, SetNameCB nameCB,
				SetMeterCB fileCB, SetMeterCB totalCB,
				SetNameCB readmeCB)
{
    // get other interesting directories
    char sysDir[_MAX_PATH], winDir[_MAX_PATH], root[_MAX_DRIVE + 1];
    GetSystemDirectory(sysDir, sizeof(sysDir));
    GetWindowsDirectory(winDir, sizeof(winDir));
    sprintf(root, "%.3s", instDir);

    // get the drive's info
    DWORD sectorsPerCluster, bytesPerSector;
    DWORD numFreeClusters, totalClusters;
    GetDiskFreeSpace(root, &sectorsPerCluster, &bytesPerSector,
			&numFreeClusters, &totalClusters);

    // check for disk space
    DWORD requiredClusters = (getRequiredSpace() + bytesPerSector - 1) /
							bytesPerSector;
    requiredClusters = (requiredClusters + sectorsPerCluster - 1) /
							sectorsPerCluster;
    if (requiredClusters >= numFreeClusters) {
	(*errorCB)("Insufficient disk space.\n"
			"Free some space or choose an\n"
			"installation folder on another disk.");
	return FALSE;
    }

    // initialize undo buffer
    UndoList undoList(root);

    // make missing directories up to installation directory
    if (!makeDirectory(instDir, undoList, errorCB))
	return FALSE;

    // initialize inst variables
    int dirStack = 0, totalSpace = 0;
    char cwd[2 * _MAX_PATH], name[_MAX_FNAME + _MAX_EXT - 1], path[_MAX_PATH];
    strcpy(cwd, instDir);

    // make input stream
    const int databaseLen = (int)(((uint32_t)database[7] << 24) +
				((uint32_t)database[6] << 16) +
				((uint32_t)database[5] << 8)  +
				(uint32_t)database[4]);
    Uncompressor stream(database + 8, databaseLen);

    // read commands from stream and execute them
    int okay = TRUE;
    while (!stream.eof()) {
	if (doMessages()) {
	    okay = FALSE;
	    break;
	}

	const int cmd = (int)stream.getByte();
	switch (cmd) {
	  // switch to the base installation directory, system
	  // directory, or windows directory and flush stack.
	  case INST_SETINSTDIR:
	  case INST_SETSYSDIR:
	  case INST_SETWINDIR:
	    (*nameCB)("");
	    (*fileCB)(0);
	    if (cmd == INST_SETINSTDIR)
		strcpy(cwd, instDir);
	    else if (cmd == INST_SETSYSDIR)
		strcpy(cwd, sysDir);
	    else
		strcpy(cwd, winDir);
	    dirStack = 0;
	    if (!setDirectory(cwd, errorCB)) {
		return FALSE;
	    }
	    break;

	  // (possibly) create a subdirectory and cd to it
	  case INST_PUSHDIR:
	    (*nameCB)("");
	    (*fileCB)(0);
	    strcat(cwd, "\\");
	    if (stream.getString(cwd + strlen(cwd), _MAX_DIR) == -1) {
		okay = FALSE;
	    }
	    else if (!makeDirectory(cwd, undoList, errorCB)) {
		return FALSE;
	    }
	    else if (strlen(cwd) > _MAX_DIR) {
		(*errorCB)("Path name too long.\n"
				"Choose a shorter installation folder name.");
		return FALSE;
	    }
	    dirStack++;
	    break;

	  // cd to the parent directory
	  case INST_POPDIR:
	    (*nameCB)("");
	    (*fileCB)(0);
	    if (--dirStack < 0) {
		okay = FALSE;
	    }
	    else {
		char* tail = strrchr(cwd, '\\');
		if (!tail) {
		    okay = FALSE;
		}
		else {
		    *tail = '\0';
		    if (!setDirectory(cwd, errorCB))
			return FALSE;
		}
	    }
	    break;

	  // install a file
	  case INST_FILE:
	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }
	    sprintf(path, "%s\\%s", cwd, name);

	    // notify user
	    (*nameCB)(name);

	    switch (installFile(path, stream, undoList, totalSpace,
					errorCB, fileCB, totalCB)) {
	      case -1:
		return FALSE;
	      case 0:
		okay = FALSE;
	    }
	    break;

#if 0
	  // install a DLL
	  case INST_DLL:
	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }
	    sprintf(path, "%s\\%s", cwd, name);

	    // test version
	    // (HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\SharedDLLs)
	    // FIXME

	    // notify user
	    (*nameCB)(name);

	    // install to a temporary location
	    // FIXME -- make tmp filename
	    switch (installFile(name, stream, undoList, totalSpace,
					errorCB, fileCB, totalCB)) {
	      case -1:
		return FALSE;
	      case 0:
		okay = FALSE;
	    }

	    // remove original dll
	    // FIXME

	    // move new dll
	    // FIXME

	    // increment dll count if in shared list
	    // FIXME

	    break;
#endif

	  case INST_REGKEY:
	    (*fileCB)(0);
	    (*nameCB)("Registry...");

	    if (stream.getString(path, sizeof(path)) == -1) {
		okay = FALSE;
		break;
	    }

	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }

	    if (!addRegistryKey(path, name, undoList)) {
		char buffer[2048];
		sprintf(buffer, "Can't create registry key:\n%s\\%s",
							path, name);
		(*errorCB)(buffer);
		return FALSE;
	    }
	    break;

	  case INST_REGSTRING: {
	    (*fileCB)(0);
	    (*nameCB)("Registry...");

	    if (stream.getString(path, sizeof(path)) == -1) {
		okay = FALSE;
		break;
	    }

	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }

	    char value[2048];
	    if (stream.getString(value, sizeof(value)) == -1) {
		okay = FALSE;
		break;
	    }
	    if (!replaceTags(value, sizeof(value),
					cwd, instDir, sysDir, winDir)) {
		(*errorCB)("Internal error -- "
				"registry string too long in database.");
		return FALSE;
	    }

	    if (!addRegistryString(path, name, value, undoList)) {
		char buffer[2048];
		sprintf(buffer, "Can't create registry string value:\n%s\\%s",
							path, name);
		(*errorCB)(buffer);
		return FALSE;
	    }
	    break;
	  }

	  case INST_REGDWORD: {
	    (*fileCB)(0);
	    (*nameCB)("Registry...");

	    if (stream.getString(path, sizeof(path)) == -1) {
		okay = FALSE;
		break;
	    }

	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }

	    DWORD value = stream.getUInt32();
	    if (!addRegistryDWord(path, name, value, undoList)) {
		char buffer[2048];
		sprintf(buffer, "Can't create registry DWORD value:\n%s\\%s",
							path, name);
		(*errorCB)(buffer);
		return FALSE;
	    }
	    break;
	  }

	  case INST_ADDREADME:
	    (*fileCB)(0);
	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }
	    sprintf(path, "%s\\%s", cwd, name);
	    (*readmeCB)(path);
	    break;

	  case INST_ADDSHORTCUT: {
	    (*fileCB)(0);
	    (*nameCB)("Shortcut...");

	    // get name of target file
	    if (stream.getString(name, sizeof(name)) == -1) {
		okay = FALSE;
		break;
	    }
	    sprintf(path, "%s\\%s", cwd, name);

	    // get name of shortcut
	    char linkname[_MAX_FNAME + _MAX_EXT - 1];
	    if (stream.getString(linkname, sizeof(linkname)) == -1) {
		okay = FALSE;
		break;
	    }

	    // get arguments
	    char args[256];
	    if (stream.getString(args, sizeof(args)) == -1) {
		okay = FALSE;
		break;
	    }

	    // get working directory
	    char workdir[_MAX_PATH];
	    if (stream.getString(workdir, sizeof(workdir)) == -1) {
		okay = FALSE;
		break;
	    }

	    // replace special tags in args and working directory
	    if (!replaceTags(args, sizeof(args),
					cwd, instDir, sysDir, winDir)) {
		(*errorCB)("Internal error -- "
				"shortcut args too long in database.");
		return FALSE;
	    }
	    if (!replaceTags(workdir, sizeof(workdir),
					cwd, instDir, sysDir, winDir)) {
		(*errorCB)("Shortcut working directory too long.\n"
				"Choose a shorter installation folder name.");
		return FALSE;
	    }

	    if (!addDesktopLink(linkname, undoList, path, args, workdir)) {
		char buffer[2048];
		sprintf(buffer, "Can't create shortcut for:\n%s", path);
		(*errorCB)(buffer);
		return FALSE;
	    }
	    break;
	  }

	  default:
	    okay = FALSE;
	}
    }

    if (!okay) {
	(*fileCB)(0);
	(*totalCB)(0);
	(*errorCB)("Internal error -- corrupt database.");
	return FALSE;
    }
    else {
	(*fileCB)(0);

	// everything is okay so clear undo list
	undoList.clear();
    }

    return TRUE;
}
// ex: shiftwidth=2 tabstop=8
