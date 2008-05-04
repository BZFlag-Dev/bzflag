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
    FTGLlayout* cname cargs \
    { \
        cxxname *l = new cxxname cxxarg; \
        if(l->Error()) \
        { \
            delete l; \
            return NULL; \
        } \
        FTGLlayout *ftgl = (FTGLlayout *)malloc(sizeof(FTGLlayout)); \
        ftgl->ptr = l; \
        ftgl->type = cxxtype; \
        return ftgl; \
    }

// FTSimpleLayout::FTSimpleLayout();
C_TOR(ftglCreateSimpleLayout, (), FTSimpleLayout, (), LAYOUT_SIMPLE);

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
            case FTGL::LAYOUT_SIMPLE: \
                return dynamic_cast<FTSimpleLayout*>(f->ptr)->cxxname cxxarg; \
        } \
        fprintf(stderr, "FTGL warning: %s not implemented for %d\n", #cname, f->type); \
        cxxerr; \
    }

// FTLayout::~FTLayout();
void ftglDestroyLayout(FTGLlayout *l)
{
    if(!l || !l->ptr)
    {
        fprintf(stderr, "FTGL warning: NULL pointer in %s\n", __FUNCTION__);
        return;
    }
    switch(l->type)
    {
        case FTGL::LAYOUT_SIMPLE:
            delete dynamic_cast<FTSimpleLayout*>(l->ptr); break;
        default:
            fprintf(stderr, "FTGL warning: %s not implemented for %d\n",
                            __FUNCTION__, l->type);
    }

    l->ptr = NULL;
    free(l);
}

// virtual void BBox(const char* string, float& llx, float& lly, float& llz, float& urx, float& ury, float& urz)
C_FUN(void, ftgGetlLayoutBBox, (FTGLlayout *f, const char * s, float c[6]),
      return, BBox, (s, c[0], c[1], c[2], c[3], c[4], c[5]));

// virtual void Render(const char* string, int renderMode);
C_FUN(void, ftglRenderLayout, (FTGLlayout *f, const char *s, int r),
      return, Render, (s, r));

// void RenderSpace(const char *string, const float ExtraSpace = 0.0)
C_FUN(void, ftglRenderLayoutSpace, (FTGLlayout *f, const char *s, float e),
      return, RenderSpace, (s, e));

// void SetFont(FTFont *fontInit)
void ftglSetLayoutFont(FTGLlayout *f, FTGLfont *font)
{
    if(!f || !f->ptr)
    {
        //XXX fprintf(stderr, "FTGL warning: NULL pointer in %s\n", __func__);
        return;
    }
    switch(f->type)
    {
        case FTGL::LAYOUT_SIMPLE:
            f->font = font;
            return dynamic_cast<FTSimpleLayout*>(f->ptr)->SetFont(font->ptr);
    }
    fprintf(stderr, "FTGL warning: %s not implemented for %d\n",
                    __FUNCTION__, f->type);
}

// FTFont *GetFont()
FTGLfont *ftglGetLayoutFont(FTGLlayout *f)
{
    if(!f || !f->ptr)
    {
        fprintf(stderr, "FTGL warning: NULL pointer in %s\n", __FUNCTION__);
        return NULL;
    }
    return f->font;
}

// void SetLineLength(const float LineLength);
C_FUN(void, ftglSetLayoutLineLength, (FTGLlayout *f, const float length),
      return, SetLineLength, (length));

// float GetLineLength() const
C_FUN(float, ftglGetLayoutLineLength, (FTGLlayout *f),
      return 0.0f, GetLineLength, ());

// void SetAlignment(const TextAlignment Alignment)
C_FUN(void, ftglSetLayoutAlignment, (FTGLlayout *f, const int a),
      return, SetAlignment, ((FTGL::TextAlignment)a));

// TextAlignment GetAlignment() const
C_FUN(int, ftglGetLayoutAlignement, (FTGLlayout *f),
      return FTGL::ALIGN_LEFT, GetAlignment, ());

// void SetLineSpacing(const float LineSpacing)
C_FUN(void, ftglSetLayoutLineSpacing, (FTGLlayout *f, const float l),
      return, SetLineSpacing, (l));

// float GetLineSpacing() const
C_FUN(float, ftglGetLayoutLineSpacing, (FTGLlayout *f),
      return 0.0f, GetLineSpacing, ());

// FT_Error FTLayout::Error() const;
C_FUN(FT_Error, ftglGetLayoutError, (FTGLlayout *f), return -1, Error, ());

FTGL_END_C_DECLS

