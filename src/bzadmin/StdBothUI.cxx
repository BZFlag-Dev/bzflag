/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "StdBothUI.h"

/* system implementation headers */
#include <iostream>

#ifdef _WIN32
	#include <stdlib.h>
	#include <wincon.h>
#else 
	#include <sys/types.h>
	#include <sys/select.h>
#endif 

/* implementation headers */
#include "global.h"

#ifdef _WIN32
unsigned long __stdcall winInput( void *that )
{
	StdBothUI *input = ( StdBothUI* )that;
	unsigned long numRead;

	while( WaitForSingleObject( input->processedEvent, INFINITE ) == WAIT_OBJECT_0 )
	{
		numRead = 0;
		ReadFile( input->console, &input->buffer[input->pos], MessageLen - input->pos, &numRead, NULL );
		if( numRead > 0 )
		{
			input->pos += numRead;
			SetEvent( input->readEvent );
		}
	}
	return 0;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

#endif 

// add this UI to the map
UIAdder StdBothUI::uiAdder( "stdboth", &StdBothUI::creator );

StdBothUI::StdBothUI( BZAdminClient &c ): BZAdminUI( c ), atEOF( false )
{
#ifdef _WIN32
	unsigned long tid;
	console = GetStdHandle( STD_INPUT_HANDLE );
	readEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	processedEvent = CreateEvent( NULL, FALSE, TRUE, NULL );
	thread = CreateThread( NULL, 0, winInput, this, 0, &tid );
	pos = 0;
#endif 
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void StdBothUI::outputMessage( const std::string &msg, ColorCode )
{
	std::cout << msg << std::endl;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


#ifdef _WIN32

bool StdBothUI::checkCommand( std::string &str )
{
	if( WaitForSingleObject( readEvent, 100 ) == WAIT_OBJECT_0 )
	{
		if( pos > 2 )
		{
			if(( buffer[pos - 1] == '\n' ) || ( buffer[pos - 1] == '\r' ) || ( pos == MessageLen ))
			{
				buffer[pos - 2] = '\0';
				str = buffer;
				pos = 0;
				SetEvent( processedEvent );
				return true;
			}
		}
		SetEvent( processedEvent );
	}
	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

#else 

bool StdBothUI::checkCommand( std::string &str )
{
	// if we read EOF last time, quit now
	if( atEOF )
	{
		str = "/quit";
		return true;
	}

	static char buffer[MessageLen + 1];
	static int pos = 0;

	fd_set rfds;
	timeval tv;
	FD_ZERO( &rfds );
	FD_SET(( unsigned int )0, &rfds );
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if( select( 1, &rfds, NULL, NULL, &tv ) > 0 )
	{
		if( read( 0, &buffer[pos], 1 ) == 0 )
		{
			// select says we have data, but there's nothing to read - assume EOF
			buffer[pos] = '\n';
			atEOF = true;
		}
		if( buffer[pos] == '\n' || pos == MessageLen - 1 )
		{
			buffer[pos] = '\0';
			str = buffer;
			if( pos != 0 )
			{
				pos = 0;
				return true;
			}
			pos = 0;
		}
		pos++;
	}
	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

#endif 


BZAdminUI *StdBothUI::creator( BZAdminClient &client )
{
	return new StdBothUI( client );
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
