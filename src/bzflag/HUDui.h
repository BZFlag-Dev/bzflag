/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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

#ifdef _WIN32
#pragma warning( 4:4786 )
#endif

#include "common.h"
#include "BzfEvent.h"
#include "OpenGLGState.h"
#include "OpenGLTexFont.h"
#include "OpenGLTexture.h"
#include "TimeKeeper.h"
#include "Bundle.h"
#include <string>
#include <vector>

class HUDuiControl;
class HUDuiDefaultKey;

typedef void		(*HUDuiCallback)(HUDuiControl*, void*);

class HUDui {
  public:
    static HUDuiControl* getFocus();
    static void		setFocus(HUDuiControl*);

    static HUDuiDefaultKey* getDefaultKey();
    static void		setDefaultKey(HUDuiDefaultKey*);

    static bool	keyPress(const BzfKeyEvent&);
    static bool	keyRelease(const BzfKeyEvent&);

  private:
    static HUDuiControl *focus;
    static HUDuiDefaultKey* defaultKey;
};

class HUDuiDefaultKey {
  public:
			HUDuiDefaultKey();
    virtual		~HUDuiDefaultKey();

    virtual bool	keyPress(const BzfKeyEvent&);
    virtual bool	keyRelease(const BzfKeyEvent&);
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
    std::string		getLabel() const;
    const OpenGLTexFont	&getFont() const;
    HUDuiControl*	getPrev() const;
    HUDuiControl*	getNext() const;
    HUDuiCallback	getCallback() const;
    void*		getUserData() const;

    void		setPosition(float x, float y);
    void		setSize(float width, float height);
    void		setLabelWidth(float width);
    void		setLabel(const std::string& label);
    void		setFont(const OpenGLTexFont&);
    void		setFontSize(float w, float h);
    void		setPrev(HUDuiControl*);
    void		setNext(HUDuiControl*);
    void		setCallback(HUDuiCallback, void*);

    bool		hasFocus() const;
    void		setFocus();
    void		showFocus(bool);

    void		render();

    static OpenGLTexture getArrow() { return *arrow; }

  protected:
    virtual void	onSetFont();
    virtual bool	doKeyPress(const BzfKeyEvent&) = 0;
    virtual bool	doKeyRelease(const BzfKeyEvent&) = 0;
    virtual void	doRender() = 0;

    void		renderFocus();
    void		renderLabel();

    void		doCallback();

  private:
    bool		showingFocus;
    OpenGLTexFont	font;
    float		x, y;
    float		width, height;
    float		fontHeight;
    float		desiredLabelWidth, trueLabelWidth;
    std::string		label;
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

    std::vector<std::string>&	getList();
    void		update();

  protected:
    bool		doKeyPress(const BzfKeyEvent&);
    bool		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    int			index;
    std::vector<std::string>	list;
};

class HUDuiTypeIn : public HUDuiControl {
  public:
			HUDuiTypeIn();
			~HUDuiTypeIn();

    int			getMaxLength() const;
    std::string		getString() const;

    void		setMaxLength(int);
    void		setString(const std::string&);
    void		setEditing(bool _allowEdit);

  protected:
    bool		doKeyPress(const BzfKeyEvent&);
    bool		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
    int			maxLength;
    std::string		string;
    int			cursorPos;
    bool		allowEdit;
};

class HUDuiLabel : public HUDuiControl {
  public:
			HUDuiLabel();
			~HUDuiLabel();

    std::string		getString() const;
    void		setString(const std::string&, const std::vector<std::string> *_params = NULL);

  protected:
    void		onSetFont();
    bool		doKeyPress(const BzfKeyEvent&);
    bool		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
  std::string		string;
  std::vector<std::string> *params;
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
// ex: shiftwidth=2 tabstop=8
