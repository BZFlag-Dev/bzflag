/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDui:
 *	User interface classes and functions for the heads-up display.
 */

#ifndef	BZF_HUD_UI_H
#define	BZF_HUD_UI_H

#include "common.h"
#include "BzfEvent.h"
#include "BzfString.h"
#include "OpenGLGState.h"
#include "OpenGLTexFont.h"
#include "OpenGLTexture.h"
#include "AList.h"
#include "TimeKeeper.h"

class HUDuiControl;
class HUDuiDefaultKey;

typedef void		(*HUDuiCallback)(HUDuiControl*, void*);

class HUDui {
  public:
    static HUDuiControl* getFocus();
    static void		setFocus(HUDuiControl*);

    static HUDuiDefaultKey* getDefaultKey();
    static void		setDefaultKey(HUDuiDefaultKey*);

    static boolean	keyPress(const BzfKeyEvent&);
    static boolean	keyRelease(const BzfKeyEvent&);

  private:
    static HUDuiControl *focus;
    static HUDuiDefaultKey* defaultKey;
};

class HUDuiDefaultKey {
  public:
			HUDuiDefaultKey();
    virtual		~HUDuiDefaultKey();

    virtual boolean	keyPress(const BzfKeyEvent&);
    virtual boolean	keyRelease(const BzfKeyEvent&);
};

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
    BzfString		getLabel() const;
    const OpenGLTexFont	&getFont() const;
    HUDuiControl*	getPrev() const;
    HUDuiControl*	getNext() const;
    HUDuiCallback	getCallback() const;
    void*		getUserData() const;

    void		setPosition(float x, float y);
    void		setSize(float width, float height);
    void		setLabelWidth(float width);
    void		setLabel(const BzfString& label);
    void		setFont(const OpenGLTexFont&);
    void		setFontSize(float w, float h);
    void		setPrev(HUDuiControl*);
    void		setNext(HUDuiControl*);
    void		setCallback(HUDuiCallback, void*);

    boolean		hasFocus() const;
    void		setFocus();
    void		showFocus(boolean);

    void		render();

    static OpenGLTexture getArrow() { return *arrow; }

  protected:
    virtual void	onSetFont();
    virtual boolean	doKeyPress(const BzfKeyEvent&) = 0;
    virtual boolean	doKeyRelease(const BzfKeyEvent&) = 0;
    virtual void	doRender() = 0;

    void		renderFocus();
    void		renderLabel();

    void		doCallback();

  private:
    boolean		showingFocus;
    OpenGLTexFont	font;
    float		x, y;
    float		width, height;
    float		fontHeight;
    float		desiredLabelWidth, trueLabelWidth;
    BzfString		label;
    HUDuiControl*	prev, *next;
    HUDuiCallback	cb;
    void*		userData;
    static OpenGLGState* gstate;
    static OpenGLTexture* arrow;
    static int		arrowFrame;
    static TimeKeeper	lastTime;
    static int		totalCount;
};

class HUDuiList : public HUDuiControl {
  public:
			HUDuiList();
			~HUDuiList();

    int			getIndex() const;
    void		setIndex(int);

    BzfStringAList&	getList();
    void		update();

  protected:
    boolean		doKeyPress(const BzfKeyEvent&);
    boolean		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    int			index;
    BzfStringAList	list;
};

class HUDuiTypeIn : public HUDuiControl {
  public:
			HUDuiTypeIn();
			~HUDuiTypeIn();

    int			getMaxLength() const;
    BzfString		getString() const;

    void		setMaxLength(int);
    void		setString(const BzfString&);

  protected:
    void		onSetFont();
    boolean		doKeyPress(const BzfKeyEvent&);
    boolean		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    int			maxLength;
    BzfString		string;
    float		stringWidth;
};

class HUDuiLabel : public HUDuiControl {
  public:
			HUDuiLabel();
			~HUDuiLabel();

    BzfString		getString() const;
    void		setString(const BzfString&);

  protected:
    void		onSetFont();
    boolean		doKeyPress(const BzfKeyEvent&);
    boolean		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    BzfString		string;
};

class HUDuiTextureLabel : public HUDuiLabel {
  public:
			HUDuiTextureLabel();
			~HUDuiTextureLabel();

    void		setTexture(const OpenGLTexture&);

  protected:
    void		doRender();

  private:
    OpenGLGState	gstate;
    OpenGLTexture	texture;
};

BZF_DEFINE_ALIST(HUDuiControlList, HUDuiControl*);

//
// HUDuiControl
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

#endif // BZF_HUD_UI_H
