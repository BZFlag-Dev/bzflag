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


// common goes first
#include "common.h"

// implementation header
#include "bzglob.h"

// system headers
#include <string>


static int match_multi( const char **pattern, const char **string );

static const char MATCH_MULTI = '*'; // matches any number of characters
static const char MATCH_SINGLE = '?'; // matches a single character


/******************************************************************************/

bool glob_match( const std::string &pattern, const std::string &string )
{
	return glob_match( pattern.c_str(), string.c_str());
}

/******************************************************************************/

bool glob_match( const char *pattern, const char *string )
{
	if( pattern == NULL )
	{
		return false;
	}
	if( string == NULL )
	{
		return false;
	}

	if(( pattern[0] == MATCH_MULTI ) && ( pattern[1] == '\0' ))
	{
		return true;
	}

	while( *pattern != '\0' )
	{
		if( *pattern == MATCH_MULTI )
		{
			pattern++;
			switch( match_multi( &pattern, &string ))
			{
				case  + 1: 
				{
					return true;
				}
				case  - 1: 
				{
					return false;
				}
			}
		}
		else if( *string == '\0' )
		{
			return false;
		}
		else if(( *pattern == MATCH_SINGLE ) || ( *pattern ==  *string ))
		{
			pattern++;
			string++;
		}
		else
		{
			return false;
		}
	}

	if( *string == '\0' )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************/

static int match_multi( const char **pattern, const char **string )
{
	const char *str =  *pattern;
	const char *obj =  *string;

	while(( *str != '\0' ) && ( *str == MATCH_MULTI ))
	{
		str++; // get rid of multiple '*'s
	}

	if( *str == '\0' )
	{
		// '*' was last, auto-match
		return  + 1;
	}

	const char *strtop = str;
	const char *objtop = obj;

	while( *str != '\0' )
	{
		if( *str == MATCH_MULTI )
		{
			*pattern = str;
			*string = obj;
			return 0; // matched this segment
		}
		else if( *obj == '\0' )
		{
			return  - 1; // can't match
		}
		else
		{
			if(( *str == MATCH_SINGLE ) || ( *str ==  *obj ))
			{
				str++;
				obj++;
				if(( *str == '\0' ) && ( *obj != '\0' ))
				{
					// advanced check
					obj++;
					objtop++;
					obj = objtop;
					str = strtop;
				}
			}
			else
			{
				obj++;
				objtop++;
				obj = objtop;
				str = strtop;
			}
		}
	}

	*pattern = str;
	*string = obj;

	return  + 1; // full match
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
