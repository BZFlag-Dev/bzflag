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
 * HUDuiFrame:
 *	User interface class and functions for a basic frame.
 */

#ifndef	__HUDUIFRAME_H__
#define	__HUDUIFRAME_H__

/* common header */
#include "common.h"

/* parent interface header */
#include "HUDuiElement.h"

/* system headers */
#include <string>

class HUDuiFrame : public HUDuiElement {
  public:
			HUDuiFrame();
    virtual		~HUDuiFrame();

    enum FrameType {
      RectangleStyle,
      RoundedRectStyle
    };

    void		setColor(float color[4]);
    const float*	getColor() const;

    void		setStyle(FrameType style);
    FrameType		getStyle() const;

    void		setLineWidth(float width);
    float		getLineWidth() const;

  protected:
    void		doRender();

  private:
    void		drawArc(float x, float y, float r, int sides,
				float atAngle, float thruAngle);

    float lineWidth;
    float color[4];
    FrameType style;
};

inline void HUDuiFrame::setColor(float _color[4])
{
  color[0] = _color[0];
  color[1] = _color[1];
  color[2] = _color[2];
  color[3] = _color[3];
}

inline const float* HUDuiFrame::getColor() const
{
  return color;
}

inline void HUDuiFrame::setLineWidth(float _width)
{
  lineWidth = _width;
}

inline float HUDuiFrame::getLineWidth() const
{
  return lineWidth;
}

inline void HUDuiFrame::setStyle(FrameType _style)
{
  style = _style;
}

inline HUDuiFrame::FrameType HUDuiFrame::getStyle() const
{
  return style;
}

#endif // __HUDUIFRAME_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
