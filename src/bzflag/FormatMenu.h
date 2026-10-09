/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FORMATMENU_H__
#define __FORMATMENU_H__

// Inherits from
#include "MenuDefaultKey.h"
#include "HUDDialog.h"

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDuiLabel.h"
#include "HUDuiDefaultKey.h"


class FormatMenu;

class FormatMenuDefaultKey final : public MenuDefaultKey
{
public:
    FormatMenuDefaultKey(FormatMenu* _menu) :
        menu(_menu) {}
    ~FormatMenuDefaultKey() {}

    bool keyPress(const BzfKeyEvent&) override;
    bool keyRelease(const BzfKeyEvent&) override;

private:
    FormatMenu* menu;
};

class FormatMenu final : public HUDDialog
{
public:
    FormatMenu();
    ~FormatMenu();

    HUDuiDefaultKey*  getDefaultKey() override
    {
        return &defaultKey;
    }
    int           getSelected() const;
    void          setSelected(int);
    void          show() override;
    void          execute() override;
    void          resize(int width, int height) override;

    void          setFormat(bool test);

public:
    static const int  NumItems;

private:
    void          addLabel(const char* msg, const char* _label);

private:
    FormatMenuDefaultKey  defaultKey;
    int           numFormats;

    HUDuiLabel*       currentLabel;
    HUDuiLabel*       pageLabel;
    int           selectedIndex;
    bool*         badFormats;

    static const int  NumColumns;
    static const int  NumReadouts;
};


#endif /* __FORMATMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
