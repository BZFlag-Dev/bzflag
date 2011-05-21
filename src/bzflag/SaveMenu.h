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

#ifndef __SAVEMENU_H__
#define __SAVEMENU_H__

/* system interface headers */

/* common interface headers */
#include "BzfEvent.h"
#include "CommandsStandard.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"


class SaveMenuDefaultKey : public MenuDefaultKey {
  public:
    SaveMenuDefaultKey() { }
    ~SaveMenuDefaultKey() { }

    bool keyPress(const BzfKeyEvent&);
    bool keyRelease(const BzfKeyEvent&);

};


class SaveMenu : public HUDDialog {
  public:
    SaveMenu();
    ~SaveMenu();

    HUDuiDefaultKey* getDefaultKey() { return &defaultKey; }
    void execute();
    void resize(int width, int height);
    void setFileName(std::string& fname);
    std::string getFileName();

  private:
    SaveMenuDefaultKey defaultKey;
    std::string          filename;
};


#endif /* __SAVEMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
