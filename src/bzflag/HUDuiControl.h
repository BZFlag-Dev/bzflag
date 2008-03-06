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
 * HUDuiControl:
 *	User interface class and functions for the basic HUD UI control.
 */

#ifndef	__HUDUICONTROL_H__
#define	__HUDUICONTROL_H__

/* common header */
#include "common.h"

/* system headers */
#include <string>

// common interface headers
#include "BzfEvent.h"
#include "OpenGLGState.h"
#include "TimeKeeper.h"

class HUDuiControl;

typedef void		(*HUDuiCallback)(HUDuiControl*, void*);

class HUDuiControl {
  friend class HUDui;
  public:
			HUDuiControl();
    virtual		~HUDuiControl();

    float		getX() const;
    float		getY() const;
    float		getWidth() const;
    float		getHeight() const;
    float		getLabelWidth() const;
    std::string		getLabel() const;
    int			getFontFace() const;
    float		getFontSize() const;
    HUDuiControl*	getPrev() const;
    HUDuiControl*	getNext() const;
    HUDuiCallback	getCallback() const;
    void*		getUserData() const;

    void		setPosition(float x, float y);
    void		setSize(float width, float height);
    void		setLabelWidth(float width);
    void		setLabel(const std::string& label);
    void		setFontFace(int face);
    void		setFontSize(float size);
    void		setPrev(HUDuiControl*);
    void		setNext(HUDuiControl*);
    void		setCallback(HUDuiCallback, void*);

    bool		hasFocus() const;
    void		setFocus();
    void		showFocus(bool);

    void		render();

    static int  getArrow() { return arrow; }

  protected:
    virtual void	onSetFont();
    virtual bool	doKeyPress(const BzfKeyEvent&) = 0;
    virtual bool	doKeyRelease(const BzfKeyEvent&) = 0;
    virtual void	doRender() = 0;

    void		renderFocus();
    void		renderLabel();

    void		doCallback();

    static const GLfloat	dimTextColor[3];
    static const GLfloat	moreDimTextColor[3];
    static const GLfloat	textColor[3];

  private:
    bool		showingFocus;
    int			fontFace;
    float		fontSize;
    float		x, y;
    float		width, height;
    float		fontHeight;
    float		desiredLabelWidth, trueLabelWidth;
    std::string		label;
    HUDuiControl*	prev, *next;
    HUDuiCallback	cb;
    void*		userData;
    static OpenGLGState* gstate;
    static int	  arrow;
    static int		arrowFrame;
    static TimeKeeper	lastTime;
    static int		totalCount;
};

//
// inline functions
//

inline
float			HUDuiControl::getX() const
{
  return x;
}

inline
float			HUDuiControl::getY() const
{
  return y;
}

inline
float			HUDuiControl::getWidth() const
{
  return width;
}

inline
float			HUDuiControl::getHeight() const
{
  return height;
}

#endif // __HUDUICONTROL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
