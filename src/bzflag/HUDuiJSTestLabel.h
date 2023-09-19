/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
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
 * HUDuiJSTestLabel:
 *  User interface classes and functions for the joystick range test
 */

#ifndef __HUDUIJSTESTLABEL_H__
#define __HUDUIJSTESTLABEL_H__

#include "HUDuiLabel.h"
#include "OpenGLGState.h"

class HUDuiJSTestLabel : public HUDuiLabel
{
public:
    HUDuiJSTestLabel();
    ~HUDuiJSTestLabel() { };

    void setSize(float newWidth, float newHeight);

protected:
    void        doRender();

private:
    float width, height;
    OpenGLGState    gstate;
};

#endif // __HUDUIJSTESTLABEL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
