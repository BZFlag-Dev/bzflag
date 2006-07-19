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
 * HUDuiImage:
 *	User interface classes and functions for the heads-up display's
 * independent images.
 */

#ifndef	__HUDUIIMAGE_H__
#define	__HUDUIIMAGE_H__

#include "HUDuiElement.h"
#include "OpenGLGState.h"

class HUDuiImage : public HUDuiElement {
  public:
			HUDuiImage();
			~HUDuiImage();

    void		setTexture(const int);
    int			getTexture();

  protected:
    void		doRender();

  private:
    OpenGLGState	gstate;
    int			texture;
};

inline int HUDuiImage::getTexture() {
  return texture;
}

#endif // __HUDUIIMAGE_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
