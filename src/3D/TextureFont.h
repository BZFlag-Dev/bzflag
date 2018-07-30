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

#ifndef _TEXTURE_FONT_H_
#define _TEXTURE_FONT_H_

/* common interface headers */
#include "VBO_Handler.h"
#include "bzfgl.h"
#include "ImageFont.h"
#include "OpenGLGState.h"

class TextureFont : public ImageFont, VBOclient
{
public:
    TextureFont();
    virtual ~TextureFont();

    virtual void initVBO();
    virtual bool isBuilt() const
    {
        return textureID != -1;
    }

    virtual void filter(bool dofilter);
    virtual void drawString(float scale, GLfloat color[4], const char *str, int len);

    virtual void free();

private:
    void reloadVBO();
    void preLoadLists();

    int         vboID;

    int         textureID;
    OpenGLGState gstate;
};

#endif //_TEXTURE_FONT_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
