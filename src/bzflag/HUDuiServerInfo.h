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

/*
 * HUDuiServerInfo:
 *	User interface class for displaying detailed information from
 *	a ServerItem.
 */

#ifndef	__HUDUISERVERINFO_H__
#define	__HUDUISERVERINFO_H__

// ancestor class
#include "HUDuiControl.h"

#include "ServerItem.h"
#include "HUDuiLabel.h"
#include <string>

class HUDuiServerInfo : public HUDuiControl {
  public:
      HUDuiServerInfo();
      ~HUDuiServerInfo();

    void setServerItem(ServerItem* item);

    void setSize(float width, float height);
    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);
    void setPosition(float x, float y);

  protected:
    void doRender();

    void resize();
    void fillReadouts();

  private:
    std::string serverKey;

    std::vector<HUDuiLabel*> readouts;
    std::vector<HUDuiLabel*> playerLabels;
};

#endif // __HUDUISERVERINFO_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
