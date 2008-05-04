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

#include "FTInternals.h"

#include "FTFontImpl.h"

#include "FTBitmapFontImpl.h"
#include "FTExtrudeFontImpl.h"
#include "FTOutlineFontImpl.h"
#include "FTPixmapFontImpl.h"
#include "FTPolygonFontImpl.h"
#include "FTTextureFontImpl.h"

#include "FTGlyphContainer.h"
#include "FTFace.h"


//
//  FTFont
//


FTFont::FTFont()
{
    /* impl is set by the child class */
    impl = NULL;
}


FTFont::~FTFont()
{
    /* Only the top class should be allowed to destroy impl, because
     * we do not know how many levels of inheritance there are. */
    delete impl;
}


bool FTFont::Attach(const char* fontFilePath)
{
    if(impl->face.Attach(fontFilePath))
    {
        impl->err = 0;
        return true;
    }
    else
    {
        impl->err = impl->face.Error();
        return false;
    }
}


bool FTFont::Attach(const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
{
    if(impl->face.Attach(pBufferBytes, bufferSizeInBytes))
    {
        impl->err = 0;
        return true;
    }
    else
    {
        impl->err = impl->face.Error();
        return false;
    }
}


bool FTFont::FaceSize(const unsigned int size, const unsigned int res)
{
    return impl->FaceSize(size, res);
}


unsigned int FTFont::FaceSize() const
{
    return impl->FaceSize();
}


void FTFont::Depth(float depth)
{
    impl->Depth(depth);
}


void FTFont::Outset(float outset)
{
    impl->Outset(outset);
}


void FTFont::Outset(float front, float back)
{
    impl->Outset(front, back);
}


bool FTFont::CharMap(FT_Encoding encoding)
{
    bool result = impl->glyphList->CharMap(encoding);
    impl->err = impl->glyphList->Error();
    return result;
}


unsigned int FTFont::CharMapCount()
{
    return impl->face.CharMapCount();
}


FT_Encoding* FTFont::CharMapList()
{
    return impl->face.CharMapList();
}


void FTFont::UseDisplayList(bool useList)
{
    impl->useDisplayLists = useList;
}

float FTFont::Ascender() const
{
    return impl->charSize.Ascender();
}


float FTFont::Descender() const
{
    return impl->charSize.Descender();
}


float FTFont::LineHeight() const
{
    return impl->charSize.Height();
}


void FTFont::Render(const wchar_t* string)
{
    impl->Render(string);
}


void FTFont::Render(const char * string)
{
    impl->Render(string);
}


void FTFont::Render(const char * string, int renderMode)
{
    impl->Render(string, renderMode);
}


void FTFont::Render(const wchar_t* string, int renderMode)
{
    impl->Render(string, renderMode);
}


float FTFont::Advance(const wchar_t* string)
{
    return impl->Advance(string);
}


float FTFont::Advance(const char* string)
{
    return impl->Advance(string);
}


void FTFont::BBox(const char* string, const int start, const int end,
                  float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz)
{
    return impl->BBox(string, start, end, llx, lly, llz, urx, ury, urz);
}


void FTFont::BBox(const wchar_t* string, const int start, const int end,
                  float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz)
{
    return impl->BBox(string, start, end, llx, lly, llz, urx, ury, urz);
}


void FTFont::BBox(const char* string, float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz)
{
    impl->BBox(string, 0, -1, llx, lly, llz, urx, ury, urz);
}


void FTFont::BBox(const wchar_t* string, float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz)
{
    impl->BBox(string, 0, -1, llx, lly, llz, urx, ury, urz);
}


FT_Error FTFont::Error() const
{
    return impl->err;
}


//
//  FTFontImpl
//


FTFontImpl::FTFontImpl(char const *fontFilePath) :
    face(fontFilePath),
    useDisplayLists(true),
    glyphList(0)
{
    err = face.Error();
    if(err == 0)
    {
        glyphList = new FTGlyphContainer(&face);
    }
}


FTFontImpl::FTFontImpl(const unsigned char *pBufferBytes,
                       size_t bufferSizeInBytes) :
    face(pBufferBytes, bufferSizeInBytes),
    useDisplayLists(true),
    glyphList(0)
{
    err = face.Error();
    if(err == 0)
    {
        glyphList = new FTGlyphContainer(&face);
    }
}


FTFontImpl::~FTFontImpl()
{
    if(glyphList)
    {
        delete glyphList;
    }
}


/* FIXME: DoRender should disappear, see commit [853]. */
void FTFontImpl::DoRender(const unsigned int chr, const unsigned int nextChr,
                          FTPoint &origin, int renderMode)
{
    if(CheckGlyph(chr))
    {
        FTPoint kernAdvance = glyphList->Render(chr, nextChr, origin, renderMode);
        origin += kernAdvance;
    }
}


template <typename T>
inline void FTFontImpl::RenderI(const T* string, int renderMode)
{
    const T* c = string;
    pen = FTPoint(0., 0.);

    while(*c)
    {
        DoRender(*c, *(c + 1), pen, renderMode);
        ++c;
    }
}


void FTFontImpl::Render(const wchar_t* string)
{
    RenderI(string, FTGL::RENDER_FRONT | FTGL::RENDER_BACK | FTGL::RENDER_SIDE);
}


void FTFontImpl::Render(const char * string)
{
    RenderI((const unsigned char *)string,
            FTGL::RENDER_FRONT | FTGL::RENDER_BACK | FTGL::RENDER_SIDE);
}


void FTFontImpl::Render(const char * string, int renderMode)
{
    RenderI((const unsigned char *)string, renderMode);
}


void FTFontImpl::Render(const wchar_t* string, int renderMode)
{
    RenderI(string, renderMode);
}


bool FTFontImpl::FaceSize(const unsigned int size, const unsigned int res)
{
    charSize = face.Size(size, res);
    err = face.Error();

    if(err != 0)
    {
        return false;
    }

    if(glyphList != NULL)
    {
        delete glyphList;
    }

    glyphList = new FTGlyphContainer(&face);
    return true;
}


unsigned int FTFontImpl::FaceSize() const
{
    return charSize.CharSize();
}


void FTFontImpl::Depth(float depth)
{
    ;
}


void FTFontImpl::Outset(float outset)
{
    ;
}


void FTFontImpl::Outset(float front, float back)
{
    ;
}


template <typename T>
inline void FTFontImpl::BBoxI(const T* string, const int start, const int end,
                              float& llx, float& lly, float& llz,
                              float& urx, float& ury, float& urz)
{
    FTBBox totalBBox;

    /* Only compute the bounds if string is non-empty. */
    if(string && ('\0' != string[start]))
    {
        float advance = 0;

        if(CheckGlyph(string[start]))
        {
            totalBBox = glyphList->BBox(string[start]);
            advance = glyphList->Advance(string[start], string[start + 1]);
        }

        /* Expand totalBox by each glyph in String (for idx) */
        for(int i = start + 1; (end < 0 && string[i])
                                 || (end >= 0 && i < end); i++)
        {
            if(CheckGlyph(string[i]))
            {
                FTBBox tempBBox = glyphList->BBox(string[i]);
                tempBBox.Move(FTPoint(advance, 0.0f, 0.0f));

                totalBBox += tempBBox;
                advance += glyphList->Advance(string[i], string[i + 1]);
            }
        }
    }

    // TODO: The Z values do not follow the proper ordering.  I'm not sure why.
    llx = totalBBox.Lower().X() < totalBBox.Upper().X() ? totalBBox.Lower().X() : totalBBox.Upper().X();
    lly = totalBBox.Lower().Y() < totalBBox.Upper().Y() ? totalBBox.Lower().Y() : totalBBox.Upper().Y();
    llz = totalBBox.Lower().Z() < totalBBox.Upper().Z() ? totalBBox.Lower().Z() : totalBBox.Upper().Z();
    urx = totalBBox.Lower().X() > totalBBox.Upper().X() ? totalBBox.Lower().X() : totalBBox.Upper().X();
    ury = totalBBox.Lower().Y() > totalBBox.Upper().Y() ? totalBBox.Lower().Y() : totalBBox.Upper().Y();
    urz = totalBBox.Lower().Z() > totalBBox.Upper().Z() ? totalBBox.Lower().Z() : totalBBox.Upper().Z();
}


void FTFontImpl::BBox(const char* string, const int start, const int end,
                      float& llx, float& lly, float& llz,
                      float& urx, float& ury, float& urz)
{
    return BBoxI(string, start, end, llx, lly, llz, urx, ury, urz);
}


void FTFontImpl::BBox(const wchar_t* string, const int start, const int end,
                      float& llx, float& lly, float& llz,
                      float& urx, float& ury, float& urz)
{
    return BBoxI(string, start, end, llx, lly, llz, urx, ury, urz);
}


template <typename T>
inline float FTFontImpl::AdvanceI(const T* string)
{
    const T* c = string;
    float width = 0.0f;

    while(*c)
    {
        if(CheckGlyph(*c))
        {
            width += glyphList->Advance(*c, *(c + 1));
        }
        ++c;
    }

    return width;
}


float FTFontImpl::Advance(const wchar_t* string)
{
    return AdvanceI(string);
}


float FTFontImpl::Advance(const char* string)
{
    return AdvanceI((const unsigned char *)string);
}


bool FTFontImpl::CheckGlyph(const unsigned int characterCode)
{
    if(NULL == glyphList->Glyph(characterCode))
    {
        unsigned int glyphIndex = glyphList->FontIndex(characterCode);
        FTGlyph* tempGlyph = MakeGlyph(glyphIndex);
        if(NULL == tempGlyph)
        {
            if(0 == err)
            {
                err = 0x13;
            }

            return false;
        }
        glyphList->Add(tempGlyph, characterCode);
    }

    return true;
}

