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
#include "HUDuiServerListCustomTab.h"

#include "playing.h"

#include "3D/FontManager.h"
#include "FontSizer.h"
#include "LocalFontFace.h"

#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"
#include "HUDuiList.h"
#include "ogl/OpenGLUtils.h"

//
// HUDuiServerListCustomTab
//

HUDuiServerListCustomTab::HUDuiServerListCustomTab() : HUDuiNestedContainer() {
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
  emptyServer->getList().push_back("Yes");
  emptyServer->getList().push_back("No");
  emptyServer->update();

  fullServer = new HUDuiList();
  fullServer->setLabel("Show Full Servers:");
  fullServer->getList().push_back("Yes");
  fullServer->getList().push_back("No");
  fullServer->update();

  teamFFAServers = new HUDuiList();
  teamFFAServers->setLabel("Show Team Free-For-All Servers:");
  teamFFAServers->getList().push_back("Yes");
  teamFFAServers->getList().push_back("No");
  teamFFAServers->update();

  openFFAServers = new HUDuiList();
  openFFAServers->setLabel("Show Open Free-For-All Servers:");
  openFFAServers->getList().push_back("Yes");
  openFFAServers->getList().push_back("No");
  openFFAServers->update();

  classicCTFServers = new HUDuiList();
  classicCTFServers->setLabel("Show Capture The Flag Servers:");
  classicCTFServers->getList().push_back("Yes");
  classicCTFServers->getList().push_back("No");
  classicCTFServers->update();

  rabbitChaseServers = new HUDuiList();
  rabbitChaseServers->setLabel("Show Rabbit Chase Servers:");
  rabbitChaseServers->getList().push_back("Yes");
  rabbitChaseServers->getList().push_back("No");
  rabbitChaseServers->update();

  ricochet = new HUDuiList();
  ricochet->setLabel("Ricochet:");
  ricochet->getList().push_back("Either");
  ricochet->getList().push_back("On");
  ricochet->getList().push_back("Off");
  ricochet->update();

  superFlags = new HUDuiList();
  superFlags->setLabel("Super Flags:");
  superFlags->getList().push_back("Either");
  superFlags->getList().push_back("On");
  superFlags->getList().push_back("Off");
  superFlags->update();

  antidoteFlag = new HUDuiList();
  antidoteFlag->setLabel("Antidote Flags:");
  antidoteFlag->getList().push_back("Either");
  antidoteFlag->getList().push_back("On");
  antidoteFlag->getList().push_back("Off");
  antidoteFlag->update();

  jumping = new HUDuiList();
  jumping->setLabel("Jumping:");
  jumping->getList().push_back("Either");
  jumping->getList().push_back("On");
  jumping->getList().push_back("Off");
  jumping->update();

  handicap = new HUDuiList();
  handicap->setLabel("Handicap:");
  handicap->getList().push_back("Either");
  handicap->getList().push_back("On");
  handicap->getList().push_back("Off");
  handicap->update();

  luaWorld = new HUDuiList();
  luaWorld->setLabel("LuaWorld:");
  luaWorld->getList().push_back("Either");
  luaWorld->getList().push_back("On");
  luaWorld->getList().push_back("Off");
  luaWorld->update();

  luaRules = new HUDuiList();
  luaRules->setLabel("LuaRules:");
  luaRules->getList().push_back("Either");
  luaRules->getList().push_back("On");
  luaRules->getList().push_back("Off");
  luaRules->update();

  createNew = new HUDuiLabel();
  createNew->setString("-> Create Tab");

  addControl(tabName);
  addControl(domainName);
  addControl(serverName);
  addControl(emptyServer);
  addControl(fullServer);
  addControl(teamFFAServers);
  addControl(openFFAServers);
  addControl(classicCTFServers);
  addControl(rabbitChaseServers);
  addControl(ricochet);
  addControl(superFlags);
  addControl(antidoteFlag);
  addControl(jumping);
  addControl(handicap);
  addControl(luaWorld);
  addControl(luaRules);
  addControl(createNew);

  getNav().addCallback(callback, this);
}


HUDuiServerListCustomTab::~HUDuiServerListCustomTab() {
  getNav().removeCallback(callback, this);
}


size_t HUDuiServerListCustomTab::callback(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* data) {
  if ((oldFocus == 0) && (changeMethod == hnPrev)) {
    ((HUDuiServerListCustomTab*)data)->getNavList()->prev();
    if (((HUDuiServerListCustomTab*)data)->getNavList()->get()->isContainer()) {
      ((HUDuiNestedContainer*)((HUDuiServerListCustomTab*)data)->getNavList()->get())->getNav().set(((HUDuiServerListCustomTab*)data)->getNavList()->get());
    }
    return HUDNavigationQueue::SkipSetFocus;
  }

  if ((oldFocus == ((HUDuiServerListCustomTab*)data)->getNav().size() - 1) && (changeMethod == hnNext)) { proposedFocus = oldFocus; }

  return proposedFocus;
}


void HUDuiServerListCustomTab::setSize(float width, float height) {
  HUDuiControl::setSize(width, height);

  resize();
}


void HUDuiServerListCustomTab::setFontSize(float size) {
  HUDuiControl::setFontSize(size);

  resize();
}


void HUDuiServerListCustomTab::setFontFace(const LocalFontFace* face) {
  HUDuiControl::setFontFace(face);

  resize();
}


void HUDuiServerListCustomTab::setPosition(float x, float y) {
  HUDuiControl::setPosition(x, y);

  resize();
}


void HUDuiServerListCustomTab::resize() {
  FontManager& fm = FontManager::instance();

  float _fontheight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());

  float y = getY() + getHeight() - _fontheight;

  float spacer = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "XXXI");

  float width = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tabName->getLabel());

  tabName->setFontFace(getFontFace());
  tabName->setFontSize(getFontSize());
  tabName->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  const int   faceID = getFontFace()->getFMFace();
  const float fontSize = getFontSize();

  width = fm.getStringWidth(faceID, fontSize, domainName->getLabel());

  domainName->setFontFace(getFontFace());
  domainName->setFontSize(fontSize);
  domainName->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, serverName->getLabel());

  serverName->setFontFace(getFontFace());
  serverName->setFontSize(fontSize);
  serverName->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, emptyServer->getLabel());

  emptyServer->setFontFace(getFontFace());
  emptyServer->setFontSize(fontSize);
  emptyServer->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, fullServer->getLabel());

  fullServer->setFontFace(getFontFace());
  fullServer->setFontSize(fontSize);
  fullServer->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, teamFFAServers->getLabel());

  teamFFAServers->setFontFace(getFontFace());
  teamFFAServers->setFontSize(fontSize);
  teamFFAServers->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, openFFAServers->getLabel());

  openFFAServers->setFontFace(getFontFace());
  openFFAServers->setFontSize(fontSize);
  openFFAServers->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, classicCTFServers->getLabel());

  classicCTFServers->setFontFace(getFontFace());
  classicCTFServers->setFontSize(fontSize);
  classicCTFServers->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, rabbitChaseServers->getLabel());

  rabbitChaseServers->setFontFace(getFontFace());
  rabbitChaseServers->setFontSize(fontSize);
  rabbitChaseServers->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, ricochet->getLabel());

  ricochet->setFontFace(getFontFace());
  ricochet->setFontSize(fontSize);
  ricochet->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, superFlags->getLabel());

  superFlags->setFontFace(getFontFace());
  superFlags->setFontSize(fontSize);
  superFlags->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, antidoteFlag->getLabel());

  antidoteFlag->setFontFace(getFontFace());
  antidoteFlag->setFontSize(fontSize);
  antidoteFlag->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, jumping->getLabel());

  jumping->setFontFace(getFontFace());
  jumping->setFontSize(fontSize);
  jumping->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, handicap->getLabel());

  handicap->setFontFace(getFontFace());
  handicap->setFontSize(fontSize);
  handicap->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, luaWorld->getLabel());

  luaWorld->setFontFace(getFontFace());
  luaWorld->setFontSize(fontSize);
  luaWorld->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  width = fm.getStringWidth(faceID, fontSize, luaRules->getLabel());

  luaRules->setFontFace(getFontFace());
  luaRules->setFontSize(fontSize);
  luaRules->setPosition(getX() + width + spacer, y);

  y = y - _fontheight;

  createNew->setFontFace(getFontFace());
  createNew->setFontSize(fontSize);
  createNew->setPosition(getX() + spacer, y);
}


HUDuiServerList* HUDuiServerListCustomTab::createServerList() {
  getNav().set((size_t) 0);
  HUDuiServerList* newServerList = new HUDuiServerList;
  if (emptyServer->getList().at(emptyServer->getIndex()) == "False") {
    newServerList->toggleFilter(HUDuiServerList::EmptyServer);
  }
  if (fullServer->getList().at(fullServer->getIndex()) == "False") {
    newServerList->toggleFilter(HUDuiServerList::FullServer);
  }
  if (teamFFAServers->getList().at(teamFFAServers->getIndex()) == "False") {
    newServerList->toggleFilter(HUDuiServerList::TeamFFAGameMode);
  }
  if (openFFAServers->getList().at(openFFAServers->getIndex()) == "False") {
    newServerList->toggleFilter(HUDuiServerList::OpenFFAGameMode);
  }
  if (classicCTFServers->getList().at(classicCTFServers->getIndex()) == "False") {
    newServerList->toggleFilter(HUDuiServerList::ClassicCTFGameMode);
  }
  if (rabbitChaseServers->getList().at(rabbitChaseServers->getIndex()) == "False") {
    newServerList->toggleFilter(HUDuiServerList::RabbitChaseGameMode);
  }
  if (jumping->getList().at(jumping->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::JumpingOn);
  }
  if (jumping->getList().at(jumping->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::JumpingOff);
  }
  if (handicap->getList().at(handicap->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::HandicapOn);
  }
  if (handicap->getList().at(handicap->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::HandicapOff);
  }
  if (ricochet->getList().at(ricochet->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::RicochetOn);
  }
  if (ricochet->getList().at(ricochet->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::RicochetOff);
  }
  if (antidoteFlag->getList().at(antidoteFlag->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::AntidoteFlagOn);
  }
  if (antidoteFlag->getList().at(antidoteFlag->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::AntidoteFlagOff);
  }
  if (superFlags->getList().at(superFlags->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::SuperFlagsOn);
  }
  if (superFlags->getList().at(superFlags->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::SuperFlagsOff);
  }
  if (luaWorld->getList().at(luaWorld->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::LuaWorldOn);
  }
  if (luaWorld->getList().at(luaWorld->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::LuaWorldOff);
  }
  if (luaRules->getList().at(luaRules->getIndex()) == "On") {
    newServerList->toggleFilter(HUDuiServerList::LuaRulesOn);
  }
  if (luaRules->getList().at(luaRules->getIndex()) == "Off") {
    newServerList->toggleFilter(HUDuiServerList::LuaRulesOff);
  }

  newServerList->domainNameFilter(domainName->getString());
  newServerList->serverNameFilter(serverName->getString());

  return newServerList;
}


void HUDuiServerListCustomTab::doRender() {
  if (getFontFace() < 0) {
    return;
  }

  float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  glColor4fv(color);

  glOutlineBoxHV(1.0f, getX(), getY(),
                 getX() + getWidth(), getY() + getHeight() + 1, -0.5f);

  tabName->render();
  domainName->render();
  serverName->render();
  emptyServer->render();
  fullServer->render();
  teamFFAServers->render();
  openFFAServers->render();
  classicCTFServers->render();
  rabbitChaseServers->render();
  ricochet->render();
  superFlags->render();
  antidoteFlag->render();
  jumping->render();
  handicap->render();
  luaWorld->render();
  luaRules->render();
  createNew->render();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
