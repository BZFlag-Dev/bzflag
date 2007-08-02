/* libIRC
* Copyright (c) 2004 Christopher Sean Morrison
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// implementation of main libIRC classes

//********************************************************************************//
//															version info																			//
//********************************************************************************//

#define _MAJOR_VERS	0
#define _MINOR_VERS	1
#define _REVISION	4

//********************************************************************************//

#include "libIRC.h"

#ifndef _WIN32
	#include <unistd.h>
#else
	#include <windows.h>
	#include <time.h>
	#include <stdio.h>
#endif

// sleep util
void IRCOSSleep ( float fTime )
{
#ifdef _WIN32
	Sleep((DWORD)(1000.0f * fTime));
#else
	usleep((unsigned int )(100000 * fTime));
#endif
}


std::string getTimeStamp ( void )
{
	std::string timeString;

#ifdef _WIN32
	struct tm *newtime;
	time_t aclock;

	time( &aclock );   // Get time in seconds
	newtime = localtime( &aclock );   // Convert time to struct tm form 

	/* Print local time as a string */
	timeString = asctime( newtime );
#endif//_WIN32

	return timeString;
}


std::string getLibVersion ( void )
{
	return string_util::format("libIRC %d.%d.%d",_MAJOR_VERS, _MINOR_VERS, _REVISION);
}

void getLibVersion ( int &major, int &minor, int &rev )
{
	major = _MAJOR_VERS;
	minor = _MINOR_VERS;
	rev = _REVISION;
}

// base message class

BaseIRCCommandInfo::BaseIRCCommandInfo()
{
	type = eUnknown;
	command = "NULL";
}

BaseIRCCommandInfo::~BaseIRCCommandInfo()
{
}

void BaseIRCCommandInfo::parse ( std::string line )
{
	params = string_util::tokenize(line,std::string(" "));
	raw = line;
	prefixed = line.c_str()[0] ==':';
	if (prefixed)
	{
		params[0].erase(params[0].begin());
		source = params[0];
		// pull off the source
		params.erase(params.begin());
	}
	else
		source = "HOST";

	// make sure we have a command
	if (params.size() > 0)
	{
		command = params[0];
		// pull off the command
		params.erase(params.begin());
	}
	else
		command = "NULL";

	// make sure we have a target
	if (params.size() > 0)
	{
		target = params[0];
		// pull off the command
		params.erase(params.begin());

		// pull off the :
		if (target.c_str()[0] == ':')
			target.erase(target.begin());
	}
	else
		target = "NULL";
}

std::string BaseIRCCommandInfo::getAsString ( int start, int end )
{
	return string_util::getStringFromList(params," ",start,end);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
