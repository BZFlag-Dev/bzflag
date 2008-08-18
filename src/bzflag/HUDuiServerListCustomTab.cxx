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
#include "HUDuiServerListCustomTab.h"

#include "playing.h"

#include "FontManager.h"
#include "FontSizer.h"
#include "LocalFontFace.h"

#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"
#include "HUDuiList.h"
#include "OpenGLUtils.h"

//
// HUDuiServerListCustomTab
//

HUDuiServerListCustomTab::HUDuiServerListCustomTab() : HUDuiNestedContainer()
{
  tabName = new HUDuiTypeIn();
  tabName->setLabel("Enter Tab Name:");
  tabName->setString("New Tab");
  tabName->setMaxLength(16);

  domainName = new HUDuiTypeIn();
  domainName->setLabel("Domain Name Filter:");
  domainName->setString("*");
  domainName->setMaxLength(32);

  serverName = new HUDuiTypeIn();
  serverName->setLabel("Server Name Filter:");
  serverName->setString("*");
  serverName->setMaxLength(32);

  emptyServer = new HUDuiList();
  emptyServer->setLabel("Show Empty Servers:");
  emptyServer->getList().push_back("True");
  emptyServer->getList().push_back("False");
  emptyServer->update();

  fullServer = new HUDuiList();
  fullServer->setLabel("Show Full Servers:");
  fullServer->getList().push_back("True");
  fullServer->getList().push_back("False");
  fullServer->update();

  ricochet = new HUDuiList();
  ricochet->setLabel("Ricochet:");
  ricochet->getList().push_back("On");
  ricochet->getList().push_back("Off");
  ricochet->getList().push_back("Either");
  ricochet->update();

  superFlags = new HUDuiList();
  superFlags->setLabel("Super Flags:");
  superFlags->getList().push_back("On");
  superFlags->getList().push_back("Off");
  superFlags->getList().push_back("Either");
  superFlags->update();

  jumping = new HUDuiList();
  jumping->setLabel("Jumping:");
  jumping->getList().push_back("On");
  jumping->getList().push_back("Off");
  jumping->getList().push_back("Either");
  jumping->update();

  createNew = new HUDuiLabel();
  createNew->setString("Create Tab");

  addControl(tabName);
  addControl(domainName);
  addControl(serverName);
  addControl(emptyServer);
  addControl(fullServer);
  addControl(ricochet);
  addControl(superFlags);
  addControl(jumping);
  addControl(createNew);

  getNav().addCallback(callback, this);
}

HUDuiServerListCustomTab::~HUDuiServerListCustomTab()
{
  getNav().removeCallback(callback, this);
}

size_t HUDuiServerListCustomTab::callback(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* data)
{
  if ((oldFocus == 0)&&(changeMethod == hnPrev))
  {
    ((HUDuiServerListCustomTab*)data)->getNavList()->prev();
    if (((HUDuiServerListCustomTab*)data)->getNavList()->get()->isContainer())
      ((HUDuiNestedContainer*)((HUDuiServerListCustomTab*)data)->getNavList()->get())->getNav().set(((HUDuiServerListCustomTab*)data)->getNavList()->get());
    return HUDNavigationQueue::SkipSetFocus;
  }
  
  if ((oldFocus == ((HUDuiServerListCustomTab*)data)->getNav().size() - 1)&&(changeMethod == hnNext)) proposedFocus = oldFocus;

  return proposedFocus;
}

void HUDuiServerListCustomTab::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height); 

  resize();
}

void HUDuiServerListCustomTab::setFontSize(float size)
{
  HUDuiControl::setFontSize(size); 

  resize();
}

void HUDuiServerListCustomTab::setFontFace(const LocalFontFace* face)
{
  HUDuiControl::setFontFace(face); 

  resize();
}

void HUDuiServerListCustomTab::setPosition(float x, float y)
{
  HUDuiControl::setPosition(x, y);

  resize();
}

void HUDuiServerListCustomTab::resize()
{
  FontManager &fm = FontManager::instance();

  float fontHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());

  float y = getY() + getHeight() - fontHeight;

  float spacer = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "XXXI");

  float width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tabName->getLabel().c_str());

  tabName->setFontFace(getFontFace());
  tabName->setFontSize(getFontSize());
  tabName->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), domainName->getLabel().c_str());

  domainName->setFontFace(getFontFace());
  domainName->setFontSize(getFontSize());
  domainName->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), serverName->getLabel().c_str());

  serverName->setFontFace(getFontFace());
  serverName->setFontSize(getFontSize());
  serverName->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), emptyServer->getLabel().c_str());

  emptyServer->setFontFace(getFontFace());
  emptyServer->setFontSize(getFontSize());
  emptyServer->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), fullServer->getLabel().c_str());

  fullServer->setFontFace(getFontFace());
  fullServer->setFontSize(getFontSize());
  fullServer->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), ricochet->getLabel().c_str());

  ricochet->setFontFace(getFontFace());
  ricochet->setFontSize(getFontSize());
  ricochet->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), superFlags->getLabel().c_str());

  superFlags->setFontFace(getFontFace());
  superFlags->setFontSize(getFontSize());
  superFlags->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), jumping->getLabel().c_str());

  jumping->setFontFace(getFontFace());
  jumping->setFontSize(getFontSize());
  jumping->setPosition(getX() + width + spacer, y);

  y = y - fontHeight;

  createNew->setFontFace(getFontFace());
  createNew->setFontSize(getFontSize());
  createNew->setPosition(getX() + spacer, y);
}

HUDuiServerList* HUDuiServerListCustomTab::createServerList()
{
  HUDuiServerList* newServerList = new HUDuiServerList;
  if (emptyServer->getList().at(emptyServer->getIndex()) == "False")
    newServerList->toggleFilter(HUDuiServerList::EmptyServer);
  if (fullServer->getList().at(fullServer->getIndex()) == "False")
    newServerList->toggleFilter(HUDuiServerList::FullServer);
  if (jumping->getList().at(jumping->getIndex()) == "On")
    newServerList->toggleFilter(HUDuiServerList::JumpingOn);
  if (jumping->getList().at(jumping->getIndex()) == "Off")
    newServerList->toggleFilter(HUDuiServerList::JumpingOff);
  if (ricochet->getList().at(ricochet->getIndex()) == "On")
    newServerList->toggleFilter(HUDuiServerList::RicochetOn);
  if (ricochet->getList().at(ricochet->getIndex()) == "Off")
    newServerList->toggleFilter(HUDuiServerList::RicochetOff);
  return newServerList;
}

void HUDuiServerListCustomTab::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  glColor4fv(color);

  glOutlineBoxHV(1.0f, getX(), getY(), getX() + getWidth(), getY() + getHeight() + 1, -0.5f);

  tabName->render();
  domainName->render();
  serverName->render();
  emptyServer->render();
  fullServer->render();
  ricochet->render();
  superFlags->render();
  jumping->render();
  createNew->render();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
