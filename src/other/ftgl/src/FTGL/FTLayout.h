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

#ifndef __ftgl__
#   warning This header is deprecated. Please use <FTGL/ftgl.h> from now.
#   include <FTGL/ftgl.h>
#endif

#ifndef __FTLayout__
#define __FTLayout__

#ifdef __cplusplus


class FTLayoutImpl;

/**
 * FTLayout is the interface for layout managers that render text.
 *
 * Specific layout manager classes are derived from this class. This class
 * is abstract and deriving classes must implement the protected
 * <code>Render</code> methods to render formatted text and
 * <code>BBox</code> methods to determine the bounding box of output text.
 *
 * @see     FTFont
 */
class FTGL_EXPORT FTLayout
{
    protected:
        FTLayout();

    public:
        virtual ~FTLayout();

        void BBox(const char* string, float& llx, float& lly,
                  float& llz, float& urx, float& ury, float& urz);

        void BBox(const wchar_t* string, float& llx, float& lly,
                  float& llz, float& urx, float& ury, float& urz);

        void Render(const char *string);

        void Render(const char *string, int renderMode);

        void Render(const wchar_t *string);

        void Render(const wchar_t *string, int renderMode);

        FT_Error Error() const;

    protected:
        FTLayoutImpl *impl;
};

#endif //__cplusplus

FTGL_BEGIN_C_DECLS

/**
 * FTGLlayout is the interface for layout managers that render text.
 */
struct _FTGLlayout;
typedef struct _FTGLlayout FTGLlayout;

/**
 * Destroy an FTGL layout object.
 *
 * @param layout  An FTGLlayout* object.
 */
FTGL_EXPORT void ftglDestroyLayout(FTGLlayout* layout);

/**
 * Get the bounding box for a string.
 *
 * @param layout  An FTGLlayout* object.
 * @param string  A char buffer
 * @param bounds  An array of 6 float values where the bounding box's lower
 *                left near and upper right far 3D coordinates will be stored.
 */
FTGL_EXPORT void ftglGetLayoutBBox(FTGLlayout *layout, const char* string,
                                   float bounds[6]);

/**
 * Render a string of characters.
 *
 * @param layout  An FTGLlayout* object.
 * @param string  Char string to be output.
 * @param mode  Render mode to display.
 */
FTGL_EXPORT void ftglRenderLayout(FTGLlayout *layout, const char *string,
                                  int mode);

/**
 * Query a layout for errors.
 *
 * @param layout  An FTGLlayout* object.
 * @return  The current error code.
 */
FTGL_EXPORT FT_Error ftglGetLayoutError(FTGLlayout* layout);

FTGL_END_C_DECLS

#endif  /* __FTLayout__ */

