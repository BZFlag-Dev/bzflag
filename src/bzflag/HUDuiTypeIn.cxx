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

// interface headers
#include "HUDuiTypeIn.h"

// system implementation headers
#include <ctype.h>

// common implementation headers
#include "FontManager.h"

// local implementation headers
#include "HUDui.h"

//
// HUDuiTypeIn
//

HUDuiTypeIn::HUDuiTypeIn()
: HUDuiControl()
, maxLength(0)
, cursorPos(0)
, allowEdit(true)
, obfuscate(false)
, colorFunc(NULL)
{
}

HUDuiTypeIn::~HUDuiTypeIn()
{
}

void		HUDuiTypeIn::setObfuscation(bool on)
{
  obfuscate = on;
}

int			HUDuiTypeIn::getMaxLength() const
{
  return maxLength;
}

std::string		HUDuiTypeIn::getString() const
{
  return string;
}

void			HUDuiTypeIn::setMaxLength(int _maxLength)
{
  maxLength = _maxLength;
  string = string.substr(0, maxLength);
  if (cursorPos > maxLength)
    cursorPos = maxLength;
  onSetFont();
}

void			HUDuiTypeIn::setString(const std::string& _string)
{
  string = _string;
  cursorPos = string.length();
  onSetFont();
}

// allows composing, otherwise not
void			HUDuiTypeIn::setEditing(bool _allowEdit)
{
  allowEdit = _allowEdit;
}

bool			HUDuiTypeIn::doKeyPress(const BzfKeyEvent& key)
{
  static const char backspace = '\b';	// ^H
  static const char whitespace = ' ';

  if (!allowEdit) return false; //or return true ??
  char c = key.ascii;
  if (c == 0){
		switch (key.button) {
		case BzfKeyEvent::Up:
		  HUDui::setFocus(getPrev());
		  return true;

		case BzfKeyEvent::Down:
		  HUDui::setFocus(getNext());
		  return true;

		case BzfKeyEvent::Left:
		  if (cursorPos > 0)
		cursorPos--;
		  return true;

		case BzfKeyEvent::Right:
		  if (cursorPos < (int)string.length())
		cursorPos++;
		  return true;

		case BzfKeyEvent::Home:
		  cursorPos = 0;
		  return true;

		case BzfKeyEvent::End:
		  cursorPos = string.length();
		  return true;

		case BzfKeyEvent::Backspace:
		  c = backspace;
		  break;

		case BzfKeyEvent::Delete:
		  if (cursorPos < (int)string.length()) {
		cursorPos++;
		c = backspace;
		  } else {
		return true;
		  }
		  break;

		default:
		  return false;
	  }
  }

  if (c == '\t') {
    HUDui::setFocus(getNext());
    return true;
  }
  if (c >0 && (!isprint(c) && c != backspace))
    return false;

  if (c == backspace) {
    if (cursorPos == 0)
		return true;

    cursorPos--;
    string = string.substr(0, cursorPos) + string.substr(cursorPos + 1, string.length() - cursorPos + 1);
    onSetFont();
  } else if (c > 0) {
    if (isspace(c))
		c = whitespace;
    if ((int)string.length() == maxLength)
		return true;

    string = string.substr(0, cursorPos) + c + string.substr( cursorPos, string.length() - cursorPos);
    cursorPos++;
    onSetFont();
  }
  return true;
}

bool			HUDuiTypeIn::doKeyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == '\t' || !isprint(key.ascii))	// ignore non-printing and tab
    return false;

  // slurp up releases
  return true;
}

void			HUDuiTypeIn::doRender()
{
  if (getFontFace() < 0) return;

  // render string
  glColor3fv(hasFocus() ? textColor : dimTextColor);

  FontManager &fm = FontManager::instance();
  std::string renderStr;
  if (obfuscate) {
    renderStr.append(string.size(), '*');
  } else {
    renderStr = string;
  }
  if (colorFunc) {
    renderStr = colorFunc(renderStr);
  }
  fm.drawString(getX(), getY(), 0, getFontFace(), getFontSize(), renderStr);

  // find the position of where to draw the input cursor
  const std::string noAnsi = stripAnsiCodes(renderStr);
  float start = fm.getStrLength(getFontFace(), getFontSize(), noAnsi.substr(0, cursorPos));

  if (HUDui::getFocus() == this && allowEdit) {
    fm.drawString(getX() + start, getY(), 0, getFontFace(), getFontSize(), "_");
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

