/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDuiTypeIn:
 *	User interface class for the heads-up display's editable input control.
 */

#ifndef	__HUDUITYPEIN_H__
#define	__HUDUITYPEIN_H__

#include "common.h"

/* system interface headers */
#include <wctype.h>
#include <string>

/* common interface headers */
#include "BzfEvent.h"
#include "HUDuiControl.h"
#include "bzUnicode.h"

// wrapper just for HUDuiTypeIn
class CountUTF8StringItr : public UTF8StringItr
{
public:
  CountUTF8StringItr(const char* string) :
    UTF8StringItr(string), counter(0) {}

  inline CountUTF8StringItr& operator++()
  {
    counter++;
    UTF8StringItr::operator++();
    return (*this);
  }

  inline void operator=(const char* value)
  {
    counter = 0;
    UTF8StringItr::operator=(value);
  }

  inline int getCount() const { return counter; }

private:
  int counter;
};

class HUDuiTypeIn : public HUDuiControl {
  public:
			HUDuiTypeIn();
			~HUDuiTypeIn();

    void		setObfuscation(bool on);
    int			getMaxLength() const;
    std::string		getString() const;

    void		setMaxLength(int);
    void		setString(const std::string&);
    void		setEditing(bool _allowEdit);

  protected:
    bool		doKeyPress(const BzfKeyEvent&);
    bool		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    int			maxLength;
    std::string		string;
    CountUTF8StringItr	cursorPos;
    bool		allowEdit;
    bool		obfuscate;
};

#endif // __HUDUITYPEIN_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
