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
#include "HUDuiTabbedControl.h"

// common implementation headers
#include "HUDNavigationQueue.h"
#include "LocalFontFace.h"
#include "3D/FontManager.h"

#include "ogl/OpenGLUtils.h"

#include <algorithm>

//
// HUDuiTabbedControl
//

HUDuiTabbedControl::HUDuiTabbedControl() : HUDuiNestedContainer() {
  showFocus(false);
  getNav().push_front(this);
}

HUDuiTabbedControl::~HUDuiTabbedControl() {
  // do nothing
}

void HUDuiTabbedControl::addControl(HUDuiControl* control) {
  if ((getNav().size() == 1) && (hasFocus())) {
    getNav().set((size_t) 0);
  }
  control->setNavQueue(&getNav());
  control->isNested(true);
  control->setParent(this);
}

void HUDuiTabbedControl::addTab(HUDuiControl* tabControl, std::string tabName, size_t index) {
  if (index >= tabs.size()) {
    index = tabs.size();
  }

  std::vector<std::pair<std::string, HUDuiControl*> >::iterator it = tabs.begin();
  std::advance(it, index);

  tabs.insert(it, std::pair<std::string, HUDuiControl*>(tabName, tabControl));

  if (tabs.size() == 1) { // First tab
    getNav().push_back(tabControl);
    tabNavQueuePosition = getNav().begin() + ((getNav().size()) - 1);
  }

  tabControl->setFontFace(getFontFace());
  tabControl->setFontSize(getFontSize());
  tabControl->setSize(getWidth(), getHeight() - tabsHeight - tabsHeight / 2);
  tabControl->setPosition(getX(), getY());

  setActiveTab(index);

  addControl(tabControl);
}

void HUDuiTabbedControl::setActiveTab(size_t tab) {
  if (tab < tabs.size()) {
    activeTab = tab;
    activeControl = tabs[activeTab].second;
    tabNavQueuePosition = getNav().erase(tabNavQueuePosition);
    tabNavQueuePosition = getNav().insert(tabNavQueuePosition, activeControl);
  }
}

void HUDuiTabbedControl::removeTab(size_t tabIndex) {
  if (tabIndex >= tabs.size()) {
    return;
  }

  tabs.erase(tabs.begin() + tabIndex);

  if (activeTab >= tabs.size()) {
    activeTab = tabs.size() - 1;
  }

  setActiveTab(activeTab);
  getNav().set((size_t)0);
}

void HUDuiTabbedControl::removeTab(HUDuiControl* control, std::string tabName) {
  std::vector<std::pair<std::string, HUDuiControl*> >::iterator it = std::find(tabs.begin(), tabs.end(), std::pair<std::string, HUDuiControl*>(tabName, control));

  if (it != tabs.end()) {
    tabs.erase(it);
  }

  if (activeTab >= tabs.size()) {
    activeTab = tabs.size() - 1;
  }

  setActiveTab(activeTab);
  getNav().set((size_t)0);
}

void HUDuiTabbedControl::setSize(float width, float height) {
  HUDuiNestedContainer::setSize(width, height);
  FontManager& fm = FontManager::instance();

  tabsHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());

  for (size_t i = 0; i < tabs.size(); i++) {
    tabs[i].second->setSize(getWidth(), getHeight() - tabsHeight - tabsHeight / 2);
  }
}

void HUDuiTabbedControl::setFontSize(float size) {
  HUDuiNestedContainer::setFontSize(size);
  FontManager& fm = FontManager::instance();

  tabsHeight = fm.getStringHeight(getFontFace()->getFMFace(), size);

  for (size_t i = 0; i < tabs.size(); i++) {
    tabs[i].second->setFontSize(size);
    tabs[i].second->setSize(getWidth(), getHeight() - tabsHeight - tabsHeight / 2);
  }
}

void HUDuiTabbedControl::setFontFace(const LocalFontFace* face) {
  HUDuiNestedContainer::setFontFace(face);

  for (size_t i = 0; i < tabs.size(); i++) {
    tabs[i].second->setFontFace(face);
  }
}

void HUDuiTabbedControl::setPosition(float x, float y) {
  HUDuiNestedContainer::setPosition(x, y);

  for (size_t i = 0; i < tabs.size(); i++) {
    tabs[i].second->setPosition(x, y);
  }
}

void HUDuiTabbedControl::drawTabBody() {
  // do nothing
}

void HUDuiTabbedControl::drawTabs() {
  FontManager& fm = FontManager::instance();

  const fvec4 color(1.0f, 1.0f, 1.0f, 1.0f);
  const fvec4 activeColor(0.0f, 1.0f, 0.0f, 1.0f);

  glColor4fv(color);

  float sideSpacer = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "X");

  float x = getX();
  float y = (getY() + (getHeight() - tabsHeight));

  for (size_t i = 0; i < tabs.size(); i++) {
    const char* text = tabs[i].first.c_str();

    x = x + sideSpacer;
    if (activeTab == i) {
      fm.drawString(x, y, 0, getFontFace()->getFMFace(), getFontSize(), text, &activeColor);
    }
    else {
      fm.drawString(x, y, 0, getFontFace()->getFMFace(), getFontSize(), text);
    }

    x = x + fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), text) + sideSpacer;

    glOutlineBoxHV(1.0f, getX(), y - tabsHeight / 2, x + 1, getY() + getHeight() + 1, -0.5f);
  }

  glOutlineBoxHV(1.0f, getX(), y - tabsHeight / 2, x + 1, getY() + getHeight() + 1, -0.5f);
}

void HUDuiTabbedControl::doRender() {
  if (getFontFace() < 0) {
    return;
  }

  drawTabs(); // Draw the tabs
  drawTabBody(); // Draw the active tab body

  // Draw the actual tab
  HUDuiControl* active = NULL;
  active = tabs[activeTab].second;
  if (active == NULL) {
    return;
  }
  active->render();
}

bool HUDuiTabbedControl::doKeyPress(const BzfKeyEvent& key) {
  if (key.unicode == 0) {
    switch (key.button) {
      case BzfKeyEvent::Left: {
        if (hasFocus()) {
          setActiveTab(activeTab - 1);
        }
        break;
      }
      case BzfKeyEvent::Right: {
        if (hasFocus()) {
          setActiveTab(activeTab + 1);
        }
        break;
      }
      case BzfKeyEvent::Down: {
        if (hasFocus()) {
          getNav().next();
        }
        break;
      }
      default: {
        return false;
      }
    }
  }

  // Does this do anything?
  switch (key.unicode) {
    case 27:
      return false;
  }

  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
