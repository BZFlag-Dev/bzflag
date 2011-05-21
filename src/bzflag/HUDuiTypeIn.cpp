/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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

// system headers
#include <ctype.h>

// common headers
#include "Clipboard.h"
#include "FontManager.h"
#include "KeyManager.h"
#include "bzUnicode.h"

// local headers
#include "LocalFontFace.h"
#include "guiplaying.h"


//
// HUDuiTypeIn
//

HUDuiTypeIn::HUDuiTypeIn()
  : HUDuiControl()
  , maxLength(0)
  , cursorPos(data.c_str()) {
  allowEdit = true; // allow editing by default
  obfuscate = false;
}


HUDuiTypeIn::~HUDuiTypeIn() {
}


void HUDuiTypeIn::setObfuscation(bool on) {
  obfuscate = on;
}


size_t HUDuiTypeIn::getMaxLength() const {
  return maxLength;
}


std::string HUDuiTypeIn::getString() const {
  return data;
}


void HUDuiTypeIn::setMaxLength(size_t _maxLength) {
  maxLength = _maxLength;
  setString(data.substr(0, maxLength));
  onSetFont();
}


void HUDuiTypeIn::setString(const std::string& _string) {
  data = _string;
  cursorPos = data.c_str();
  while (*cursorPos) {
    ++cursorPos;
  }
  onSetFont();
}


// allows composing, otherwise not
void HUDuiTypeIn::setEditing(bool _allowEdit) {
  allowEdit = _allowEdit;
}


bool HUDuiTypeIn::decrementCursor() {
  size_t pos = cursorPos.getCount();
  if (pos <= 1) {
    return false;
  }
  --pos;
  cursorPos = data.c_str(); // reset the cursor
  while ((cursorPos.getCount() < pos) && (*cursorPos)) {
    ++cursorPos;
  }
  return true;
}


void HUDuiTypeIn::pasteText(const std::string& text) {
  if (text.empty()) {
    return;
  }

  bool changed = false;
  for (size_t i = 0; i < text.size(); i++) {
    changed = doInsert(text[i]) || changed;
  }

  if (changed) {
    onSetFont();
    if (cb != NULL) {
      cb(this, userData);
    }
  }
}


bool HUDuiTypeIn::doKeyPress(const BzfKeyEvent& key) {
  static unsigned int backspace = '\b'; // ^H

  if (HUDuiControl::doKeyPress(key)) {
    return true;
  }

  if (!allowEdit) {
    return false; // or return true ??
  }

  const std::string& cmd = KEYMGR.get(key, true);
  if (cmd == "paste") {
    pasteText(getClipboard());
    return true;
  }
  else if ((cmd.size() > 6) && (cmd.substr(0, 6) == "paste ")) {
    pasteText(cmd.substr(6));
    return true;
  }

  unsigned int c = key.unicode;

  if (c == 0) {
    switch (key.button) {
      case BzfKeyEvent::Left: {
        if ((key.modifiers & BzfKeyEvent::AltKey) != 0) {
          return false;
        }
        if ((key.modifiers & BzfKeyEvent::ControlKey) == 0) {
          // skip a character
          decrementCursor();
        }
        else {
          // skip a word
          while (decrementCursor() &&  iswspace(*cursorPos)) {}
          while (decrementCursor() && !iswspace(*cursorPos)) {}
          if (iswspace(*cursorPos)) {
            ++cursorPos; // move into the first character of the word
          }
        }
        return true;
      }
      case BzfKeyEvent::Right: {
        if ((key.modifiers & BzfKeyEvent::AltKey) != 0) {
          return false;
        }
        if ((key.modifiers & BzfKeyEvent::ControlKey) == 0) {
          // skip a character
          if (*cursorPos) {
            ++cursorPos;
          }
        }
        else {
          // skip a word
          while (*cursorPos &&  iswspace(*cursorPos)) { ++cursorPos; }
          while (*cursorPos && !iswspace(*cursorPos)) { ++cursorPos; }
        }
        return true;
      }
      case BzfKeyEvent::Home: {
        cursorPos = data.c_str();
        return true;
      }
      case BzfKeyEvent::End: {
        while (*cursorPos) {
          ++cursorPos;
        }
        return true;
      }
      case BzfKeyEvent::Backspace: {
        c = backspace;
        break;
      }
      case BzfKeyEvent::Delete: {
        if (*cursorPos) {
          ++cursorPos;
          c = backspace;
        }
        else {
          return true;
        }
        break;
      }
      default: {
        return false;
      }
    }
  }

  if (!iswprint(c) && (c != backspace)) {
    return false;
  }

  bool changed = false;
  if (c == backspace) {
    changed = doBackspace();
  }
  else {
    changed = doInsert(c);
  }

  if (changed) {
    onSetFont();
    if (cb != NULL) {
      cb(this, userData);
    }
  }

  return true;
}


bool HUDuiTypeIn::doInsert(unsigned int c) {
  if (iswspace(c)) {
    c = ' ';
  }
  CountUTF8StringItr cusi(data.c_str());
  while (*cusi) { ++cusi; }
  if (cusi.getCount() >= maxLength) {
    return false;
  }
  bzUTF8Char ch(c);
  size_t pos = (cursorPos.getBufferFromHere() - data.c_str());
  // copy to the current cursor location
  std::string temp = data.substr(0, pos);
  // insert the new character
  temp += ch.str();
  // copy the rest of the string
  temp += data.substr(pos, data.length());
  data = temp;
  // new buffer, restart cursor
  pos = cursorPos.getCount();
  cursorPos = data.c_str();
  while (cursorPos.getCount() < pos) {
    ++cursorPos;
  }
  // bump the cursor
  ++cursorPos;

  if (cb != NULL) {
    cb(this, userData);
  }

  return true;
}

bool HUDuiTypeIn::doBackspace() {
  size_t pos = cursorPos.getCount();
  if (pos == 1) {
    return false;
  }

  // copy up to cursor position - 1
  --pos;
  cursorPos = data.c_str();
  while (cursorPos.getCount() < pos) {
    ++cursorPos;
  }
  const char* s = data.c_str();
  const char* a = cursorPos.getBufferFromHere();
  ++cursorPos;
  const char* b = cursorPos.getBufferFromHere();

  // remove the character from the data
  const std::string tmpData = data.substr(0, (a - s)) + data.substr(b - s);
  data = tmpData;

  // new buffer, restart cursor
  cursorPos = data.c_str();
  while (cursorPos.getCount() < pos) {
    ++cursorPos;
  }

  if (cb != NULL) {
    cb(this, userData);
  }

  return true;
}


bool HUDuiTypeIn::doKeyRelease(const BzfKeyEvent& key) {
  const std::string& cmd = KEYMGR.get(key, false);
  if (cmd == "paste") {
    pasteText(getClipboard());
    return true;
  }
  else if ((cmd.size() > 6) && (cmd.substr(0, 6) == "paste ")) {
    pasteText(cmd.substr(6));
    return true;
  }

  if (key.unicode == '\t' || !iswprint(key.unicode)) {
    return false; // ignore non-printing and tab
  }

  return true; // slurp up releases
}


void HUDuiTypeIn::doRender() {
  if (getFontFace() == NULL) {
    return;
  }
  const int faceID = getFontFace()->getFMFace();

  // render string
  glColor3fv(hasFocus() ? textColor : dimTextColor);

  FontManager& fm = FontManager::instance();
  std::string renderStr;
  if (obfuscate) {
    CountUTF8StringItr cusi(data.c_str());
    while (*cusi) { ++cusi; }
    renderStr.append(cusi.getCount(), '*');
  }
  else {
    renderStr = data;
  }

  fm.drawString(getX(), getY(), 0, faceID, getFontSize(), renderStr);

  // find the position of where to draw the input cursor
  float start = fm.getStringWidth(faceID, getFontSize(),
                                  renderStr.substr(0, cursorPos.getBufferFromHere() - data.c_str()));

  if (hasFocus() && allowEdit) {
    fm.drawString(getX() + start, getY(), 0, faceID, getFontSize(), "_");
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
