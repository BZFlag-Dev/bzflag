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

// interface headers
#include "HUDuiJSTestLabel.h"

// system headers
#include <iostream>

// common implementation headers
#include "TextureManager.h"
#include "OpenGLTexture.h"
#include "playing.h"

//
// HUDuiJSTestLabel
//

HUDuiJSTestLabel::HUDuiJSTestLabel() : HUDuiLabel(), width(0), height(0)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setBlending();
    builder.enableTexture(false);
    gstate = builder.getState();
}

void HUDuiJSTestLabel::setSize(float newWidth, float newHeight)
{
    width = newWidth;
    height = newHeight;
}

void            HUDuiJSTestLabel::doRender()
{
    gstate.setState();

    // scale elements from a relative screen height of 800 pixels
#define BZ_SCALE_JS_TEST_ELEMS(x) std::ceil(float(x) * height / 800.0f)

    // appearance constants
    const float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 0.75f };

    const auto realCursorThickness = BZ_SCALE_JS_TEST_ELEMS(2);
    const auto realCursorLength = BZ_SCALE_JS_TEST_ELEMS(40.0f);
    const float realCursorColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    const auto modifiedCursorLength = BZ_SCALE_JS_TEST_ELEMS(20.0f);
    const float modifiedCursorColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    // draw the background
    const auto rangeLimit = float(BZDB.evalInt("jsRangeMax")) / 100.0f;
    glColor4fv(backgroundColor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(getX() + (1.0f - rangeLimit) / 2.0f * width, getY() + (1.0f + rangeLimit) / 2.0f * height);
    glVertex2f(getX() + (1.0f - rangeLimit) / 2.0f * width, getY() + (1.0f - rangeLimit) / 2.0f * height);
    glVertex2f(getX() + (1.0f + rangeLimit) / 2.0f * width, getY() + (1.0f - rangeLimit) / 2.0f * height);
    glVertex2f(getX() + (1.0f + rangeLimit) / 2.0f * width, getY() + (1.0f + rangeLimit) / 2.0f * height);
    glEnd();

    // draw the real cursor
    float jsx, jsy;
    mainWindow->getJoyPosition(jsx, jsy);
    jsx *= BZDB.evalInt("jsInvertAxes") % 2 == 1 ? -1.0f : 1.0f; // invert axes as required
    jsy *= BZDB.evalInt("jsInvertAxes") > 1 ? -1.0f : 1.0f;
    auto jsxTransformed = ((1.0f + jsx) / 2.0f) * width;
    auto jsyTransformed = ((1.0f - jsy) / 2.0f) * height;
    glPushAttrib(GL_LINE_BIT);
    glLineWidth(realCursorThickness);
    glColor4fv(realCursorColor);
    glBegin(GL_LINES);
    glVertex2f(getX() + jsxTransformed - realCursorLength / 2.0f, getY() + jsyTransformed);
    glVertex2f(getX() + jsxTransformed + realCursorLength / 2.0f, getY() + jsyTransformed);
    glVertex2f(getX() + jsxTransformed, getY() + jsyTransformed - realCursorLength / 2.0f);
    glVertex2f(getX() + jsxTransformed, getY() + jsyTransformed + realCursorLength / 2.0f);
    glEnd();
    glPopAttrib();

    // draw the modified cursor
    mainWindow->getJoyPosition(jsx, jsy);
    applyJSModifiers(jsx, jsy);
    jsxTransformed = ((1.0f + jsx) / 2.0f) * width;
    jsyTransformed = ((1.0f - jsy) / 2.0f) * height;
    glColor4fv(modifiedCursorColor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(getX() + jsxTransformed, getY() + jsyTransformed - modifiedCursorLength / 2.0f);
    glVertex2f(getX() + jsxTransformed + modifiedCursorLength / 2.0f, getY() + jsyTransformed);
    glVertex2f(getX() + jsxTransformed, getY() + jsyTransformed + modifiedCursorLength / 2.0f);
    glVertex2f(getX() + jsxTransformed - modifiedCursorLength / 2.0f, getY() + jsyTransformed);
    glEnd();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
