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

// BZFlag common header
#include "common.h"

// Interface headers
#include "ImageFont.h"
#include "TextureFont.h"

// System headers
#include <string>
#include <string.h>

// Common implementation headers
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"
#include "VBO_Handler.h"

// Local implementation headers
#include "TextureManager.h"

TextureFont::TextureFont() : vboID(-1), textureID(-1)
{
    vboManager.registerClient(this);
}

TextureFont::~TextureFont()
{
    vboVT.vboFree(vboID);
    vboManager.unregisterClient(this);
}

void TextureFont::initVBO()
{
    vboID = -1;
}

void TextureFont::reloadVBO()
{
    glm::vec2 textur[1024];
    glm::vec3 vertex[1024];

    vboID = vboVT.vboAlloc(4 * numberOfCharacters);

    glm::vec2 *texPtr = textur;
    glm::vec3 *verPtr = vertex;

    FontMetrics *fontSym = fontMetrics;
    for (int i = numberOfCharacters; i > 0; i--, fontSym++)
    {
        float deltaX = (float)fontSym->initialDist;
        float fFontY = (float)(fontSym->endY - fontSym->startY);
        float fFontX = (float)(fontSym->endX - fontSym->startX);
        float startX = (float)fontSym->startX / (float)textureXSize;
        float endX   = (float)fontSym->endX   / (float)textureXSize;
        float startY = (float)(textureYSize - fontSym->startY) / (float)textureYSize;
        float endY   = (float)(textureYSize - fontSym->endY)   / (float)textureYSize;

        *texPtr++ = glm::vec2(startX, startY);
        *texPtr++ = glm::vec2(startX, endY);
        *texPtr++ = glm::vec2(endX, startY);
        *texPtr++ = glm::vec2(endX, endY);

        *verPtr++ = glm::vec3(deltaX,          fFontY, 0.0f);
        *verPtr++ = glm::vec3(deltaX,          0.0f,   0.0f);
        *verPtr++ = glm::vec3(deltaX + fFontX, fFontY, 0.0f);
        *verPtr++ = glm::vec3(deltaX + fFontX, 0.0f,   0.0f);
    }
    vboVT.vertexData(vboID,  4 * numberOfCharacters, vertex);
    vboVT.textureData(vboID, 4 * numberOfCharacters, textur);
}

void TextureFont::preLoadLists()
{
    if (texture.size() < 1)
    {
        logDebugMessage(2,"Font %s does not have an associated texture name, not loading\n", texture.c_str());
        return;
    }

    // load up the texture
    TextureManager &tm = TextureManager::instance();
    std::string textureAndDir = "fonts/" + texture;
    textureID = tm.getTextureID(textureAndDir.c_str());

    if (textureID == -1)
    {
        logDebugMessage(2,"Font texture %s has invalid ID\n", texture.c_str());
        return;
    }
    logDebugMessage(4,"Font %s (face %s) has texture ID %d\n", texture.c_str(), faceName.c_str(), textureID);

    // fonts are usually pixel aligned
    tm.setTextureFilter(textureID, OpenGLTexture::Nearest);

    // create GState
    OpenGLGStateBuilder builder(gstate);
    builder.setTexture(textureID);
    builder.setBlending();
    builder.setAlphaFunc();
    gstate = builder.getState();
}


void TextureFont::free(void)
{
    textureID = -1;
}

void TextureFont::filter(bool dofilter)
{
    TextureManager &tm = TextureManager::instance();
    if (textureID >= 0)
    {
        const OpenGLTexture::Filter type = dofilter ? OpenGLTexture::Max
                                           : OpenGLTexture::Nearest;
        tm.setTextureFilter(textureID, type);
    }
}

void TextureFont::drawString(float scale, GLfloat color[4], const char *str,
                             int len)
{
    if (!str)
        return;

    if (textureID == -1)
        preLoadLists();

    if (textureID == -1)
        return;

    if (vboID < 0)
        reloadVBO();

    gstate.setState();

    TextureManager &tm = TextureManager::instance();
    if (!tm.bind(textureID))
        return;

    if (color[0] >= 0)
        glColor4f(color[0], color[1], color[2], color[3]);

    glPushMatrix();
    glScalef(scale, scale, 1);

    vboVT.enableArrays();
    int charToUse = 0;
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < len; i++)
    {
        charToUse = str[i] - ' ';
        if (charToUse < 0)
            charToUse = 0;
        else if (charToUse > numberOfCharacters)
            charToUse = 0;

        if (charToUse)
            glDrawArrays(GL_TRIANGLE_STRIP, vboID + 4 * charToUse, 4);
        glTranslatef((float)(fontMetrics[charToUse].fullWidth), 0.0f, 0.0f);
    }
    if (color[0] >= 0)
        glColor4f(1, 1, 1, 1);
    glPopMatrix();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
