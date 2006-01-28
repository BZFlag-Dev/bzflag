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

#ifndef __NEWVERSIONMENU_H__
#define __NEWVERSIONMENU_H__

/* common interface headers */
#include "HUDDialog.h"
#include "cURLManager.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "HUDuiControl.h"
#include "HUDuiTypeIn.h"


/** this class provides options for setting the gui
 */
class NewVersionMenu : public HUDDialog, cURLManager {
public:
  NewVersionMenu(std::string announce, std::string url, std::string date);
  ~NewVersionMenu();

  HUDuiDefaultKey* getDefaultKey()
  {
    return MenuDefaultKey::getInstance();
  }
  void execute();
  void resize(int width, int height);

  // cURL download
  void finalization(char *data, unsigned int length, bool good);

private:
  // no default constructor
  NewVersionMenu();

  // cURL status
  void collectData(char* ptr, int len);
  int byteTransferred;

  // menu items
  HUDuiControl* yes;
  HUDuiControl* no;
  HUDuiControl* status;
};

#endif