/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef __SILENCEDEFAULTKEY_H__
#define __SILENCEDEFAULTKEY_H__

#include "HUDui.h"


class SilenceDefaultKey : public HUDuiDefaultKey {
public:
  SilenceDefaultKey();
  bool		keyPress(const BzfKeyEvent&);
  bool		keyRelease(const BzfKeyEvent&);
};

#endif
