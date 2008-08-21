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
 * HUDuiServerListCustomTab:
 *	User interface class for displaying detailed information from
 *	a ServerItem.
 */

#ifndef	__HUDUISERVERLISTCUSTOMTAB_H__
#define	__HUDUISERVERLISTCUSTOMTAB_H__

// ancestor class
#include "HUDuiNestedContainer.h"

#include "ServerItem.h"
#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"
#include "HUDuiList.h"
#include "HUDuiServerList.h"
#include <string>

class HUDuiServerListCustomTab : public HUDuiNestedContainer {
  public:
      HUDuiServerListCustomTab();
      ~HUDuiServerListCustomTab();

    void setSize(float width, float height);
    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);
    void setPosition(float x, float y);

    HUDuiServerList* createServerList();

    static size_t callback(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* data);

    HUDuiLabel* createNew;
    HUDuiTypeIn* tabName;

  protected:
    void doRender();

    void resize();

  private:
    HUDuiTypeIn* domainName;
    HUDuiTypeIn* serverName;

    HUDuiList* emptyServer;
    HUDuiList* fullServer;
    HUDuiList* teamFFAServers;
    HUDuiList* openFFAServers;
    HUDuiList* classicCTFServers;
    HUDuiList* rabbitChaseServers;
    HUDuiList* ricochet;
    HUDuiList* superFlags;
    HUDuiList* antidoteFlag;
    HUDuiList* jumping;
    HUDuiList* handicap;

    std::list<HUDuiControl*> controls;
};

#endif // __HUDuiServerListCustomTab_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
