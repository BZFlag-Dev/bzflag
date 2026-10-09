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

#ifndef __SAVEWORLDMENU_H__
#define __SAVEWORLDMENU_H__

// Inherits from
#include "HUDDialog.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "HUDuiTypeIn.h"
#include "HUDuiLabel.h"


class SaveWorldMenu final : public HUDDialog
{
public:
    SaveWorldMenu();
    ~SaveWorldMenu();

    HUDuiDefaultKey* getDefaultKey() override;

    void execute() override;
    void resize(int width, int height) override;

private:
    HUDuiTypeIn* filename;
    HUDuiLabel* status;
};


#endif /* __SAVEWORLDMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
