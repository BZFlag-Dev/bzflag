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

/* WinPlatformFactory:
 *	Factory for Windows platform stuff.
 */

#ifndef BZF_WIN_PLATFORM_FACTORY_H
#define BZF_WIN_PLATFORM_FACTORY_H

#include "PlatformFactory.h"
#include <windows.h>

typedef void (__cdecl *SIG_PF)(int);

class WinPlatformFactory : public PlatformFactory {
public:
	WinPlatformFactory();
	~WinPlatformFactory();

	std::istream*			createConfigInStream() const;
	std::ostream*			createConfigOutStream() const;
	void				createConsole();
	void				writeConsole(const char*, bool error);
	double				getTime() const;
	double				getClock() const;
	void				sleep(double timeInSeconds) const;
	std::string			getUserName() const;
	void				setEnv(const std::string&, const std::string&);
	void				unsetEnv(const std::string&);
	std::string			getEnv(const std::string&) const;
	void				signalRaise(Signal);

protected:
	SigType				signalInstall(Signal);
	SigType				signalInstallIgnore(Signal);
	SigType				signalInstallDefault(Signal);

private:
	WinPlatformFactory(const WinPlatformFactory&);
	WinPlatformFactory&	operator=(const WinPlatformFactory&);

	std::string			getConfigFileName() const;
	SigType				doSignalInstall(Signal, SIG_PF);
	static void			onSignal(int);
	static int			toSigno(Signal);
	static Signal		fromSigno(int);

private:
	unsigned int		clockZero;
	LARGE_INTEGER		qpcZero;
	double				qpcFrequency;
	bool				hasConsole;
};

#endif // BZF_WIN_PLATFORM_FACTORY_H
