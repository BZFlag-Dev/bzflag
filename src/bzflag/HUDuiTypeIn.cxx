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

// interface headers
#include "HUDuiTypeIn.h"

// system implementation headers
#include <ctype.h>

// common implementation headers
#include "FontManager.h"
#include "bzUnicode.h"

//
// HUDuiTypeIn
//

HUDuiTypeIn::HUDuiTypeIn()
: HUDuiControl(), maxLength(0), cursorPos(string.c_str())
{
  allowEdit = true; // allow editing by default
  obfuscate = false;
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
  if (cursorPos.getCount() > maxLength)
  {
    cursorPos = string.c_str();
    while (*cursorPos)
      ++cursorPos;
  }
  onSetFont();
}

void			HUDuiTypeIn::setString(const std::string& _string)
{
  string = _string;
  cursorPos = string.c_str();
  while (*cursorPos)
    ++cursorPos;
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

  if (HUDuiControl::doKeyPress(key))
    return true;

  if (!allowEdit) return false; //or return true ??
  unsigned int c = key.chr;
  if (c == 0) switch (key.button) {
    case BzfKeyEvent::Left: {
      int pos = cursorPos.getCount();
      // uhh...there's not really any way to reverse over a multibyte string
      // do this the hard way: reset to the beginning and advance to the current
      // position, minus a character.
      if (pos > 0) {
	--pos;
	cursorPos = string.c_str();
	while (cursorPos.getCount() < pos && (*cursorPos))
	  ++cursorPos;
      }
      return true;
    }

    case BzfKeyEvent::Right:
      if (*cursorPos)
	++cursorPos;
      return true;

    case BzfKeyEvent::Home:
      cursorPos = string.c_str();
      return true;

    case BzfKeyEvent::End:
      while (*cursorPos)
        ++cursorPos;
      return true;

    case BzfKeyEvent::Backspace:
      c = backspace;
      break;

    case BzfKeyEvent::Delete:
      if (*cursorPos) {
	++cursorPos;
	c = backspace;
      } else {
	return true;
      }
      break;

    default:
      return false;
  }

  if (!iswprint(c) && c != backspace)
    return false;

  if (c == backspace) {
    int pos = cursorPos.getCount();
    if (pos == 1) { 
      goto noRoom;
    } else {
      // copy up to cursor position - 1
      cursorPos = string.c_str();
      --pos;
      while (cursorPos.getCount() < pos)
        ++cursorPos;
      std::string temp = string.substr(0, cursorPos.getBufferFromHere() - string.c_str());
      // skip the deleted character
      ++cursorPos;
      // copy the remainder
      pos = (int)(cursorPos.getBufferFromHere() - string.c_str());
      temp += string.substr(pos, string.length() - pos);
      string = temp;
      // new buffer, restart cursor
      pos = cursorPos.getCount();
      cursorPos = string.c_str();
      while (cursorPos.getCount() < (pos - 1))
	++cursorPos;
    }

    onSetFont();
  } else {
    if (iswspace(c))
      c = whitespace;

    CountUTF8StringItr cusi(string.c_str());
    while (*cusi) ++cusi;
    if (cusi.getCount() >= maxLength) goto noRoom;

    UTF8Char ch(c);
    int pos = (int)(cursorPos.getBufferFromHere() - string.c_str());
    // copy to the current cursor location
    std::string temp = string.substr(0, pos);
    // insert the new character
    temp += ch.str();
    // copy the rest of the string
    temp += string.substr(pos, string.length());
    string = temp;
    // new buffer, restart cursor
    pos = cursorPos.getCount();
    cursorPos = string.c_str();
    while (cursorPos.getCount() < pos)
      ++cursorPos;

    // bump the cursor
    ++cursorPos;
    onSetFont();
  }
  return true;

noRoom:
  // ring bell?
  return true;
}

bool			HUDuiTypeIn::doKeyRelease(const BzfKeyEvent& key)
{
  if (key.chr == '\t' || !iswprint(key.chr))	// ignore non-printing and tab
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
  fm.drawString(getX(), getY(), 0, getFontFace(), getFontSize(), renderStr.c_str());

  // find the position of where to draw the input cursor
  float start = fm.getStringWidth(getFontFace(), getFontSize(), 
    renderStr.substr(0, cursorPos.getBufferFromHere() - string.c_str()).c_str());

  if (hasFocus() && allowEdit) {
    fm.drawString(getX() + start, getY(), 0, getFontFace(), getFontSize(), "_");
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
