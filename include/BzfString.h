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
 *	A simple string implementation with reference counting.
 *	Designed for efficient time behavior when editing.  Can
 *	waste some space.
 *
 * Description of selected members:
 *   isNull() const					true iff null (zero-length) string
 *   compact()						Make space efficient
 *   operator+(const BzfString&) const	and ...
 *   operator+(const char*) const	Concatenate strings
 *   operator+=(const BzfString&)	and ...
 *   operator+=(const char*)		Append string
 *   operator<<(const BzfString&)	and ...
 *   operator<<(const char*)		Append string (op is evaluated left to
 *									right, so can append several strings)
 *   operator()(int)				Return substring (from `start to end
 *									of string)
 *   operator()(int, int)			Return substring
 */

#ifndef BZF_STRING_H
#define BZF_STRING_H

#include "common.h"
#include "bzfio.h"
#include <stdarg.h>

class BzfString {
public:
	typedef unsigned int size_type;

	BzfString();
	BzfString(const BzfString&);
	BzfString(const char*);
	BzfString(const char*, size_type length);
	~BzfString();
	BzfString&			operator=(const BzfString&);

	bool				empty() const;
	size_type			size() const;
	const char*			c_str() const;
	void				compact();
	void				swap(BzfString&);

	BzfString			operator+(const BzfString&) const;
	BzfString			operator+(const char*) const;
	BzfString&			operator+=(const BzfString&);
	BzfString&			operator+=(const char*);
	BzfString&			operator+=(char);
	BzfString&			operator<<(const BzfString&);
	BzfString&			operator<<(const char*);

	BzfString			operator()(size_type start) const;
	BzfString			operator()(size_type start, size_type length) const;

	bool				operator==(const char*) const;
	bool				operator!=(const char*) const;
	bool				operator==(const BzfString&) const;
	bool				operator!=(const BzfString&) const;
	bool				operator<(const BzfString&) const;
	bool				operator<=(const BzfString&) const;
	bool				operator>(const BzfString&) const;
	bool				operator>=(const BzfString&) const;

	friend ostream&		operator<<(ostream&, const BzfString&);

	void				append(const char*, size_type length);
	void				truncate(size_type length);

	static BzfString	format(const char* fmt, ...);
	static BzfString	vformat(const char* fmt, va_list);

private:
	void				makeUnique();
	void				ref();
	bool				unref();

private:
	class Rep {
	public:
						Rep(const char*, size_type length);
						Rep(const Rep*);
						~Rep();

	public:
		int				refCount;				// reference count
		size_type		length;					// length of string
		size_type		size;					// size of string buffer
		char*			string;
	};
	Rep*				rep;
};

#endif // BZF_STRING_H
