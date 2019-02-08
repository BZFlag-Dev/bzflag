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

// interface headers
#include "HUDuiTextureLabel.h"

// system headers
#include <iostream>

// common implementation headers
#include "TextureManager.h"
#include "OpenGLTexture.h"
#include "VBO_Handler.h"
#include "VBO_Drawing.h"

//
// HUDuiTextureLabel
//

HUDuiTextureLabel::HUDuiTextureLabel() : HUDuiLabel(), texture()
{
}

HUDuiTextureLabel::~HUDuiTextureLabel()
{
}

void            HUDuiTextureLabel::setTexture(const int t)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setTexture(t);
    builder.setBlending();
    gstate = builder.getState();
    texture = t;
}

void            HUDuiTextureLabel::doRender()
{
    if (getFontFace() < 0) return;

    // render string if texture filter is Off, otherwise draw the texture
    // about the same size and position as the string would be.
    if (!gstate.isTextured() || texture < 0)
        HUDuiLabel::doRender();
    else   // why use a font? it's an image, use the image size, let every pixel be seen!!! :)
    {
        const float _height = getFontSize(); //texture.getHeight();//
        TextureManager &tm = TextureManager::instance();

        const float _width = _height * (1.0f / tm.GetAspectRatio(texture));
        //const float descent = font.getDescent();
        const float descent = 0;
        const float xx = getX();
        const float yy = getY();
        gstate.setState();
        glColor4f(textColor[0], textColor[1], textColor[2], 1.0f);

        glPushMatrix();
        glTranslatef(xx, yy - descent, 0.0f);
        glScalef(_width, _height, 0.0f);
        DRAWER.asimmetricTexturedRect();
        glPopMatrix();
    }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
