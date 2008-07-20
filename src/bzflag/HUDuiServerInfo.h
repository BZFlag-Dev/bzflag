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
 * HUDuiServerInfo:
 *	User interface class for the heads-up display's label (text display) control
 */

#ifndef	__HUDUISERVERINFO_H__
#define	__HUDUISERVERINFO_H__

// ancestor class
#include "HUDuiControl.h"

#include "ServerItem.h"

#include <string>

#include "BzfEvent.h"

class HUDuiServerInfo : public HUDuiControl {
  public:
			HUDuiServerInfo();
			~HUDuiServerInfo();

    void setServerItem(ServerItem* item);

  protected:
    void doRender();

  private:
    ServerItem* serverPointer;
};

#endif // __HUDUISERVERINFO_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
