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

#include "../FTFont/FTFontImpl.h"
#include "./FTLayoutImpl.h"


//
//  FTLayout
//


FTLayout::FTLayout()
{
    /* impl is set by the child class */
    impl = NULL;
}


FTLayout::~FTLayout()
{
    /* Only the top class should be allowed to destroy impl, because
     * we do not know how many levels of inheritance there are. */
    delete impl;
}


void FTLayout::BBox(const char* string, float& llx, float& lly,
                    float& llz, float& urx, float& ury, float& urz)
{
    impl->BBox(string, llx, lly, llz, urx, ury, urz);
}


void FTLayout::BBox(const wchar_t* string, float& llx, float& lly,
                    float& llz, float& urx, float& ury, float& urz)
{
    impl->BBox(string, llx, lly, llz, urx, ury, urz);
}


void FTLayout::Render(const char *string)
{
    impl->Render(string);
}


void FTLayout::Render(const char *string, int renderMode)
{
    impl->Render(string, renderMode);
}


void FTLayout::Render(const wchar_t *string)
{
    impl->Render(string);
}


void FTLayout::Render(const wchar_t *string, int renderMode)
{
    impl->Render(string, renderMode);
}


FT_Error FTLayout::Error() const
{
    return impl->err;
}


//
//  FTLayoutImpl
//


FTLayoutImpl::FTLayoutImpl() :
    err(0)
{
    ;
}


FTLayoutImpl::~FTLayoutImpl()
{
    ;
}


void FTLayoutImpl::DoRender(FTFont *font, const unsigned int chr,
                            const unsigned int nextChr, int renderMode)
{
    font->impl->DoRender(chr, nextChr, pen, renderMode);
}


void FTLayoutImpl::CheckGlyph(FTFont *font, const unsigned int Chr)
{
    font->impl->CheckGlyph(Chr);
}


FTGlyphContainer * FTLayoutImpl::GetGlyphs(FTFont *font)
{
    return font->impl->glyphList;
}


FTSize & FTLayoutImpl::GetCharSize(FTFont *font)
{
    return font->impl->charSize;
}

