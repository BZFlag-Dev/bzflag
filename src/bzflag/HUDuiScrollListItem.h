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

/*
 * HUDuiScrollListItem:
 *	FILL THIS IN
 */

#ifndef	__HUDUISCROLLLISTITEM_H__
#define	__HUDUISCROLLLISTITEM_H__

// ancestor class
#include "HUDuiControl.h"

#include "HUDuiLabel.h"
#include <string>

class HUDuiScrollListItem : public HUDuiControl {
  public:
      HUDuiScrollListItem();
      HUDuiScrollListItem(std::string data);
      HUDuiScrollListItem(HUDuiLabel* data);
      ~HUDuiScrollListItem();
			
    void setFontSize(float size);
    void setFontFace(const LocalFontFace* fontFace);
	
    void setSize(float width, float height);
    void setPosition(float x, float y);

    std::string getValue();

  protected:
    void doRender();

  private:
    void shorten(float width);
	
    std::string stringValue;
    HUDuiLabel* label;
};

#endif // __HUDuiScrollListItem_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8