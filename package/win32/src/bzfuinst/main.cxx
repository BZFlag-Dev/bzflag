/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
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
 * bzflag uninstaller main app
 */

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <assert.h>
#include "resource.h"

enum Action { RMFILE, RMDIR, RMKEY, RMVALUE, RMLINK };
struct Item {
  public:
    Action		action;
    const char*		str1;
    const char*		str2;
};

static HINSTANCE	hInstance = NULL;
static HWND		dialog = NULL;

// FIXME -- get these from bzflag.spc
static Item		items[] =
{
RMVALUE, "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\bzflag", "UninstallString",
RMVALUE, "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\bzflag", "DisplayName",
RMKEY,   "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", "bzflag",
RMFILE,  "%i\\doc\\README.HTM", "",
RMFILE,  "%i\\doc\\LICENSE", "",
RMDIR,   "%i\\doc", "",
RMFILE,  "%i\\data\\laser.wav", "",
RMFILE,  "%i\\data\\bbolt.rgb", "",
RMFILE,  "%i\\data\\bbolt.rgb", "",
RMFILE,  "%i\\data\\boxwall.rgb", "",
RMFILE,  "%i\\data\\caution.rgb", "",
RMFILE,  "%i\\data\\clouds.rgb", "",
RMFILE,  "%i\\data\\explode1.rgb", "",
RMFILE,  "%i\\data\\fixedbr.rgb", "",
RMFILE,  "%i\\data\\fixedmr.rgb", "",
RMFILE,  "%i\\data\\flag.rgb", "",
RMFILE,  "%i\\data\\flage.rgb", "",
RMFILE,  "%i\\data\\gbolt.rgb", "",
RMFILE,  "%i\\data\\ground.rgb", "",
RMFILE,  "%i\\data\\helvbi.rgb", "",
RMFILE,  "%i\\data\\helvbr.rgb", "",
RMFILE,  "%i\\data\\laser.rgb", "",
RMFILE,  "%i\\data\\missile.rgb", "",
RMFILE,  "%i\\data\\mountain.rgb", "",
RMFILE,  "%i\\data\\panel.rgb", "",
RMFILE,  "%i\\data\\pbolt.rgb", "",
RMFILE,  "%i\\data\\pyrwall.rgb", "",
RMFILE,  "%i\\data\\rbolt.rgb", "",
RMFILE,  "%i\\data\\roof.rgb", "",
RMFILE,  "%i\\data\\timesbi.rgb", "",
RMFILE,  "%i\\data\\timesbr.rgb", "",
RMFILE,  "%i\\data\\title.rgb", "",
RMFILE,  "%i\\data\\wall.rgb", "",
RMFILE,  "%i\\data\\ybolt.rgb", "",
RMFILE,  "%i\\data\\boom.wav", "",
RMFILE,  "%i\\data\\explosion.wav", "",
RMFILE,  "%i\\data\\fire.wav", "",
RMFILE,  "%i\\data\\flag_alert.wav", "",
RMFILE,  "%i\\data\\flag_drop.wav", "",
RMFILE,  "%i\\data\\flag_grab.wav", "",
RMFILE,  "%i\\data\\flag_lost.wav", "",
RMFILE,  "%i\\data\\flag_won.wav", "",
RMFILE,  "%i\\data\\laser.wav", "",
RMFILE,  "%i\\data\\pop.wav", "",
RMFILE,  "%i\\data\\ricochet.wav", "",
RMFILE,  "%i\\data\\shock.wav", "",
RMFILE,  "%i\\data\\teleport.wav", "",
RMDIR,   "%i\\data", "",
RMLINK,  "bzflag", "",
RMFILE,  "%i\\bzflag.exe", "",
RMFILE,  "%i\\bzfs.exe", "",
RMFILE,  "%i\\bzfuinst.exe", "",
RMDIR,   "%i", ""
};

static BOOL CALLBACK	dlgProc(HWND, UINT, WPARAM, LPARAM);

//
// win32 utility
//

static void		showError(const char* msg)
{
    MessageBox(dialog, msg, "", MB_OK | MB_ICONERROR | MB_APPLMODAL);
}

static void		setTotalMeterCB(int n)
{
    SendMessage(GetDlgItem(dialog, IDC_TOTAL_METER), PBM_SETPOS, n, 0);
}

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

static BOOL		replaceTags(char* args, int maxLen,
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

//
// dialog procs
//

static BOOL CALLBACK	dlgProc(HWND hwnd, UINT msg,
					WPARAM wParam, LPARAM lParam)
{
    int result = 0;

    switch (msg) {
      case WM_COMMAND:
	switch (LOWORD(wParam)) {
	  case IDOK:
	    PostQuitMessage(0);
	    return TRUE;
	}
	break;

      case WM_INITDIALOG:
	EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
	return TRUE;
    }

    return FALSE;
}

//
// message loop
//

int			doMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
	if (msg.message == WM_QUIT)
	    return 1;
	if (GetMessage(&msg, NULL, 0, 0)) {
	    if (!IsDialogMessage(dialog, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	}
    }
    return 0;
}

//
// main
//

int WINAPI		WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance,
				PSTR szCmdLine, int iCmdShow)
{
    InitCommonControls();
    hInstance = hInst;

    dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN),
						NULL, (DLGPROC)dlgProc);

    RECT rect;
    GetWindowRect(dialog, &rect);
    const int w = rect.right - rect.left;
    const int h = rect.bottom - rect.top;
    MoveWindow(dialog,
		(GetSystemMetrics(SM_CXSCREEN) - w) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - h) / 2, w, h, FALSE);
    ShowWindow(dialog, iCmdShow);
    UpdateWindow(dialog);

    // get important directories
    static const char* desktop = "\\Desktop\\";
    static const char* linkext = ".lnk";
    char sysDir[_MAX_PATH], winDir[_MAX_PATH];
    char instDir[_MAX_PATH], linkDir[_MAX_PATH];
    GetSystemDirectory(sysDir, sizeof(sysDir));
    GetWindowsDirectory(winDir, sizeof(winDir));
    strncpy(instDir, szCmdLine, sizeof(instDir) - 1);
    instDir[sizeof(instDir)] = '\0';
    const char* profile = getenv("USERPROFILE");
    if (!profile)
	profile = winDir;
    if (profile)
	sprintf(linkDir, "%s%s", profile, desktop);
    else
	linkDir[0] = '\0';

    const int n = sizeof(items) / sizeof(items[0]);
    setTotalMeterCB(0);
    for (int i = 0; i < n; i++) {
	switch (items[i].action) {
	  case RMFILE: {
	    char path[2 * _MAX_PATH];
	    sprintf(path, "%s", items[i].str1);
	    if (replaceTags(path, sizeof(path), instDir, sysDir, winDir))
		unlink(path);
	    break;
	  }

	  case RMDIR: {
	    char path[2 * _MAX_PATH];
	    sprintf(path, "%s", items[i].str1);
	    if (replaceTags(path, sizeof(path), instDir, sysDir, winDir))
		rmdir(path);
	    break;
	  }

	  case RMKEY: {
	    HKEY key = findKey(items[i].str1);
	    if (key) {
		RegDeleteKey(key, items[i].str2);
		RegCloseKey(key);
	    }
	    break;
	  }

	  case RMVALUE: {
	    HKEY key = findKey(items[i].str1);
	    if (key) {
		RegDeleteValue(key, items[i].str2);
		RegCloseKey(key);
	    }
	    break;
	  }

	  case RMLINK:
	    if (linkDir[0]) {
		char path[2 * _MAX_PATH];
		sprintf(path, "%s%s%s", linkDir, items[i].str1, linkext);
		unlink(path);
	    }
	    break;
	}
	setTotalMeterCB(100 * i / n);
	doMessages();
    }
    setTotalMeterCB(100);
    EnableWindow(GetDlgItem(dialog, IDOK), TRUE);
    SetFocus(GetDlgItem(dialog, IDOK));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
	if (!IsDialogMessage(dialog, &msg)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }

    DestroyWindow(dialog);

    return msg.wParam;
}
