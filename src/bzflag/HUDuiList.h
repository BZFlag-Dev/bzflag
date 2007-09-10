/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 * HUDuiList:
 *	User interface class for the heads-up display's list (value selection from
 *  a set) control.
 */

#ifndef	__HUDUILIST_H__
#define	__HUDUILIST_H__

// ancestor class
#include "HUDuiControl.h"

#include <string>
#include <vector>

#include "BzfEvent.h"

class HUDuiList : public HUDuiControl {
  public:
			HUDuiList();
			~HUDuiList();

    int			getIndex() const;
    void		setIndex(int);

    void		createSlider(int);

    std::vector<std::string>&	getList();
    void		update();

  protected:
    bool		doKeyPress(const BzfKeyEvent&);
    bool		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    int			index;
    std::vector<std::string>	list;
};

#endif // __HUDUILIST_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
