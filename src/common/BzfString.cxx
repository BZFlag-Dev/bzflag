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

#include "BzfString.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

//
// BzfString::Rep
//

BzfString::Rep::Rep(const char* _string, size_type _length) :
								refCount(1),
								length(_length),
								size(_length + 1)
{
	string = new char[size];
	if (length > 0) {
		if (_string)
			::memcpy(string, _string, length * sizeof(char));
		else
			::memset(string, '\0', length * sizeof(char));
	}
	string[length] = '\0';
}

BzfString::Rep::Rep(const Rep* rep) :
								refCount(1),
								length(rep->length),
								size(rep->size)
{
	string = new char[size];
	if (length >= 1) {
		if (rep->string)
			::memcpy(string, rep->string, length * sizeof(char));
		else
			::memset(string, '\0', length * sizeof(char));
	}
	string[length] = '\0';
}

BzfString::Rep::~Rep()
{
	delete[] string;
}

//
// BzfString
//

BzfString::BzfString()
{
	rep = new Rep("", 0);
}

BzfString::BzfString(const BzfString& string)
{
	rep = string.rep;
	ref();
}

BzfString::BzfString(const char* string)
{
	if (!string)
		rep = new Rep("", 0);
	else
		rep = new Rep(string, ::strlen(string));
}

BzfString::BzfString(const char* string, size_type length)
{
	if (!string)
		rep = new Rep("", 0);
	else
		rep = new Rep(string, length);
}

BzfString::~BzfString()
{
	if (unref()) delete rep;
}

BzfString&				BzfString::operator=(const BzfString& s)
{
	if (rep != s.rep) {
		if (unref()) delete rep;
		rep = s.rep;
		ref();
	}
	return *this;
}

BzfString::size_type	BzfString::size() const
{
	return rep->length;
}

const char*				BzfString::c_str() const
{
	return rep->string;
}

bool					BzfString::empty() const
{
	return rep->length == 0;
}

void					BzfString::compact()
{
	if (rep->length + 1 == rep->size) return;		// already compacted
	Rep* newRep = new Rep(c_str(), size());
	if (unref()) delete rep;
	rep = newRep;
}

BzfString				BzfString::operator+(const BzfString& tail) const
{
	BzfString s(*this);
	return s += tail;
}

BzfString				BzfString::operator+(const char* tail) const
{
	BzfString s(*this);
	return s += tail;
}

BzfString&				BzfString::operator+=(const BzfString& tail)
{
	append(tail.c_str(), tail.size());
	return *this;
}

BzfString&				BzfString::operator+=(const char* tail)
{
	if (tail) append(tail, ::strlen(tail));
	return *this;
}

BzfString&				BzfString::operator<<(const BzfString& tail)
{
	return operator+=(tail);
}

BzfString&				BzfString::operator<<(const char* tail)
{
	return operator+=(tail);
}

BzfString				BzfString::operator()(size_type start) const
{
	return operator()(start, rep->length - start);
}

BzfString				BzfString::operator()(
								size_type start, size_type length) const
{
	assert(start < rep->length);
	assert(start + length <= rep->length);
	return BzfString(rep->string + start, length);
}

bool					BzfString::operator==(const char* s) const
{
	if (rep->length != strlen(s))
		return false;
	return ::memcmp(rep->string, s, rep->length) == 0;
}

bool					BzfString::operator!=(const char* s) const
{
	return !operator==(s);
}

bool					BzfString::operator==(const BzfString& s) const
{
	if (rep == s.rep) return true;
	if (rep->length != s.rep->length) return false;
	return ::memcmp(rep->string, s.rep->string, rep->length) == 0;
}

bool					BzfString::operator!=(const BzfString& s) const
{
	return !operator==(s);
}

bool					BzfString::operator<(const BzfString& s) const
{
	if (rep == s.rep) return false;
	return ::strcmp(rep->string, s.rep->string) < 0;
}

bool					BzfString::operator<=(const BzfString& s) const
{
	if (rep == s.rep) return true;
	return ::strcmp(rep->string, s.rep->string) <= 0;
}

bool					BzfString::operator>(const BzfString& s) const
{
	return !operator<=(s);
}

bool					BzfString::operator>=(const BzfString& s) const
{
	return !operator<(s);
}

void					BzfString::makeUnique()
{
	if (rep->refCount != 1) {
		unref();
		rep = new Rep(rep);
	}
}

void					BzfString::append(const char* string, size_type length)
{
	makeUnique();
	size_type newLength = rep->length + length;
	if (newLength >= rep->size) {
		do { rep->size <<= 1; } while (newLength >= rep->size);
		char* newString = new char[rep->size];
		::memcpy(newString, rep->string, rep->length);
		delete[] rep->string;
		rep->string = newString;
	}
	::memcpy(rep->string + rep->length, string, length);
	rep->length = newLength;
	rep->string[rep->length] = '\0';
}

void					BzfString::truncate(size_type length)
{
	makeUnique();
	if (length >= rep->length) return;
	rep->length = length;
	rep->string[rep->length] = '\0';
}

BzfString				BzfString::format(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	BzfString result = vformat(fmt, args);
	va_end(args);
	return result;
}

BzfString				BzfString::vformat(const char* fmt, va_list args)
{
	// FIXME -- should prevent buffer overflow in all cases
	// not all platforms support vsnprintf so we'll use vsprintf and a
	// big temporary buffer and hope for the best.
	char buffer[8192];
	vsprintf(buffer, fmt, args);
	return BzfString(buffer);
}

void					BzfString::ref()
{
	++rep->refCount;
}

bool					BzfString::unref()
{
	return (--rep->refCount == 0);
}

//
// BzfString friend functions
//

ostream&				operator<<(ostream& stream, const BzfString& string)
{
	return stream.write(string.rep->string, string.rep->length);
}
