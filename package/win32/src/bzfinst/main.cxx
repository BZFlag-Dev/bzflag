/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
 * bzflag installer main app
 */

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <objbase.h>
#include <shlobj.h>
#include <string.h>
#include <direct.h>
#include <assert.h>
#include "resource.h"
#include "install.h"

enum { HELLO_PAGE, LICENSE_PAGE, DIRECTORY_PAGE, INSTALL_PAGE, FINISH_PAGE };
static HINSTANCE	hInstance = NULL;
static HWND		dialog = NULL;
static HWND		dialogPage = NULL;
static int		dialogPageID = -1;
static int		dialogID[] = {
				IDD_HELLO,
				IDD_LICENSE,
				IDD_DIRECTORY,
				IDD_INSTALL,
				IDD_FINISH
			};
static char*		instDirectory = NULL;
static char*		readmeName = NULL;
static char		appName[256];
static const char*	defaultDir = "C:\\Program Files\\";

static BOOL CALLBACK	dlgProc(HWND, UINT, WPARAM, LPARAM);

//
// win32 utility
//

static void		showError(const char* msg)
{
    MessageBox(dialog, msg, "", MB_OK | MB_ICONERROR | MB_APPLMODAL);
}

static void		setNameCB(const char* name)
{
    SetWindowText(GetDlgItem(dialogPage, IDC_FILENAME), name);
}

static void		setReadmeCB(const char* name)
{
    free(readmeName);
    readmeName = strdup(name);
}

static void		setFileMeterCB(int n)
{
    SendMessage(GetDlgItem(dialogPage, IDC_FILE_METER), PBM_SETPOS, n, 0);
}

static void		setTotalMeterCB(int n)
{
    SendMessage(GetDlgItem(dialogPage, IDC_TOTAL_METER), PBM_SETPOS, n, 0);
}

static int CALLBACK	dirBrowseCB(HWND hwnd, UINT msg,
				LPARAM lParam, LPARAM lpData)
{
    if (msg == BFFM_INITIALIZED) {
	SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

static char*		getDirectory(const char* orig)
{
    if (!orig) orig = "C:\\";

    LPMALLOC mallocObj;
    if (SHGetMalloc(&mallocObj) != NOERROR)
	return NULL;

    BROWSEINFO info;
    memset(&info, 0, sizeof(info));
    info.hwndOwner      = dialog;
    info.pidlRoot       = NULL;
    info.pszDisplayName = (LPSTR)malloc(MAX_PATH);
    info.lpszTitle      = "Choose an installation directory:";
    info.ulFlags        = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    info.lpfn           = dirBrowseCB;
    info.lParam         = (LPARAM)orig;

    LPITEMIDLIST pidl;
    if ((pidl = SHBrowseForFolder(&info)) == NULL) {
	delete[] info.pszDisplayName;
	return NULL;
    }

    if (::SHGetPathFromIDList(pidl, info.pszDisplayName) == NULL) {
	mallocObj->Free(pidl);
	mallocObj->Release();
	delete[] info.pszDisplayName;
	return NULL;
    }

    mallocObj->Free(pidl);
    mallocObj->Release();
    return info.pszDisplayName;
}

//
// page changing
//

static void		setPage(int page)
{
    assert(page >= HELLO_PAGE && page <= FINISH_PAGE);
    if (page == dialogPageID)
	return;

    dialogPageID = page;
    HWND newPage = CreateDialog(hInstance,
			MAKEINTRESOURCE(dialogID[page]), dialog,
			(DLGPROC)dlgProc);
    if (!newPage) {
	showError("Internal error -- can't switch page");
	return;
    }
    SetWindowLong(newPage, GWL_EXSTYLE,
		GetWindowLong(newPage, GWL_EXSTYLE) |
		WS_EX_CONTROLPARENT);

    RECT rect;
    GetClientRect(GetDlgItem(dialog, IDC_WORKSPACE), &rect);
    MapWindowPoints(GetDlgItem(dialog, IDC_WORKSPACE), dialog,
						(LPPOINT)&rect, 2);

    if (page == LICENSE_PAGE) {
	SendMessage(GetDlgItem(newPage, IDC_LICENSE), EM_SETSEL,
				(WPARAM)-1, (LPARAM)0);
    }

    MoveWindow(newPage, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top, FALSE);
    ShowWindow(newPage, SW_SHOW);

    if (dialogPage)
	DestroyWindow(dialogPage);
    dialogPage = newPage;
}

static void		pageBack(int page)
{
    page--;
    setPage(page);
}

static void		pageNext(int page)
{
    page++;

    if (page == DIRECTORY_PAGE + 1) {
	// parse the directory name
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(instDirectory, drive, dir, fname, ext);

	// if no drive specified then use the current drive
	if (drive[0] == 0) {
	    char* cwd = getcwd(NULL, 0);
	    _splitpath(cwd, drive, NULL, NULL, NULL);
	    free(cwd);
	}

	// check length
	if (strlen(drive) + strlen(dir) + strlen(fname) + strlen(ext) >= _MAX_PATH) {
	    showError("Folder name is too long.");
	    page--;
	    return;
	}

	// make base installation directory name.  append file and extension
	// to directory cos we're assuming this is a path to a directory.
	char instDir[_MAX_PATH];
	sprintf(instDir, "%s%s%s%s", drive, dir, fname, ext);

	// check for bogus characters in instDirectory (skip drive specifier)
	if (strpbrk(instDir + 2, "/:*?\"<>|") != 0) {
	    showError("Folder name cannot include:\n / : * ? \" < > |");
	    page--;
	    return;
	}

	free(instDirectory);
	instDirectory = strdup(instDir);
    }

    else if (page == INSTALL_PAGE + 1) {
	ShowWindow(GetDlgItem(dialogPage, IDC_INSTALLING), TRUE);
	ShowWindow(GetDlgItem(dialogPage, IDC_PREINSTALL), FALSE);
	if (!install(instDirectory, showError, setNameCB,
			setFileMeterCB, setTotalMeterCB,
			setReadmeCB)) {
	    ShowWindow(GetDlgItem(dialogPage, IDC_PREINSTALL), TRUE);
	    ShowWindow(GetDlgItem(dialogPage, IDC_INSTALLING), FALSE);
	    SetWindowText(GetDlgItem(dialogPage, IDC_FILENAME), "");
	    SendMessage(GetDlgItem(dialogPage, IDC_FILE_METER),
						PBM_SETPOS, 0, 0);
	    SendMessage(GetDlgItem(dialogPage, IDC_TOTAL_METER),
						PBM_SETPOS, 0, 0);
	    return;
	}
    }

    setPage(page);
}

//
// dialog procs
//

static BOOL CALLBACK	helloProc(HWND, UINT msg, WPARAM, LPARAM)
{
    switch (msg) {
      case WM_INITDIALOG:
	EnableWindow(GetDlgItem(dialog, IDC_BACK), FALSE);
	EnableWindow(GetDlgItem(dialog, IDC_NEXT), TRUE);
	SetWindowText(GetDlgItem(dialog, IDCANCEL), "Cancel");
	return TRUE;
    }
    return FALSE;
}

static BOOL CALLBACK	licenseProc(HWND hwnd, UINT msg, WPARAM, LPARAM)
{
    switch (msg) {
      case WM_INITDIALOG: {
	EnableWindow(GetDlgItem(dialog, IDC_BACK), TRUE);
	EnableWindow(GetDlgItem(dialog, IDC_NEXT), TRUE);
	SetWindowText(GetDlgItem(dialog, IDCANCEL), "Cancel");

	HRSRC licenseResource     = NULL;
	HGLOBAL licenseData       = NULL;
	const char* licenseString = NULL;
	licenseResource = FindResource(hInstance,
				MAKEINTRESOURCE(IDR_LICENSE),
				"rawstring");
	if (licenseResource)
	    licenseData = LoadResource(hInstance, licenseResource);
	if (licenseData)
	    licenseString = (const char*)LockResource(licenseData);
	if (licenseString)
	    SetWindowText(GetDlgItem(hwnd, IDC_LICENSE), licenseString);
	else
	    SetWindowText(GetDlgItem(hwnd, IDC_LICENSE),
			"ERROR -- License not found!");
	return TRUE;
      }
    }
    return FALSE;
}

static BOOL CALLBACK	directoryProc(HWND hwnd, UINT msg,
					WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
      case WM_INITDIALOG: {
	EnableWindow(GetDlgItem(dialog, IDC_BACK), TRUE);
	EnableWindow(GetDlgItem(dialog, IDC_NEXT), TRUE);
	SetWindowText(GetDlgItem(dialog, IDCANCEL), "Cancel");
	SetWindowText(GetDlgItem(hwnd, IDC_DIRECTORY), instDirectory);

	int requiredSpace = getRequiredSpace();
	const HWND spaceHwnd = GetDlgItem(hwnd, IDC_SPACE);
	char inBuffer[512], outBuffer[512];
	GetWindowText(spaceHwnd, inBuffer, sizeof(inBuffer));
	sprintf(outBuffer, inBuffer, requiredSpace / 1024);
	SetWindowText(spaceHwnd, outBuffer);
	return TRUE;
      }

      case WM_COMMAND:
	switch (LOWORD(wParam)) {
	  case IDC_BROWSE: {
	    char* newDir = getDirectory(instDirectory);
	    if (newDir) {
		free(instDirectory);
		instDirectory = newDir;
		SetWindowText(GetDlgItem(hwnd, IDC_DIRECTORY),
							instDirectory);
	    }
	    return TRUE;
	  }

	  case IDC_DIRECTORY:
	    if (HIWORD(wParam) == EN_CHANGE) {
		HWND ctrl = (HWND)lParam;
		int n = GetWindowTextLength(ctrl);
		free(instDirectory);
		instDirectory = (char*)malloc(n + 1);
		GetWindowText(ctrl, instDirectory, n + 1);
	    }
	    return FALSE;
	}
    }
    return FALSE;
}

static BOOL CALLBACK	installProc(HWND hwnd, UINT msg, WPARAM, LPARAM)
{
    switch (msg) {
      case WM_INITDIALOG: {
	EnableWindow(GetDlgItem(dialog, IDC_BACK), TRUE);
	EnableWindow(GetDlgItem(dialog, IDC_NEXT), TRUE);
	SetWindowText(GetDlgItem(dialog, IDCANCEL), "Cancel");

	HWND filename, fileProgress, totalProgress;
	filename = GetDlgItem(hwnd, IDC_FILENAME);
	fileProgress = GetDlgItem(hwnd, IDC_FILE_METER);
	totalProgress = GetDlgItem(hwnd, IDC_TOTAL_METER);
	SetWindowText(filename, "");
	SendMessage(fileProgress,  PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SendMessage(fileProgress,  PBM_SETPOS,   0, 0);
	SendMessage(totalProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SendMessage(totalProgress, PBM_SETPOS,   0, 0);
	return TRUE;
      }
    }
    return FALSE;
}

static BOOL CALLBACK	finishProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM)
{
    switch (msg) {
      case WM_INITDIALOG:
	EnableWindow(GetDlgItem(dialog, IDC_BACK), FALSE);
	EnableWindow(GetDlgItem(dialog, IDC_NEXT), FALSE);
	SetWindowText(GetDlgItem(dialog, IDCANCEL), "Quit");
	ShowWindow(GetDlgItem(hwnd, IDC_README),
			readmeName ? SW_SHOW : SW_HIDE);
	SendMessage(GetDlgItem(hwnd, IDC_README), BM_SETCHECK,
			readmeName ? 1 : 0, 0);
	return TRUE;

      case WM_COMMAND:
	switch (LOWORD(wParam)) {
	  case IDCANCEL:
	    if (readmeName &&
		SendMessage(GetDlgItem(dialogPage, IDC_README),
						BM_GETCHECK, 0, 0)) {
		ShellExecute(NULL, "open", readmeName,
				NULL, NULL, SW_SHOWNORMAL);
	    }
	    PostQuitMessage(0);
	    return TRUE;
	}
    }
    return FALSE;
}

static BOOL CALLBACK	dlgProc(HWND hwnd, UINT msg,
					WPARAM wParam, LPARAM lParam)
{
    int result = 0;

    switch (dialogPageID) {
      case HELLO_PAGE:
	result = helloProc(hwnd, msg, wParam, lParam);
	break;

      case LICENSE_PAGE:
	result = licenseProc(hwnd, msg, wParam, lParam);
	break;

      case DIRECTORY_PAGE:
	result = directoryProc(hwnd, msg, wParam, lParam);
	break;

      case INSTALL_PAGE:
	result = installProc(hwnd, msg, wParam, lParam);
	break;

      case FINISH_PAGE:
	result = finishProc(hwnd, msg, wParam, lParam);
	break;
    }
    if (result) return result;

    switch (msg) {
      case WM_COMMAND:
	switch (LOWORD(wParam)) {
	  case IDCANCEL:
	    if (IDOK == MessageBox(hwnd,
			"You haven't finished installing.\n"
			"Really quit?",
			"Quit?",
			MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL))
	    PostQuitMessage(0);
	    return TRUE;

	  case IDC_BACK:
	    pageBack(dialogPageID);
	    return TRUE;

	  case IDOK:
	  case IDC_NEXT:
	    pageNext(dialogPageID);
	    return TRUE;
	}
	break;

      case WM_INITDIALOG:
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
    CoInitialize(NULL);
    InitCommonControls();
    hInstance = hInst;

    // load name of app to install
    if (LoadString(hInstance, IDS_INSTALL_NAME,
				appName, sizeof(appName) / sizeof(appName[0])) == 0) {
	MessageBox(NULL, "Error loading install name.  Exiting.",
			"Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
	return 1;
    }

    // make default install directory
    instDirectory = (char*)malloc(strlen(defaultDir) + strlen(appName) + 1);
    strcpy(instDirectory, defaultDir);
    strcat(instDirectory, appName);

    // load dialog
    dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN),
						NULL, (DLGPROC)dlgProc);
    pageBack(HELLO_PAGE + 1);

    // change the title
    char titleFormat[256];
    if (LoadString(hInstance, IDS_TITLE, titleFormat,
				sizeof(titleFormat) / sizeof(titleFormat[0]))) {
	char title[256];
	sprintf(title, titleFormat, appName);
	SetWindowText(dialog, title);
    }

    // center it and show it
    RECT rect;
    GetWindowRect(dialog, &rect);
    const int w = rect.right - rect.left;
    const int h = rect.bottom - rect.top;
    MoveWindow(dialog,
		(GetSystemMetrics(SM_CXSCREEN) - w) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - h) / 2, w, h, FALSE);
    ShowWindow(dialog, iCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
	if (!IsDialogMessage(dialog, &msg)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }

    DestroyWindow(dialogPage);
    DestroyWindow(dialog);
    free(instDirectory);
    free(readmeName);
    CoUninitialize();

    return msg.wParam;
}
