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

/* BzfString:
 *	A simple string implementation with reference counting.
 *	Designed for efficient time behavior when editing.  Can
 *	waste some space.
 *
 * I'd have preferred to use the name `String' but, you guessed it,
 * X11 strikes again.  As if there weren't enough reasons to dislike
 * X11.  It's a honking great big steaming pile of name-space pollution.
 *
 * Description of selected members:
 *   isNull() const			True iff null (zero-length) string
 *   compact()				Make space efficient
 *   operator+(const BzfString&) const	and ...
 *   operator+(const char*) const	Concatenate strings
 *   operator+=(const BzfString&)	and ...
 *   operator+=(const char*)		Append string
 *   operator<<(const BzfString&)	and ...
 *   operator<<(const char*)		Append string (op is evaluated left to
 *					right, so can append several strings)
 *   operator()(int)			Return substring (from `start to end
 *					of string)
 *   operator()(int, int)		Return substring
 */

#ifndef BZF_STRING_H
#define	BZF_STRING_H

#include "common.h"

#if defined(sun) || defined(__MWERKS__)
// solaris compiler doesn't like declaration of ostream
#include "bzfio.h"
#else
class ostream;
#endif

class BzfString {
  public:
			BzfString();
			BzfString(const BzfString&);
			BzfString(const char*);
			BzfString(const char*, int length);
			~BzfString();
    BzfString&		operator=(const BzfString&);

    boolean		isNull() const;
    int			getLength() const;
    const char*		getString() const;
			operator const char*() const;
    void		compact();

    BzfString		operator+(const BzfString&) const;
    BzfString		operator+(const char*) const;
    BzfString&		operator+=(const BzfString&);
    BzfString&		operator+=(const char*);
    BzfString&		operator<<(const BzfString&);
    BzfString&		operator<<(const char*);

    BzfString		operator()(int start) const;
    BzfString		operator()(int start, int length) const;

    boolean		operator==(const char*) const;
    boolean		operator!=(const char*) const;
    boolean		operator==(const BzfString&) const;
    boolean		operator!=(const BzfString&) const;
    boolean		operator<(const BzfString&) const;
    boolean		operator<=(const BzfString&) const;
    boolean		operator>(const BzfString&) const;
    boolean		operator>=(const BzfString&) const;

    friend ostream&	operator<<(ostream&, const BzfString&);

    void		append(const char*, int length);
    void		truncate(int length);

  private:
    void		makeUnique();
    void		ref();
    boolean		unref();

  private:
    class Rep {
      public:
			Rep(const char*, int length);
			Rep(const Rep*);
			~Rep();

      public:
	int		refCount;		// reference count
	int		length;			// length of string
	int		size;			// size of string buffer
	char*		string;
    };
    Rep*		rep;
};

#include "AList.h"
BZF_DEFINE_ALIST(BzfStringAList, BzfString);

#endif // BZF_STRING_H
