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
#include "HUDuiTabbedControl.h"

// common implementation headers
#include "HUDNavigationQueue.h"
#include "FontManager.h"
#include "LocalFontFace.h"

#include "OpenGLGState.h"
#include "bzfgl.h"

//
// HUDuiTabbedControl
//

HUDuiTabbedControl::HUDuiTabbedControl() : HUDuiNestedContainer()
{
  showFocus(false);
  getNav().push_front(this);
}

HUDuiTabbedControl::~HUDuiTabbedControl()
{
  // clean up
  //navList->removeCallback(gotFocus, this);
}

void HUDuiTabbedControl::addControl(HUDuiControl *control)
{
  //HUDuiNestedContainer::addControl(control);
  //nestedNavList.push_back(control);
  if ((getNav().size() == 1)&&(hasFocus()))
    getNav().set((size_t) 0);
  control->setNavQueue(&getNav());
  control->isNested(true);
  control->setParent(this);
}

void HUDuiTabbedControl::addTab(HUDuiControl* tabControl, std::string tabName)
{
  tabs.push_back(std::pair<std::string, HUDuiControl*>(tabName, tabControl));
  //activeTab = (int) tabs.size() - 1;

  if (tabs.size() == 1) // First tab
    getNav().push_back(tabControl);
    tabNavQueuePosition = getNav().begin() + (((int)getNav().size()) - 1);

  setActiveTab((int) tabs.size() - 1);

  addControl(tabControl);
}

void HUDuiTabbedControl::setActiveTab(int tab)
{
  if ((tab >= 0)&&(tab < (int) tabs.size()))
  {
    activeTab = tab;
    activeControl = tabs[activeTab].second;
    tabNavQueuePosition = getNav().erase(tabNavQueuePosition);
    tabNavQueuePosition = getNav().insert(tabNavQueuePosition, activeControl);
  }
}

HUDuiControl* HUDuiTabbedControl::getActiveTab()
{
  return activeControl;
}

void HUDuiTabbedControl::removeTab(int tabIndex)
{
  if (tabIndex >= (int) tabs.size())
    return;

  tabs.erase(tabs.begin() + tabIndex);
}

void HUDuiTabbedControl::setSize(float width, float height)
{
  HUDuiNestedContainer::setSize(width, height);
  FontManager &fm = FontManager::instance();

  float tabsHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());

  for (int i=0; i<(int)tabs.size(); i++)
  {
    tabs[i].second->setSize(getWidth(), getHeight() - 2*tabsHeight);
  }
}

void HUDuiTabbedControl::setFontSize(float size)
{
  HUDuiNestedContainer::setFontSize(size);
  FontManager &fm = FontManager::instance();

  float tabsHeight = fm.getStringHeight(getFontFace()->getFMFace(), size);

  for (int i=0; i<(int)tabs.size(); i++)
  {
    tabs[i].second->setFontSize(size);
    tabs[i].second->setSize(getWidth(), getHeight() - 2*tabsHeight);
  }
}

void HUDuiTabbedControl::setFontFace(const LocalFontFace* face)
{
  HUDuiNestedContainer::setFontFace(face);

  for (int i=0; i<(int)tabs.size(); i++)
  {
    tabs[i].second->setFontFace(face);
  }
}

void HUDuiTabbedControl::setPosition(float x, float y)
{
  HUDuiNestedContainer::setPosition(x, y);

  for (int i=0; i<(int)tabs.size(); i++)
  {
    tabs[i].second->setPosition(x, y);
  }
}

void HUDuiTabbedControl::drawTabBody()
{
  // do nothing
}

void HUDuiTabbedControl::drawTabs()
{
  FontManager &fm = FontManager::instance();
  float color[4];

  color[0] = 1.0f;
  color[1] = 1.0f;
  color[2] = 1.0f;
  color[3] = 1.0f;

  float activeColor[3];

  activeColor[0] = 0.0f;
  activeColor[1] = 1.0f;
  activeColor[2] = 0.0f;

  OpenGLGState::resetState();  // fixme: shouldn't be needed
  glLineWidth(1.0f);
  glColor4fv(color);
  
  // Draw top tabs
  float tabsHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());

  float sideSpacer = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "X");
  float topSpacer = tabsHeight/2;

  float x = getX();
  float y = (getY() + (getHeight() - tabsHeight));

  for (int i=0; i<(int)tabs.size(); i++)
  {
    const char* text = tabs[i].first.c_str();

    x = x + sideSpacer;
    if (activeTab == i)
      fm.drawString(x, y - topSpacer, 0, getFontFace()->getFMFace(), getFontSize(), text, activeColor);
    else
      fm.drawString(x, y - topSpacer, 0, getFontFace()->getFMFace(), getFontSize(), text);
		
    x = x + fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), text) + sideSpacer;

    glBegin(GL_LINES);
    glVertex2f(x, getY() + getHeight()); // Top vertex of divider
    glVertex2f(x, y - 2*topSpacer); // Bottom vertex of divider
    glEnd();

  }

  glBegin(GL_LINES);

  glVertex2f(getX(), (getY() + getHeight())); // Top line left vertex
  glVertex2f(x, (getY() + getHeight())); // Top line right vertex

  glVertex2f(getX(), (getY() + getHeight() - tabsHeight - 2*topSpacer)); // Bottom line left vertex
  glVertex2f(x, (getY() + getHeight() - tabsHeight - 2*topSpacer)); // Bottom line right vertex

  glVertex2f(getX(), (getY() + getHeight())); // Left line top vertex
  glVertex2f(getX(), (getY() + getHeight() - tabsHeight - 2*topSpacer)); // Left line bottom vertex

  glEnd();
}

void HUDuiTabbedControl::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  drawTabs(); // Draw the tabs
  drawTabBody(); // Draw the active tab body

  // Draw the actual tab
  HUDuiControl* active = NULL;
  active = tabs[activeTab].second;
  if (active == NULL)
    return;
  active->render();
}

bool HUDuiTabbedControl::doKeyPress(const BzfKeyEvent& key)
{
  if (key.chr == 0)
    switch (key.button) {
      case BzfKeyEvent::Left:
	if (hasFocus())
	  setActiveTab(activeTab - 1);
        break;

      case BzfKeyEvent::Right:
	if (hasFocus())
	  setActiveTab(activeTab + 1);
        break;

      case BzfKeyEvent::Down:
	if (hasFocus())
	  getNav().next();
	break;

      default:
        return false;
  }

  switch (key.chr) {
    case 13:
    case 27:
      return false;
  }

  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8