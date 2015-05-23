/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__SHOTSTATS_H__
#define	__SHOTSTATS_H__

#include "common.h"

#include "HUDDialog.h"
#include "HUDuiControl.h"
#include "HUDuiDefaultKey.h"

class Player;

/** ShotStats displays a set of statistics on player's shots and accuracies
 */
class ShotStats : public HUDDialog {
public:
  ShotStats();
  ~ShotStats();

  HUDuiDefaultKey* getDefaultKey();
  void resize(int width, int height);
  void execute();
  void dismiss();
  void show();

  void refresh();

  static int getFontFace();

private:

  void addStats(Player* player, std::vector<HUDuiControl*>& list);
  void createLabel(const std::string &str, std::vector<HUDuiControl*>& list);

  int staticLabelCount;
  int rows;
  int columns;
  bool visible;

};

#endif // __SHOTSTATS_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
