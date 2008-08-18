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

/*
 * HUDuiTabbedControl:
 *	Fill this in.
 */

#ifndef	__HUDUITABBEDCONTROL_H__
#define	__HUDUITABBEDCONTROL_H__

// ancestor class
#include "HUDuiNestedContainer.h"

#include "HUDNavigationQueue.h"
#include <vector>

class HUDuiTabbedControl : public HUDuiNestedContainer {
  public:
      HUDuiTabbedControl();
      ~HUDuiTabbedControl();

    HUDuiControl* getActiveTab();
    std::string getActiveTabName();

    void setActiveTab(int tab);
	
    void addTab(HUDuiControl* tabControl, std::string tabName);
    void addTab(HUDuiControl* tabControl, std::string tabName, int index);
    void removeTab(int tabIndex);

    int getTabCount();
    HUDuiControl* getTab(int index);

    void setSize(float width, float height);
    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);
    void setPosition(float x, float y);

  protected:
    std::vector<std::pair<std::string, HUDuiControl*>> tabs;
    void doRender();

    void drawTabs();
    void drawTabBody();

    void addControl(HUDuiControl *control);

    bool doKeyPress(const BzfKeyEvent& key);

  private:
    int activeTab;

    HUDNavigationQueue::iterator tabNavQueuePosition;

    HUDuiControl* activeControl;

    float tabsHeight;
};

#endif // __HUDUITABBEDCONTROL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
