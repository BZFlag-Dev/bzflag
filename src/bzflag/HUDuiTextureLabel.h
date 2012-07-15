/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
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
 * HUDuiTextureLabel:
 *	User interface classes and functions for the heads-up display's textured
 *  label controls.
 */

#ifndef	__HUDUITEXTURELABEL_H__
#define	__HUDUITEXTURELABEL_H__

#include "HUDuiLabel.h"
#include "OpenGLGState.h"

class HUDuiTextureLabel : public HUDuiLabel {
  public:
			HUDuiTextureLabel();
			~HUDuiTextureLabel();

    void		setTexture(const int);
    int			getTexture() const;

  protected:
    void		doRender();

  private:
    OpenGLGState	gstate;
    int		texture;
};

inline int HUDuiTextureLabel::getTexture() const {
  return texture;
}

#endif // __HUDUITEXTURELABEL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
