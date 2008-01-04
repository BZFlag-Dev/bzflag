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

#ifndef __FORMATMENU_H__
#define __FORMATMENU_H__

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiLabel.h"
#include "HUDuiDefaultKey.h"
#include "MenuDefaultKey.h"
#include "HUDNavigationQueue.h"


class FormatMenu;

class FormatMenuDefaultKey : public MenuDefaultKey {
public:
  FormatMenuDefaultKey(FormatMenu* _menu) :
    menu(_menu) { }
  ~FormatMenuDefaultKey() { }

  bool keyPress(const BzfKeyEvent&);
  bool keyRelease(const BzfKeyEvent&);

private:
  FormatMenu* menu;
};

class FormatMenu : public HUDDialog {
  friend class FormatMenuDefaultKey;
public:
  FormatMenu();
  ~FormatMenu();

  HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
  void			show();
  void			execute();
  void			resize(int width, int height);

private:
  void			addLabel(const char* msg, const char* _label, bool navigable = false);
  void			setPage(int page);
  void			setFormat(bool test);

private:
  static const int	NumItems;
  FormatMenuDefaultKey	defaultKey;
  int			numFormats;
  float			center;

  HUDuiLabel*		currentLabel;
  HUDuiLabel*		pageLabel;
  int			page;
  bool*			badFormats;
  static size_t		navCallback(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void*);

  static const int	NumColumns;
  static const int	NumReadouts;
};


#endif /* __FORMATMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
