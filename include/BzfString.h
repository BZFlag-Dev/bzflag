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

/* BzfString:
 *	An extension of std::string 
 *
 */

#ifndef BZF_STRING_H
#define BZF_STRING_H

#include "common.h"
#include "bzfio.h"
#include <stdarg.h>
#include <string>

class BzfString : public std::string
{
public:
	BzfString()
	{
	}

	BzfString(const char *src)
	{
		assign(src);
	}

	BzfString(const std::string &src)
	{
		assign(src);
	}

	BzfString(const char* src, size_type length)
	{
		assign(src, 0, length );
	}


	BzfString& operator=(const BzfString& src)
	{
		if (this != &src)
			assign(src);
		return *this;
	}

	BzfString& truncate(int len)
	{
		resize(len);
		return *this;
	}

	BzfString& operator+(const BzfString& src)
	{
		append(src);
		return *this;
	}

	BzfString operator()(size_type off, size_type count = npos) const
	{
		return BzfString(substr(off, count).c_str());
	}


	static BzfString	format(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		BzfString result = vformat(fmt, args);
		va_end(args);
		return result;
	}

	static BzfString	vformat(const char* fmt, va_list args)
	{
		// FIXME -- should prevent buffer overflow in all cases
		// not all platforms support vsnprintf so we'll use vsprintf and a
		// big temporary buffer and hope for the best.
		char buffer[8192];
		vsprintf(buffer, fmt, args);
		return BzfString(buffer);
	}
};

#endif // BZF_STRING_H
