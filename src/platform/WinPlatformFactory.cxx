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

#include "WinPlatformFactory.h"
#include "FileManager.h"
#include <time.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

PlatformFactory*		PlatformFactory::getInstance()
{
	if (instance == NULL)
		instance = new WinPlatformFactory;
	return instance;
}

WinPlatformFactory::WinPlatformFactory() : hasConsole(false)
{
	// initialize signals
	signalInit();

	// see if we can use the performance counter.  use multimedia timer
	// if we can't.
	LARGE_INTEGER freq;
	if (QueryPerformanceFrequency(&freq)) {
		qpcFrequency = 1.0 / (double)freq.QuadPart;
		QueryPerformanceCounter(&qpcZero);
		clockZero = 0;
	}
	else {
		qpcFrequency = 0.0;
		clockZero    = (unsigned int)timeGetTime();
	}
}

WinPlatformFactory::~WinPlatformFactory()
{
	// do nothing
}

istream*				WinPlatformFactory::createConfigInStream() const
{
	return FILEMGR->createDataInStream(getConfigFileName());
}

ostream*				WinPlatformFactory::createConfigOutStream() const
{
	return FILEMGR->createDataOutStream(getConfigFileName());
}

void					WinPlatformFactory::createConsole()
{
	// ignore if already created
	if (hasConsole)
		return;

	// open a console
	if (!AllocConsole())
		return;
	HANDLE herr = GetStdHandle(STD_ERROR_HANDLE);

	// prep console
	COORD size = { 80, 1000 };
	SetConsoleScreenBufferSize(herr, size);

/*
	// reopen stderr
	if (!freopen("con", "w", stderr))
		return;
*/

	// reopen stdout to point at console
	freopen("con", "w", stdout);
	hasConsole = true;
}

void					WinPlatformFactory::writeConsole(
							const char* msg, bool error)
{
	// ignore is no console or no message
	if (!hasConsole || msg == NULL)
		return;

	// check msg for backspaces.  ignore the message if there are any.
	if (strchr(msg, '\b') != NULL)
		return;

	// write message
	DWORD n;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE herr = GetStdHandle(STD_ERROR_HANDLE);
	GetConsoleScreenBufferInfo(herr, &info);
	if (error)
		SetConsoleTextAttribute(herr,
							FOREGROUND_RED |
							FOREGROUND_GREEN |
							FOREGROUND_INTENSITY);
	else
		SetConsoleTextAttribute(herr,
							FOREGROUND_RED |
							FOREGROUND_GREEN |
							FOREGROUND_BLUE);
	WriteConsole(herr, msg, strlen(msg), &n, NULL);
	SetConsoleTextAttribute(herr, info.wAttributes);
}

double					WinPlatformFactory::getTime() const
{
	return time(NULL);
}

double					WinPlatformFactory::getClock() const
{
	if (qpcFrequency != 0.0) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return qpcFrequency * (double)(now.QuadPart - qpcZero.QuadPart);
	}
	else {
		unsigned int now = (unsigned int)timeGetTime();
		return 1.0e-3 * (now - clockZero);
	}
}

void					WinPlatformFactory::sleep(double t) const
{
	Sleep((DWORD)(1000.0 * t));
}

BzfString				WinPlatformFactory::getUserName() const
{
	char username[256];
	DWORD usernameLen = sizeof(username);
	GetUserName(username, &usernameLen);
	if (*username)
		return BzfString(username);
	else
		return "windows_user";
}

void					WinPlatformFactory::setEnv(
								const BzfString& name, const BzfString& value)
{
	putenv(BzfString::format("%s=%s", name.c_str(), value.c_str()).c_str());
}

void					WinPlatformFactory::unsetEnv(const BzfString& name)
{
	putenv(name.c_str());
}

BzfString				WinPlatformFactory::getEnv(const BzfString& name) const
{
	const char* value = getenv(name.c_str());
	return BzfString(value != NULL ? value : "");
}

void					WinPlatformFactory::signalRaise(Signal sig)
{
	int signo = toSigno(sig);
	if (signo != 0)
		raise(signo);
}

PlatformFactory::SigType
						WinPlatformFactory::signalInstall(Signal sig)
{
	return doSignalInstall(sig, onSignal);
}

PlatformFactory::SigType
						WinPlatformFactory::signalInstallIgnore(Signal sig)
{
	return doSignalInstall(sig, SIG_IGN);
}

PlatformFactory::SigType
						WinPlatformFactory::signalInstallDefault(Signal sig)
{
	return doSignalInstall(sig, SIG_DFL);
}

PlatformFactory::SigType
						WinPlatformFactory::doSignalInstall(
								Signal sig, SIG_PF func)
{
	const int signo = toSigno(sig);

	// ignore signals the platform doesn't support
	if (signo == 0)
		return kSigDefault;

	// change signal handler
	func = signal(signo, func);

	// translate function pointer
	if (func == SIG_ERR)
		return kSigError;
	else if (func == SIG_IGN)
		return kSigIgnore;
	else if (func == SIG_DFL)
		return kSigDefault;
	else
		return kSigFunction;
}

void					WinPlatformFactory::onSignal(int signo)
{
	// reinstall handler
	signal(signo, onSignal);

	// forward the signal
	PLATFORM->signalForward(fromSigno(signo));
}

int						WinPlatformFactory::toSigno(Signal sig)
{
	switch (sig) {
		case kSigINT:
			return SIGINT;

		case kSigILL:
			return SIGILL;

		case kSigABRT:
			return SIGABRT;

		case kSigSEGV:
			return SIGSEGV;

		case kSigTERM:
			return SIGTERM;

		default:
			return 0;
	}
}

Signal					WinPlatformFactory::fromSigno(int signo)
{
	switch (signo) {
		case SIGINT:
			return kSigINT;

		case SIGILL:
			return kSigILL;

		case SIGABRT:
			return kSigABRT;

		case SIGSEGV:
			return kSigSEGV;

		case SIGTERM:
			return kSigTERM;

		default:
			return kSigNone;
	}
}

BzfString				WinPlatformFactory::getConfigFileName() const
{
	// get location of personal files from system.  this appears to be
	// the closest thing to a home directory on windows.  use root of
	// C drive as a default in case we can't get the path or it doesn't
	// exist.
	BzfString name("C:");
	char dir[MAX_PATH];
	ITEMIDLIST* idl;
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
		if (SHGetPathFromIDList(idl, dir)) {
			DWORD attr = GetFileAttributes(dir);
			if (attr != 0xffffffff && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
				name = dir;
		}

		IMalloc* shalloc;
		if (SUCCEEDED(SHGetMalloc(&shalloc))) {
			shalloc->Free(idl);
			shalloc->Release();
		}
	}

	// append the config file name
	return FILEMGR->catPath(name, "bzflag.bzc");
}
