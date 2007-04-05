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

#ifndef _BITMAP_FONT_H_
#define _BITMAP_FONT_H_

/* common header */
#include "common.h"

// System Headers
#include <crystalspace.h>

/* interface headers */
#include "ImageFont.h"

class BitmapFont : public ImageFont {
public:
  BitmapFont();
  virtual ~BitmapFont();

  virtual void build();
  virtual bool isBuilt() const {return true;}

  virtual void filter(bool dofilter);
  virtual void drawString(float scale, GLfloat color[4], const char *str, int len);
  virtual void drawString(int x, int y, GLfloat color[3], const char *str,
			  int len);

  virtual void free();

private:
  /// A pointer to the 3D renderer plugin.
  csRef<iGraphics3D> g3d;
  csRef<iFont>       font;
};

#endif //_BITMAP_FONT_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

