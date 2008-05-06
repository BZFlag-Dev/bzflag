/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 *               2008 Éric Beets <ericbeets@free.fr>
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

FTGL_BEGIN_C_DECLS

#define C_TOR(cname, cargs, cxxname, cxxarg, cxxtype) \
    FTGLfont* cname cargs \
    { \
        cxxname *f = new cxxname cxxarg; \
        if(f->Error()) \
        { \
            delete f; \
            return NULL; \
        } \
        FTGLfont *ftgl = (FTGLfont *)malloc(sizeof(FTGLfont)); \
        ftgl->ptr = f; \
        ftgl->type = cxxtype; \
        return ftgl; \
    }

// FTBitmapFont::FTBitmapFont();
C_TOR(ftglCreateBitmapFont, (const char *fontname),
      FTBitmapFont, (fontname), FONT_BITMAP);

// FTExtrudeFont::FTExtrudeFont();
C_TOR(ftglCreateExtrudeFont, (const char *fontname),
      FTExtrudeFont, (fontname), FONT_EXTRUDE);

// FTOutlineFont::FTOutlineFont();
C_TOR(ftglCreateOutlineFont, (const char *fontname),
      FTOutlineFont, (fontname), FONT_OUTLINE);

// FTPixmapFont::FTPixmapFont();
C_TOR(ftglCreatePixmapFont, (const char *fontname),
      FTPixmapFont, (fontname), FONT_PIXMAP);

// FTPolygonFont::FTPolygonFont();
C_TOR(ftglCreatePolygonFont, (const char *fontname),
      FTPolygonFont, (fontname), FONT_POLYGON);

// FTTextureFont::FTTextureFont();
C_TOR(ftglCreateTextureFont, (const char *fontname),
      FTTextureFont, (fontname), FONT_TEXTURE);

#define C_FUN(cret, cname, cargs, cxxerr, cxxname, cxxarg) \
    cret cname cargs \
    { \
        if(!f || !f->ptr) \
        { \
            fprintf(stderr, "FTGL warning: NULL pointer in %s\n", #cname); \
            cxxerr; \
        } \
        switch(f->type) \
        { \
            case FTGL::FONT_BITMAP: \
                return dynamic_cast<FTBitmapFont*>(f->ptr)->cxxname cxxarg; \
            case FTGL::FONT_EXTRUDE: \
                return dynamic_cast<FTExtrudeFont*>(f->ptr)->cxxname cxxarg; \
            case FTGL::FONT_OUTLINE: \
                return dynamic_cast<FTOutlineFont*>(f->ptr)->cxxname cxxarg; \
            case FTGL::FONT_PIXMAP: \
                return dynamic_cast<FTPixmapFont*>(f->ptr)->cxxname cxxarg; \
            case FTGL::FONT_POLYGON: \
                return dynamic_cast<FTPolygonFont*>(f->ptr)->cxxname cxxarg; \
            case FTGL::FONT_TEXTURE: \
                return dynamic_cast<FTTextureFont*>(f->ptr)->cxxname cxxarg; \
        } \
        fprintf(stderr, "FTGL warning: %s not implemented for %d\n", #cname, f->type); \
        cxxerr; \
    }

// FTFont::~FTFont();
void ftglDestroyFont(FTGLfont *f)
{
    if(!f || !f->ptr)
    {
        fprintf(stderr, "FTGL warning: NULL pointer in %s\n", __FUNCTION__);
        return;
    }
    switch(f->type)
    {
        case FTGL::FONT_BITMAP:
            delete dynamic_cast<FTBitmapFont*>(f->ptr); break;
        case FTGL::FONT_EXTRUDE:
            delete dynamic_cast<FTExtrudeFont*>(f->ptr); break;
        case FTGL::FONT_OUTLINE:
            delete dynamic_cast<FTOutlineFont*>(f->ptr); break;
        case FTGL::FONT_PIXMAP:
            delete dynamic_cast<FTPixmapFont*>(f->ptr); break;
        case FTGL::FONT_POLYGON:
            delete dynamic_cast<FTPolygonFont*>(f->ptr); break;
        case FTGL::FONT_TEXTURE:
            delete dynamic_cast<FTTextureFont*>(f->ptr); break;
        default:
            fprintf(stderr, "FTGL warning: %s not implemented for %d\n",
                            __FUNCTION__, f->type);
            break;
    }

    f->ptr = NULL;
    free(f);
}

// bool FTFont::Attach(const char* fontFilePath);
C_FUN(int, ftglAttachFile, (FTGLfont *f, const char* path),
      return 0, Attach, (path));

// bool FTFont::Attach(const unsigned char *pBufferBytes,
//                     size_t bufferSizeInBytes);
C_FUN(int, ftglAttachData, (FTGLfont *f, const unsigned char *p, size_t s),
      return 0, Attach, (p, s));

// bool FTFont::CharMap(FT_Encoding encoding);
C_FUN(int, ftglSetFontCharMap, (FTGLfont *f, FT_Encoding enc),
      return 0, CharMap, (enc));

// unsigned int FTFont::CharMapCount();
C_FUN(unsigned int, ftglGetFontCharMapCount, (FTGLfont *f),
      return 0, CharMapCount, ());

// FT_Encoding* FTFont::CharMapList();
C_FUN(FT_Encoding *, ftglGetFontCharMapList, (FTGLfont* f),
      return NULL, CharMapList, ());

// virtual bool FTFont::FaceSize(const unsigned int size,
//                               const unsigned int res = 72);
C_FUN(int, ftglSetFontFaceSize, (FTGLfont *f, unsigned int s, unsigned int r),
      return 0, FaceSize, (s, r > 0 ? r : 72));

// unsigned int FTFont::FaceSize() const;
// XXX: need to call FaceSize() as FTFont::FaceSize() because of FTGLTexture
C_FUN(unsigned int, ftglGetFontFaceSize, (FTGLfont *f),
      return 0, FTFont::FaceSize, ());

// virtual void FTFont::Depth(float depth);
C_FUN(void, ftglSetFontDepth, (FTGLfont *f, float d), return, Depth, (d));

// virtual void FTFont::Outset(float front, float back);
C_FUN(void, ftglSetFontOutset, (FTGLfont *f, float front, float back),
      return, FTFont::Outset, (front, back));

// void FTFont::UseDisplayList(bool useList);
C_FUN(void, ftglSetFontDisplayList, (FTGLfont *f, int l),
      return, UseDisplayList, (l != 0));

// float FTFont::Ascender() const;
C_FUN(float, ftglGetFontAscender, (FTGLfont *f), return 0.f, Ascender, ());

// float FTFont::Descender() const;
C_FUN(float, ftglGetFontDescender, (FTGLfont *f), return 0.f, Descender, ());

// float FTFont::LineHeight() const;
C_FUN(float, ftglGetFontLineHeight, (FTGLfont *f), return 0.f, LineHeight, ());

// void FTFont::BBox(const char* string, float& llx, float& lly, float& llz,
//                   float& urx, float& ury, float& urz);
C_FUN(void, ftglGetFontBBox, (FTGLfont *f, const char* s, int start, int end,
                              float c[6]),
      return, BBox, (s, start, end, c[0], c[1], c[2], c[3], c[4], c[5]));

// float FTFont::Advance(const char* string);
C_FUN(float, ftglGetFontAdvance, (FTGLfont *f, const char* s),
      return 0.f, Advance, (s));

// virtual void Render(const char* string, int renderMode);
C_FUN(void, ftglRenderFont, (FTGLfont *f, const char *s, int r),
      return, Render, (s, r));

// FT_Error FTFont::Error() const;
C_FUN(FT_Error, ftglGetFontError, (FTGLfont *f), return -1, Error, ());

FTGL_END_C_DECLS

