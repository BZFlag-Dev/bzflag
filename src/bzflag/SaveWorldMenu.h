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

#ifndef	__SAVEWORLDMENU_H__
#define	__SAVEWORLDMENU_H__

// common - 1st
#include "common.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiTypeIn.h"
#include "HUDuiLabel.h"


class SaveWorldMenu : public HUDDialog {
public:
  SaveWorldMenu();
  ~SaveWorldMenu();

  HUDuiDefaultKey* getDefaultKey();

  void execute();
  void resize(int width, int height);

private:
  HUDuiTypeIn* filename;
  HUDuiLabel* status;
};


#endif /* __SAVEWORLDMENU_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

