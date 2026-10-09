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

#ifndef __QUITMENU_H__
#define __QUITMENU_H__

// Inherits from
#include "MenuDefaultKey.h"
#include "HUDDialog.h"

/* common interface headers */
#include "BzfEvent.h"
#include "CommandsStandard.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"


class QuitMenuDefaultKey final : public MenuDefaultKey
{
public:
    QuitMenuDefaultKey() {}
    ~QuitMenuDefaultKey() {}

    bool keyPress(const BzfKeyEvent&) override;
    bool keyRelease(const BzfKeyEvent&) override;

};


class QuitMenu final : public HUDDialog
{
public:
    QuitMenu();
    ~QuitMenu();

    HUDuiDefaultKey* getDefaultKey() override
    {
        return &defaultKey;
    }
    void execute() override
    {
        CommandsStandard::quit();
    }
    void resize(int width, int height) override;

private:
    QuitMenuDefaultKey defaultKey;

};


#endif /* __QUITMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
