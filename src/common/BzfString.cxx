/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfio.h"
#include <string.h>
#include "BzfString.h"

//
// BzfString::Rep
//

BzfString::Rep::Rep(const char* _string, int _length) :
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
  if (length > 1) {
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

BzfString::BzfString(const char* string, int length)
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

BzfString&		BzfString::operator=(const BzfString& s)
{
  if (rep != s.rep) {
    if (unref()) delete rep;
    rep = s.rep;
    ref();
  }
  return *this;
}

int			BzfString::getLength() const
{
  return rep->length;
}

const char*		BzfString::getString() const
{
  return rep->string;
}

BzfString::operator const char*() const
{
  return rep->string;
}

boolean			BzfString::isNull() const
{
  return rep->length == 0;
}

void			BzfString::compact()
{
  if (rep->length + 1 == rep->size) return;	// already compacted
  Rep* newRep = new Rep(getString(), getLength());
  if (unref()) delete rep;
  rep = newRep;
}

BzfString		BzfString::operator+(const BzfString& tail) const
{
  BzfString s(*this);
  return s += tail;
}

BzfString		BzfString::operator+(const char* tail) const
{
  BzfString s(*this);
  return s += tail;
}

BzfString&		BzfString::operator+=(const BzfString& tail)
{
  append(tail.getString(), tail.getLength());
  return *this;
}

BzfString&		BzfString::operator+=(const char* tail)
{
  if (tail) append(tail, ::strlen(tail));
  return *this;
}

BzfString&		BzfString::operator<<(const BzfString& tail)
{
  return operator+=(tail);
}

BzfString&		BzfString::operator<<(const char* tail)
{
  return operator+=(tail);
}

BzfString		BzfString::operator()(int start) const
{
  return operator()(start, rep->length - start);
}

BzfString		BzfString::operator()(int start, int length) const
{
#ifdef DEBUG
  // NOTE: should throw an exception instead
  assert(start >= 0 && start < rep->length);
  assert(length >= 0 && start + length <= rep->length);
#endif
  return BzfString(rep->string + start, length);
}

boolean			BzfString::operator==(const char* s) const
{
  if (rep->length != (int)strlen(s)) return False;
  return ::memcmp(rep->string, s, rep->length) == 0;
}

boolean			BzfString::operator!=(const char* s) const
{
  return !operator==(s);
}

boolean			BzfString::operator==(const BzfString& s) const
{
  if (rep == s.rep) return True;
  if (rep->length != s.rep->length) return False;
  return ::memcmp(rep->string, s.rep->string, rep->length) == 0;
}

boolean			BzfString::operator!=(const BzfString& s) const
{
  return !operator==(s);
}

boolean			BzfString::operator<(const BzfString& s) const
{
  if (rep == s.rep) return False;
  return ::strcmp(rep->string, s.rep->string) < 0;
}

boolean			BzfString::operator<=(const BzfString& s) const
{
  if (rep == s.rep) return True;
  return ::strcmp(rep->string, s.rep->string) <= 0;
}

boolean			BzfString::operator>(const BzfString& s) const
{
  return !operator<=(s);
}

boolean			BzfString::operator>=(const BzfString& s) const
{
  return !operator<(s);
}

void			BzfString::makeUnique()
{
  if (rep->refCount != 1) {
    unref();
    rep = new Rep(rep);
  }
}

void			BzfString::append(const char* string, int length)
{
  makeUnique();
  int newLength = rep->length + length;
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

void			BzfString::truncate(int length)
{
  makeUnique();
  if (length >= rep->length) return;
  rep->length = length;
  rep->string[rep->length] = '\0';
}

void			BzfString::ref()
{
  ++rep->refCount;
}

boolean			BzfString::unref()
{
  return (--rep->refCount == 0);
}

//
// BzfString friend functions
//

ostream&		operator<<(ostream& stream, const BzfString& string)
{
  return stream << string.rep->string;
}
