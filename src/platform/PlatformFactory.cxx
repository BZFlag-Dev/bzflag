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

#include "common.h"
#include "PlatformFactory.h"

PlatformFactory*		PlatformFactory::instance = NULL;

PlatformFactory::PlatformFactory()
{
	// do nothing
}

PlatformFactory::~PlatformFactory()
{
	// do nothing
}

PlatformFactory::SignalHandler
						PlatformFactory::signalCatch(
								Signal sig, SignalHandler func)
{
	// save old function
	SignalHandler oldFunc = signalHandlers[sig];

	// install signal
	SigType result;
	if (func == &signalIgnore)
		result = signalInstallIgnore(sig);
	else if (func == &signalDefault)
		result = signalInstallDefault(sig);
	else
		result = signalInstall(sig);

	// bail if we failed
	if (result == kSigError)
		return NULL;

	// save new function
	signalHandlers[sig] = func;

	// return old function
	return oldFunc;
}

void					PlatformFactory::signalIgnore(Signal)
{
	// do nothing -- this is never called;  only its address is used.
}

void					PlatformFactory::signalDefault(Signal)
{
	// do nothing -- this is never called;  only its address is used.
}

void					PlatformFactory::signalInit()
{
	// get currently installed signal handlers
	for (int sig = 0; sig < kSigLast; ++sig)
		switch (signalInstallDefault(static_cast<Signal>(sig))) {
			case kSigError:
			case kSigDefault:
				signalHandlers[sig] = &signalDefault;
				break;

			case kSigIgnore:
				signalHandlers[sig] = &signalIgnore;
				break;

			case kSigFunction:
				signalHandlers[sig] = NULL;
				break;
		}
}

void					PlatformFactory::signalForward(Signal signo)
{
	(*signalHandlers[signo])(signo);
}
