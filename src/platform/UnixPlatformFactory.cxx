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

#include "UnixPlatformFactory.h"
#include "StateDatabase.h"
#include "FileManager.h"
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#if !defined(NO_UNIX_GET_INSTANCE)
PlatformFactory*		PlatformFactory::getInstance()
{
	if (instance == NULL)
		instance = new UnixPlatformFactory;
	return instance;
}
#endif

UnixPlatformFactory::UnixPlatformFactory()
{
	// initialize signals
	signalInit();
}

UnixPlatformFactory::~UnixPlatformFactory()
{
	// do nothing
}

istream*				UnixPlatformFactory::createConfigInStream() const
{
	istream* stream   = NULL;
	BzfString filename = getConfigFileName();

	// try host specific name first
	if (stream == NULL && getenv("HOST")) {
		// construct host specific name
		BzfString filename2 = filename;
		filename2 += ".";
		filename2 += getenv("HOST");

		stream = FILEMGR->createDataInStream(filename2);
	}

	// try generic name
	if (stream == NULL)
		stream = FILEMGR->createDataInStream(filename);

	return stream;
}

ostream*				UnixPlatformFactory::createConfigOutStream() const
{
	ostream* stream   = NULL;
	BzfString filename = getConfigFileName();

	// try host specific name first
	if (stream == NULL && getenv("HOST")) {
		// construct host specific name
		BzfString filename2 = filename;
		filename2 += ".";
		filename2 += getenv("HOST");

		stream = FILEMGR->createDataOutStream(filename2);
	}

	// try generic name
	if (stream == NULL)
		stream = FILEMGR->createDataOutStream(filename);

	return stream;
}

void					UnixPlatformFactory::createConsole()
{
	// do nothing -- unix has a console
}

void					UnixPlatformFactory::writeConsole(
							const char* msg, bool error)
{
	fprintf(error ? stderr : stdout, "%s", msg);
	fflush(error ? stderr : stdout);
}

double					UnixPlatformFactory::getTime() const
{
	return time(NULL);
}

double					UnixPlatformFactory::getClock() const
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return static_cast<double>(now.tv_sec) +
				1.0e-6 * static_cast<double>(now.tv_usec);
}

void					UnixPlatformFactory::sleep(double t) const
{
	struct timeval tv;
	tv.tv_sec  = (long)t;
	tv.tv_usec = (long)(1.0e6 * (t - floor(t)));
	select(0, NULL, NULL, NULL, &tv);
}

BzfString				UnixPlatformFactory::getUserName() const
{
	struct passwd* pwent = getpwuid(getuid());
	return BzfString((pwent != NULL) ? pwent->pw_name : "unix_user");
}

void					UnixPlatformFactory::setEnv(
								const BzfString& name, const BzfString& value)
{
#if !defined(__linux__)
	putenv(BzfString::format("%s=%s", name.c_str(), value.c_str()).c_str());
#else
	setenv(name.c_str(), value.c_str(), 1);
#endif
}

void					UnixPlatformFactory::unsetEnv(const BzfString& name)
{
#if !defined(__linux__)
	putenv(name.c_str());
#else
	unsetenv(name.c_str());
#endif
}

BzfString				UnixPlatformFactory::getEnv(const BzfString& name) const
{
	const char* value = getenv(name.c_str());
	return BzfString(value != NULL ? value : "");
}

void					UnixPlatformFactory::signalRaise(Signal sig)
{
	int signo = toSigno(sig);
	if (signo != 0)
		raise(signo);
}

PlatformFactory::SigType
						UnixPlatformFactory::signalInstall(Signal sig)
{
	return doSignalInstall(sig, onSignal);
}

PlatformFactory::SigType
						UnixPlatformFactory::signalInstallIgnore(Signal sig)
{
	return doSignalInstall(sig, SIG_IGN);
}

PlatformFactory::SigType
						UnixPlatformFactory::signalInstallDefault(Signal sig)
{
	return doSignalInstall(sig, SIG_DFL);
}

PlatformFactory::SigType
						UnixPlatformFactory::doSignalInstall(
								Signal sig, SIG_PF func)
{
	struct sigaction act, oact;
	const int signo = toSigno(sig);

	// ignore signals the platform doesn't support
	if (signo == 0)
		return kSigDefault;

	// prepare to change signal handler
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
#ifdef SA_NODEFER
	act.sa_flags = SA_NODEFER;
#else
	act.sa_flags = 0;
#endif
	if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
		/* SunOS 4.x */
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else {
#ifdef SA_RESTART
		/* SVR4, 4.4BSD */
		act.sa_flags |= SA_RESTART;
#endif
	}

	// change signal handler
	if (sigaction(signo, &act, &oact) < 0)
		return kSigError;

	// translate function pointer
	if (oact.sa_handler == SIG_IGN)
		return kSigIgnore;
	else if (oact.sa_handler == SIG_DFL)
		return kSigDefault;
	else
		return kSigFunction;
}

void					UnixPlatformFactory::onSignal(int signo)
{
	PLATFORM->signalForward(fromSigno(signo));
}

int						UnixPlatformFactory::toSigno(Signal sig)
{
	// enums chosen to match Unix signals
	return sig;
}

Signal					UnixPlatformFactory::fromSigno(int signo)
{
	// enums chosen to match Unix signals
	return static_cast<Signal>(signo);
}

BzfString				UnixPlatformFactory::getConfigFileName() const
{
	struct passwd* pwent = getpwuid(getuid());
	BzfString homeDir((pwent != NULL && pwent->pw_dir != NULL) ?
								pwent->pw_dir : "");
	return FILEMGR->catPath(homeDir, ".bzflag");
}
