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

/* PlatformFactory:
 *	Abstract builder for (non-audio/video) platform dependent stuff.
 */

#ifndef BZF_PLATFORM_FACTORY_H
#define BZF_PLATFORM_FACTORY_H

#include "common.h"
#include "BzfString.h"
#include "bzfio.h"

#define PLATFORM (PlatformFactory::getInstance())

enum Signal {
    kSigNone = 0,
    kSigHUP  = 1,						// SIGHUP
    kSigINT  = 2,						// SIGINT
    kSigQUIT = 3,						// SIGQUIT
    kSigILL  = 4,						// SIGILL
    kSigABRT = 6,						// SIGABRT
    kSigBUS  = 7,						// SIGBUS
    kSigUSR1 = 10,						// SIGUSR1
    kSigSEGV = 11,						// SIGSEGV
    kSigUSR2 = 12,						// SIGUSR2
    kSigPIPE = 13,						// SIGPIPE
    kSigALRM = 14,						// SIGALRM
    kSigTERM = 15,						// SIGTERM
    kSigLast
};
#define kSigIGN &PlatformFactory::signalIgnore
#define kSigDFL &PlatformFactory::signalDefault

class PlatformFactory {
public:
	typedef void (*SignalHandler)(Signal);

	PlatformFactory();
	virtual ~PlatformFactory();

	// create input/output stream for configuration file (wherever
	// it's supposed to go)
	virtual istream*	createConfigInStream() const = 0;
	virtual ostream*	createConfigOutStream() const = 0;

	// create a console for text output and write to it (either
	// normal or error text).  if possible stdout and stderr
	// should also be redirected to the console.
	virtual void		createConsole() = 0;
	virtual void		writeConsole(const char*, bool error = false) = 0;

	// get the current time.  the time is in seconds since midnight
	// 00:00:00 UTC 1/1/1970.  precision should be at least seconds
	// but may be higher.
	virtual double		getTime() const = 0;

	// get the application clock.  the clock has an arbitrary zero
	// so it's only useful for computing time intervals.  the clock
	// should have as great a precision as possible.
	virtual double		getClock() const = 0;

	// sleep for the given number of seconds
	virtual void		sleep(double timeInSeconds) const = 0;

	// get the current user name (if possible)
	virtual BzfString	getUserName() const = 0;

	// get/set/unset environment variables
	virtual void		setEnv(const BzfString&, const BzfString&) = 0;
	virtual void		unsetEnv(const BzfString&) = 0;
	virtual BzfString	getEnv(const BzfString&) const = 0;

	// add/remove handlers for signals.  note that some platforms have
	// severe restrictions on what signal handlers can do so take care.
	// returns the previous handler.
	SignalHandler		signalCatch(Signal, SignalHandler);

	// raise a signal
	virtual void		signalRaise(Signal) = 0;

	// get the singleton instance
	static PlatformFactory*		getInstance();

	// dummy functions used to indicate that a signal should be
	// ignored or do its default action.
	static void			signalIgnore(Signal);
	static void			signalDefault(Signal);

	// used by subclasses
	void				signalForward(Signal);

	// not protected to work around compiler bug in VC++
	enum SigType { kSigError, kSigFunction, kSigIgnore, kSigDefault };

protected:
	virtual SigType		signalInstall(Signal) = 0;
	virtual SigType		signalInstallIgnore(Signal) = 0;
	virtual SigType		signalInstallDefault(Signal) = 0;
	void				signalInit();

private:
	PlatformFactory(const PlatformFactory&);
	PlatformFactory&	operator=(const PlatformFactory&);

private:
	SignalHandler		signalHandlers[kSigLast];

	static PlatformFactory*		instance;
};

#endif // BZF_PLATFORM_FACTORY_H
