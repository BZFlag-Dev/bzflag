/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "FTGL/ftgl.h"

#include "FTGlyphContainer.h"
#include "FTFace.h"
#include "FTCharmap.h"


FTGlyphContainer::FTGlyphContainer(FTFace* f)
:   face(f),
    err(0)
{
    glyphs.push_back(NULL);
    charMap = new FTCharmap(face);
}


FTGlyphContainer::~FTGlyphContainer()
{
    GlyphVector::iterator it;
    for(it = glyphs.begin(); it != glyphs.end(); ++it)
    {
        delete *it;
    }

    glyphs.clear();
    delete charMap;
}


bool FTGlyphContainer::CharMap(FT_Encoding encoding)
{
    bool result = charMap->CharMap(encoding);
    err = charMap->Error();
    return result;
}


unsigned int FTGlyphContainer::FontIndex(const unsigned int characterCode) const
{
    return charMap->FontIndex(characterCode);
}


void FTGlyphContainer::Add(FTGlyph* tempGlyph, const unsigned int characterCode)
{
    charMap->InsertIndex(characterCode, glyphs.size());
    glyphs.push_back(tempGlyph);
}


const FTGlyph* const FTGlyphContainer::Glyph(const unsigned int characterCode) const
{
    signed int index = charMap->GlyphListIndex(characterCode);
    return glyphs[index];
}


FTBBox FTGlyphContainer::BBox(const unsigned int characterCode) const
{
    return glyphs[charMap->GlyphListIndex(characterCode)]->BBox();
}


float FTGlyphContainer::Advance(const unsigned int characterCode, const unsigned int nextCharacterCode)
{
    unsigned int left = charMap->FontIndex(characterCode);
    unsigned int right = charMap->FontIndex(nextCharacterCode);

    float width = face->KernAdvance(left, right).Xf();
    width += glyphs[charMap->GlyphListIndex(characterCode)]->Advance().Xf();

    return width;
}


FTPoint FTGlyphContainer::Render(const unsigned int characterCode, const unsigned int nextCharacterCode, FTPoint penPosition, int renderMode)
{
    FTPoint kernAdvance, advance;

    unsigned int left = charMap->FontIndex(characterCode);
    unsigned int right = charMap->FontIndex(nextCharacterCode);

    kernAdvance = face->KernAdvance(left, right);

    if(!face->Error())
    {
        advance = glyphs[charMap->GlyphListIndex(characterCode)]->Render(penPosition, renderMode);
    }

    kernAdvance += advance;
    return kernAdvance;
}
