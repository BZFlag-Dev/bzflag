/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "HUDuiControl.h"

// system headers
#include <iostream>

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "TextureManager.h"
#include "FontManager.h"
#include "VBO_Handler.h"

// local implementation headers
#include "HUDui.h"


//
// HUDuiControl
//

// init static members
const GLfloat       HUDuiControl::dimTextColor[3] = { 0.7f, 0.7f, 0.7f };
const GLfloat       HUDuiControl::moreDimTextColor[3] = { 0.4f, 0.4f, 0.4f };
const GLfloat       HUDuiControl::textColor[3] = { 1.0f, 1.0f, 1.0f };
OpenGLGState*       HUDuiControl::gstate = NULL;
int     HUDuiControl::arrow = -1;
int         HUDuiControl::arrowFrame = 0;
TimeKeeper      HUDuiControl::lastTime;
int         HUDuiControl::totalCount = 0;

HUDuiControl::HUDuiControl() : showingFocus(true),
    fontFace(-1), fontSize(11),
    x(0.0f), y(0.0f),
    width(1.0f), height(1.0f),
    fontHeight(11.0f),
    desiredLabelWidth(0.0f),
    trueLabelWidth(0.0f),
    prev(this), next(this),
    cb(NULL), userData(NULL)
{
    if (totalCount == 0)
    {
        // load arrow texture
        TextureManager &tm = TextureManager::instance();
        arrow = tm.getTextureID( "menu_arrow" );

        // make gstate for focus arrow
        gstate = new OpenGLGState;
        OpenGLGStateBuilder builder(*gstate);
        builder.setTexture(arrow);
        builder.setBlending();
//    builder.setSmoothing();
        *gstate = builder.getState();

        // get start time for animation
        lastTime = TimeKeeper::getCurrent();
    }

    totalCount++;
}

HUDuiControl::~HUDuiControl()
{
    if (--totalCount == 0)
    {
        delete gstate;
        arrow = -1;
        gstate = NULL;
    }
}

float           HUDuiControl::getLabelWidth() const
{
    return desiredLabelWidth;
}

std::string     HUDuiControl::getLabel() const
{
    return BundleMgr::getCurrentBundle()->getLocalString(label);
}

int         HUDuiControl::getFontFace() const
{
    return fontFace;
}

float           HUDuiControl::getFontSize() const
{
    return fontSize;
}

HUDuiControl*       HUDuiControl::getPrev() const
{
    return prev;
}

HUDuiControl*       HUDuiControl::getNext() const
{
    return next;
}

HUDuiCallback       HUDuiControl::getCallback() const
{
    return cb;
}

const void*     HUDuiControl::getUserData() const
{
    return userData;
}

void            HUDuiControl::setPosition(float _x, float _y)
{
    x = _x;
    y = _y;
}

void            HUDuiControl::setSize(float _width, float _height)
{
    width = _width;
    height = _height;
}

void            HUDuiControl::setLabelWidth(float labelWidth)
{
    desiredLabelWidth = labelWidth;
}

void            HUDuiControl::setLabel(const std::string& _label)
{

    label = _label;
    if (fontFace >= 0)
    {
        FontManager &fm = FontManager::instance();
        trueLabelWidth = fm.getStrLength(fontFace, fontSize, getLabel() + "99");
    }
}

void            HUDuiControl::setFontFace(int _fontFace)
{
    fontFace = _fontFace;
    onSetFont();
}

void            HUDuiControl::setFontSize(float size)
{
    fontSize = size;
    onSetFont();
}

void            HUDuiControl::setPrev(HUDuiControl* _prev)
{
    if (!_prev) prev = this;
    else prev = _prev;
}

void            HUDuiControl::setNext(HUDuiControl* _next)
{
    if (!_next) next = this;
    else next = _next;
}

void            HUDuiControl::setCallback(HUDuiCallback _cb, const void* _ud)
{
    cb = _cb;
    userData = _ud;
}

void            HUDuiControl::onSetFont()
{
    if (fontFace >= 0)
    {
        FontManager &fm = FontManager::instance();
        fontHeight = fm.getStrHeight(fontFace, fontSize, getLabel());
        trueLabelWidth = fm.getStrLength(fontFace, fontSize, getLabel() + "99");
    }
    else
    {
        fontHeight = 11.0f;
        trueLabelWidth = 0.0f;
    }
}

bool            HUDuiControl::hasFocus() const
{
    return this == HUDui::getFocus();
}

void            HUDuiControl::setFocus()
{
    HUDui::setFocus(this);
}

void            HUDuiControl::showFocus(bool _showingFocus)
{
    showingFocus = _showingFocus;
}

void            HUDuiControl::doCallback()
{
    if (cb) (*cb)(this, userData);
}

void            HUDuiControl::renderFocus()
{
    float fh2;

    TextureManager &tm = TextureManager::instance();
    const ImageInfo &info = tm.getInfo(arrow);

    if (gstate->isTextured())   // assumes there are w/h frames of animation h x h in each image
    {
        float imageSize = (float)info.y;
        int uFrames = 1;
        if (imageSize != 0)
            uFrames = int(info.x/imageSize); // 4;
        int vFrames = 1; // 4;
        float du = 1.0f / (float)uFrames;
        float dv = 1.0f / (float)vFrames;

        float u = (float)(arrowFrame % uFrames) / (float)uFrames;
        float v = (float)(arrowFrame / uFrames) / (float)vFrames;
        fh2 = floorf(1.5f * fontHeight) - 1.0f; // this really should not scale the image based on the font,
        gstate->setState();             // best would be to load an image for each size
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        float imageXShift = 0.5f;
        float imageYShift = -fh2 * 0.2f;
        float outputSize = fh2;
        glm::vec2 textures[4];
        glm::vec3 vertices[4];

        textures[0] = glm::vec2(u, v);
        textures[1] = glm::vec2(u + du, v);
        textures[2] = glm::vec2(u, v + dv);
        textures[3] = glm::vec2(u + du, v + dv);
        vertices[0] = glm::vec3(x + imageXShift - outputSize, y + imageYShift, 0);
        vertices[1] = glm::vec3(x + imageXShift, y + imageYShift, 0);
        vertices[2] = glm::vec3(x + imageXShift - outputSize, y + outputSize + imageYShift, 0);
        vertices[3] = glm::vec3(x + imageXShift, y + outputSize + imageYShift, 0);

        int vboIndex = vboVT.vboAlloc(4);
        vboVT.textureData(vboIndex, 4, textures);
        vboVT.vertexData(vboIndex, 4, vertices);
        vboVT.enableArrays();
        glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
        vboVT.vboFree(vboIndex);

        TimeKeeper nowTime = TimeKeeper::getCurrent();
        if (nowTime - lastTime > 0.07f)
        {
            lastTime = nowTime;
            if (++arrowFrame == uFrames * vFrames) arrowFrame = 0;
        }
    }
    else
    {
        fh2 = floorf(0.5f * fontHeight);
        gstate->setState();
        glm::vec3 vertices[6];
        vertices[0] = glm::vec3(x - fh2 - fontHeight, y + fontHeight - 1.0f, 0);
        vertices[1] = glm::vec3(x - fh2 - fontHeight, y, 0);
        vertices[2] = glm::vec3(x - fh2 - 1.0f, y + 0.5f * (fontHeight - 1.0f), 0);
        vertices[3] = glm::vec3(x - fh2 - fontHeight, y + fontHeight - 1.0f, 0);
        vertices[4] = glm::vec3(x - fh2 - fontHeight, y, 0);
        vertices[5] = glm::vec3(x - fh2 - 1.0f, y + 0.5f * (fontHeight - 1.0f), 0);
        int vboIndex = vboV.vboAlloc(6);
        vboV.vertexData(vboIndex, 6, vertices);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, vboIndex, 3);

        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINE_LOOP, vboIndex + 3, 3);
        vboVT.vboFree(vboIndex);
    }
}

void            HUDuiControl::renderLabel()
{
    std::string theLabel = getLabel();
    if (theLabel.length() > 0 && fontFace >= 0)
    {
        FontManager &fm = FontManager::instance();
        const float dx = (desiredLabelWidth > trueLabelWidth)
                         ? desiredLabelWidth : trueLabelWidth;
        fm.drawString(x - dx, y, 0, fontFace, fontSize, theLabel);
    }
}

void            HUDuiControl::render()
{
    if (hasFocus() && showingFocus) renderFocus();
    const GLfloat *col = hasFocus() ? textColor : dimTextColor;
    glColor4f(col[0], col[1], col[2], 1.0f);
    renderLabel();
    doRender();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
