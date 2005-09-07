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

#ifndef _BITMAP_FONT_H_
#define _BITMAP_FONT_H_

/* common header */
#include "common.h"

/* interface headers */
#include "ImageFont.h"
#include "bzfgl.h"
#include "OpenGLGState.h"

class BitmapFont : public ImageFont {
public:
  BitmapFont();
  virtual ~BitmapFont();

  virtual void build();
  virtual bool isBuilt() const {return loaded;}

  virtual void filter(bool dofilter);
  virtual void drawString(float scale, GLfloat color[4], const char *str, int len);

  virtual void free();

private:
  OpenGLGState gstate;
  unsigned char *bitmaps[MAX_TEXTURE_FONT_CHARS];
  bool	loaded;
};

#endif //_BITMAP_FONT_H_
