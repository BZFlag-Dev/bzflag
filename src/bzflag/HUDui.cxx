/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <math.h>
#include <ctype.h>
#include "bzfgl.h"
#include "HUDui.h"
#include "texture.h"

static const char*	arrowFile = "ybolt";
static const GLfloat	dimTextColor[3] = { 0.7f, 0.7f, 0.7f };
static const GLfloat	textColor[3] = { 1.0f, 1.0f, 1.0f };

//
// HUDui
//

HUDuiControl*		HUDui::focus = NULL;
HUDuiDefaultKey*	HUDui::defaultKey = NULL;

HUDuiControl*		HUDui::getFocus()
{
  return focus;
}

void			HUDui::setFocus(HUDuiControl* _focus)
{
  focus = _focus;
}

HUDuiDefaultKey*	HUDui::getDefaultKey()
{
  return defaultKey;
}

void			HUDui::setDefaultKey(HUDuiDefaultKey* _defaultKey)
{
  defaultKey = _defaultKey;
}

boolean			HUDui::keyPress(const BzfKeyEvent& key)
{
  if (defaultKey && defaultKey->keyPress(key)) return True;
  if (focus && focus->doKeyPress(key)) return True;
  return False;
}

boolean			HUDui::keyRelease(const BzfKeyEvent& key)
{
  if (defaultKey && defaultKey->keyRelease(key)) return True;
  if (focus && focus->doKeyRelease(key)) return True;
  return False;
}

//
// HUDuiDefaultKey
//

HUDuiDefaultKey::HUDuiDefaultKey()
{
  // do nothing
}

HUDuiDefaultKey::~HUDuiDefaultKey()
{
  // do nothing
}

boolean			HUDuiDefaultKey::keyPress(const BzfKeyEvent&)
{
  return False;
}

boolean			HUDuiDefaultKey::keyRelease(const BzfKeyEvent&)
{
  return False;
}

//
// HUDuiControl
//

OpenGLGState*		HUDuiControl::gstate;
OpenGLTexture*		HUDuiControl::arrow;
int			HUDuiControl::arrowFrame = 0;
TimeKeeper		HUDuiControl::lastTime;
int			HUDuiControl::totalCount = 0;

HUDuiControl::HUDuiControl() : showingFocus(True),
				x(0.0f), y(0.0f),
				width(1.0f), height(1.0f),
				fontHeight(11.0f),
				desiredLabelWidth(0.0f),
				trueLabelWidth(0.0f),
				prev(this), next(this),
				cb(NULL), userData(NULL)
{
  if (totalCount == 0) {
    // load arrow texture
    arrow = new OpenGLTexture;
    *arrow = getTexture(arrowFile, OpenGLTexture::Linear);

    // make gstate for focus arrow
    gstate = new OpenGLGState;
    OpenGLGStateBuilder builder(*gstate);
    builder.setTexture(*arrow);
    builder.setBlending();
//    builder.setSmoothing();
    builder.enableTextureReplace();
    *gstate = builder.getState();

    // get start time for animation
    lastTime = TimeKeeper::getCurrent();
  }

  totalCount++;
}

HUDuiControl::~HUDuiControl()
{
  if (--totalCount == 0) {
    delete arrow;
    delete gstate;
    arrow = NULL;
    gstate = NULL;
  }
}

float			HUDuiControl::getLabelWidth() const
{
  return desiredLabelWidth;
}

BzfString		HUDuiControl::getLabel() const
{
  return label;
}

const OpenGLTexFont&	HUDuiControl::getFont() const
{
  return font;
}

HUDuiControl*		HUDuiControl::getPrev() const
{
  return prev;
}

HUDuiControl*		HUDuiControl::getNext() const
{
  return next;
}

HUDuiCallback		HUDuiControl::getCallback() const
{
  return cb;
}

void*			HUDuiControl::getUserData() const
{
  return userData;
}

void			HUDuiControl::setPosition(float _x, float _y)
{
  x = _x;
  y = _y;
}

void			HUDuiControl::setSize(float _width, float _height)
{
  width = _width;
  height = _height;
}

void			HUDuiControl::setLabelWidth(float labelWidth)
{
  desiredLabelWidth = labelWidth;
}

void			HUDuiControl::setLabel(const BzfString& _label)
{
  label = _label;
  if (font.isValid()) trueLabelWidth = font.getWidth(label);
}

void			HUDuiControl::setFont(const OpenGLTexFont& _font)
{
  font = _font;
  onSetFont();
}

void			HUDuiControl::setFontSize(float w, float h)
{
  font.setSize(w, h);
  onSetFont();
}

void			HUDuiControl::setPrev(HUDuiControl* _prev)
{
  if (!_prev) prev = this;
  else prev = _prev;
}

void			HUDuiControl::setNext(HUDuiControl* _next)
{
  if (!_next) next = this;
  else next = _next;
}

void			HUDuiControl::setCallback(HUDuiCallback _cb, void* _ud)
{
  cb = _cb;
  userData = _ud;
}

void			HUDuiControl::onSetFont()
{
  if (font.isValid()) {
    int ascent = (int)font.getAscent();
    fontHeight = (float)(ascent | 1) - 4.0f;
    trueLabelWidth = font.getWidth(label);
  }
  else {
    fontHeight = 11.0f;
    trueLabelWidth = 0.0f;
  }
}

boolean			HUDuiControl::hasFocus() const
{
  return this == HUDui::getFocus();
}

void			HUDuiControl::setFocus()
{
  HUDui::setFocus(this);
}

void			HUDuiControl::showFocus(boolean _showingFocus)
{
  showingFocus = _showingFocus;
}

void			HUDuiControl::doCallback()
{
  if (cb) (*cb)(this, userData);
}

void			HUDuiControl::renderFocus()
{
  const float fh2 = floorf(0.5f * fontHeight);

  if (gstate->isTextured()) {
    static const int uFrames = 1; // 4;
    static const int vFrames = 1; // 4;
    static const float du = 1.0f / (float)uFrames;
    static const float dv = 1.0f / (float)vFrames;

    const float u = (float)(arrowFrame % uFrames) / (float)uFrames;
    const float v = (float)(arrowFrame / uFrames) / (float)vFrames;
    gstate->setState();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
      glTexCoord2f(u, v);
      glVertex2f(x - fh2 - fontHeight, y);
      glTexCoord2f(u + du, v);
      glVertex2f(x - fh2 - 1.0f, y);
      glTexCoord2f(u + du, v + dv);
      glVertex2f(x - fh2 - 1.0f, y + fontHeight - 1.0f);
      glTexCoord2f(u, v + dv);
      glVertex2f(x - fh2 - fontHeight, y + fontHeight - 1.0f);
    glEnd();

    TimeKeeper nowTime = TimeKeeper::getCurrent();
    if (nowTime - lastTime > 0.07f) {
      lastTime = nowTime;
      if (++arrowFrame == uFrames * vFrames) arrowFrame = 0;
    }
  }

  else {
    gstate->setState();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
      glVertex2f(x - fh2 - fontHeight, y + fontHeight - 1.0f);
      glVertex2f(x - fh2 - fontHeight, y);
      glVertex2f(x - fh2 - 1.0f, y + 0.5f * (fontHeight - 1.0f));
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
      glVertex2f(x - fh2 - fontHeight, y + fontHeight - 1.0f);
      glVertex2f(x - fh2 - fontHeight, y);
      glVertex2f(x - fh2 - 1.0f, y + 0.5f * (fontHeight - 1.0f));
    glEnd();
  }
}

void			HUDuiControl::renderLabel()
{
  if (label.getLength() > 0 && font.isValid()) {
    const float dx = (desiredLabelWidth > trueLabelWidth) ?
			desiredLabelWidth : trueLabelWidth;
    font.draw(label, x - dx - 2.0f * fontHeight, y);
  }
}

void			HUDuiControl::render()
{
  if (hasFocus() && showingFocus) renderFocus();
  glColor3fv(hasFocus() ? textColor : dimTextColor);
  renderLabel();
  doRender();
}

//
// HUDuiList
//

HUDuiList::HUDuiList() : HUDuiControl(), index(-1)
{
  // do nothing
}

HUDuiList::~HUDuiList()
{
  // do nothing
}

int			HUDuiList::getIndex() const
{
  return index;
}

void			HUDuiList::setIndex(int _index)
{
  if (_index < 0) index = 0;
  else if (_index >= list.getLength()) index = list.getLength() - 1;
  else index = _index;
}

BzfStringAList&		HUDuiList::getList()
{
  return list;
}

void			HUDuiList::update()
{
  setIndex(index);
}

boolean			HUDuiList::doKeyPress(const BzfKeyEvent& key)
{
  if (key.ascii == '\t') {
    HUDui::setFocus(getNext());
    return True;
  }

  switch (key.button) {
    case BzfKeyEvent::Up:
      HUDui::setFocus(getPrev());
      break;

    case BzfKeyEvent::Down:
      HUDui::setFocus(getNext());
      break;

    case BzfKeyEvent::Left:
      if (index != -1) {
	if (--index < 0) index = list.getLength() - 1;
	doCallback();
      }
      break;

    case BzfKeyEvent::Right:
      if (index != -1) {
	if (++index >= list.getLength()) index = 0;
	doCallback();
      }
      break;

    case BzfKeyEvent::Home:
      if (index != -1) {
	index = 0;
	doCallback();
      }
      break;

    case BzfKeyEvent::End:
      if (index != -1) {
	index = list.getLength() - 1;
	doCallback();
      }
      break;

    default:
      return False;
  }

  return True;
}

boolean			HUDuiList::doKeyRelease(const BzfKeyEvent&)
{
  // ignore key releases
  return False;
}

void			HUDuiList::doRender()
{
  if (index != -1 && getFont().isValid()) {
    glColor3fv(hasFocus() ? textColor : dimTextColor);
    getFont().draw(list[index], getX(), getY());
  }
}

//
// HUDuiTypeIn
//

HUDuiTypeIn::HUDuiTypeIn() : HUDuiControl(), maxLength(0), stringWidth(0.0f)
{
}

HUDuiTypeIn::~HUDuiTypeIn()
{
}

int			HUDuiTypeIn::getMaxLength() const
{
  return maxLength;
}

BzfString		HUDuiTypeIn::getString() const
{
  return string;
}

void			HUDuiTypeIn::setMaxLength(int _maxLength)
{
  maxLength = _maxLength;
  string.truncate(maxLength);
  onSetFont();
}

void			HUDuiTypeIn::setString(const BzfString& _string)
{
  string = _string;
  onSetFont();
}

void			HUDuiTypeIn::onSetFont()
{
  // recompute string width
  HUDuiControl::onSetFont();
  if (getFont().isValid())
    stringWidth = (float)getFont().getWidth(string);
}

boolean			HUDuiTypeIn::doKeyPress(const BzfKeyEvent& key)
{
  static const char backspace = '\b';	// ^H
  static const char whitespace = ' ';

  char c = key.ascii;
  if (c == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      HUDui::setFocus(getPrev());
      return True;

    case BzfKeyEvent::Down:
      HUDui::setFocus(getNext());
      return True;

    case BzfKeyEvent::Delete:
      c = backspace;
      break;

    default:
      return False;
  }

  if (c == '\t') {
    HUDui::setFocus(getNext());
    return True;
  }
  if (!isprint(c) && c != backspace)
    return False;

  if (c == backspace) {
    if (string.getLength() == 0) goto noRoom;
    string.truncate(string.getLength() - 1);
    onSetFont();
  }
  else {
    if (isspace(c)) c = whitespace;
    if (string.getLength() == maxLength) goto noRoom;
    string.append(&c, 1);
    onSetFont();
  }
  return True;

noRoom:
  // ring bell?
  return True;
}

boolean			HUDuiTypeIn::doKeyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == '\t' || !isprint(key.ascii))	// ignore non-printing and tab
    return False;

  // slurp up releases
  return True;
}

void			HUDuiTypeIn::doRender()
{
  if (!getFont().isValid()) return;

  // render string
  glColor3fv(hasFocus() ? textColor : dimTextColor);
  getFont().draw(string, getX(), getY());

  // render cursor
  if (HUDui::getFocus() == this)
    getFont().draw("_", getX() + stringWidth, getY());
}

//
// HUDuiLabel
//

HUDuiLabel::HUDuiLabel() : HUDuiControl()
{
}

HUDuiLabel::~HUDuiLabel()
{
}

BzfString		HUDuiLabel::getString() const
{
  return string;
}

void			HUDuiLabel::setString(const BzfString& _string)
{
  string = _string;
  onSetFont();
}

void			HUDuiLabel::onSetFont()
{
  HUDuiControl::onSetFont();
}

boolean			HUDuiLabel::doKeyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      HUDui::setFocus(getPrev());
      break;

    case BzfKeyEvent::Down:
      HUDui::setFocus(getNext());
      break;

    default:
      return False;
  }

  if (key.ascii == '\t') {
    HUDui::setFocus(getNext());
    return True;
  }

  switch (key.ascii) {
    case 13:
    case 27:
      return False;
  }
  return True;
}

boolean			HUDuiLabel::doKeyRelease(const BzfKeyEvent&)
{
  return False;
}

void			HUDuiLabel::doRender()
{
  if (!getFont().isValid()) return;

  // render string
  glColor3fv(hasFocus() ? textColor : dimTextColor);
  getFont().draw(string, getX(), getY());
}

//
// HUDuiTextureLabel
//

HUDuiTextureLabel::HUDuiTextureLabel() : HUDuiLabel()
{
}

HUDuiTextureLabel::~HUDuiTextureLabel()
{
}

void			HUDuiTextureLabel::setTexture(const OpenGLTexture& t)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(t);
  builder.setBlending();
  gstate = builder.getState();
}

void			HUDuiTextureLabel::doRender()
{
  if (!getFont().isValid()) return;

  // render string if texture filter is Off, otherwise draw the texture
  // about the same size and position as the string would be.
  if (OpenGLTexture::getFilter() == OpenGLTexture::Off || !gstate.isTextured()) {
    HUDuiLabel::doRender();
  }
  else {
    const OpenGLTexFont& font = getFont();
    const float width = font.getWidth(getString());
    const float height = font.getHeight();
    const float descent = font.getDescent();
    const float x = getX();
    const float y = getY();
    gstate.setState();
    glColor3fv(textColor);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(x, y - descent);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(x + width, y - descent);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(x + width, y - descent + height);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(x, y - descent + height);
    glEnd();
  }
}
// ex: shiftwidth=2 tabstop=8
