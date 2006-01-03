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

/*
 * HUDuiTypeIn:
 *	User interface class for the heads-up display's editable input control.
 */

#ifndef	__HUDUITYPEIN_H__
#define	__HUDUITYPEIN_H__

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "BzfEvent.h"
#include "HUDuiControl.h"


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
    int			cursorPos;
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
