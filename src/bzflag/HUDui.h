/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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
#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "TimeKeeper.h"
#include "Bundle.h"

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

    static int getArrow() { return arrow; }

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
    static int          arrow;
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

    void		createSlider(int);

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
    void		setDarker(bool d); // render darker than usual when not in focus

  protected:
    void		onSetFont();
    bool		doKeyPress(const BzfKeyEvent&);
    bool		doKeyRelease(const BzfKeyEvent&);
    void		doRender();

  private:
  std::string		string;
  std::vector<std::string> *params;
  bool			darker;
};

class HUDuiTextureLabel : public HUDuiLabel {
  public:
			HUDuiTextureLabel();
			~HUDuiTextureLabel();

    void		setTexture(const int);

  protected:
    void		doRender();

  private:
    OpenGLGState	gstate;
    int         	texture;
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
