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

/* UnixPlatformFactory:
 *	Factory for Unix platform stuff.
 */

#ifndef BZF_UNIX_PLATFORM_FACTORY_H
#define BZF_UNIX_PLATFORM_FACTORY_H

#include "PlatformFactory.h"
#include <signal.h>

/* some platforms don't have a SIG_PF type. */
#ifndef SIG_PF
typedef void (*SIG_PF)(int);
#endif

class UnixPlatformFactory : public PlatformFactory {
public:
	UnixPlatformFactory();
	~UnixPlatformFactory();

	istream*			createConfigInStream() const;
	ostream*			createConfigOutStream() const;
	void				createConsole();
	void				writeConsole(const char*, bool error);
	double				getTime() const;
	double				getClock() const;
	void				sleep(double timeInSeconds) const;
	BzfString			getUserName() const;
	void				setEnv(const BzfString&, const BzfString&);
	void				unsetEnv(const BzfString&);
	BzfString			getEnv(const BzfString&) const;
	void				signalRaise(Signal);

protected:
	SigType				signalInstall(Signal);
	SigType				signalInstallIgnore(Signal);
	SigType				signalInstallDefault(Signal);

private:
	UnixPlatformFactory(const UnixPlatformFactory&);
	UnixPlatformFactory& operator=(const UnixPlatformFactory&);

	BzfString			getConfigFileName() const;
	SigType				doSignalInstall(Signal, SIG_PF);
	static void			onSignal(int);
	static int			toSigno(Signal);
	static Signal		fromSigno(int);
};

#endif // BZF_UNIX_PLATFORM_FACTORY_H
