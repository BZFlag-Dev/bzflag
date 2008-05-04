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

#ifndef __FTOutlineFontImpl__
#define __FTOutlineFontImpl__

#include "FTFontImpl.h"

class FTGlyph;

class FTOutlineFontImpl : public FTFontImpl
{
    friend class FTOutlineFont;

    protected:
        FTOutlineFontImpl(const char* fontFilePath) :
            FTFontImpl(fontFilePath), outset(0.0f) {};

        FTOutlineFontImpl(const unsigned char *pBufferBytes,
                          size_t bufferSizeInBytes) :
            FTFontImpl(pBufferBytes, bufferSizeInBytes), outset(0.0f) {};

        /**
         * Set the outset distance for the font. Only implemented by
         * FTOutlineFont, FTPolygonFont and FTExtrudeFont
         *
         * @param outset  The outset distance.
         */
        virtual void Outset(float o) { outset = o; }

        /**
         * Renders a string of characters
         *
         * @param string    'C' style string to be output.
         */
        virtual void Render(const char* string);

        /**
         * Render a string of characters
         *
         * @param string    'C' style string to be output.
         * @param renderMode    Render mode to display
         */
        virtual void Render(const char* string, int renderMode);

        /**
         * Renders a string of characters
         *
         * @param string    wchar_t string to be output.
         */
        virtual void Render(const wchar_t* string);

        /**
         * Render a string of characters
         *
         * @param string    wchar_t string to be output.
         * @param renderMode    Render mode to display
         */
        virtual void Render(const wchar_t *string, int renderMode);

    private:
        /**
         * Construct a FTOutlineGlyph.
         *
         * @param g The glyph index NOT the char code.
         * @return  An FTOutlineGlyph or <code>null</code> on failure.
         */
        inline virtual FTGlyph* MakeGlyph(unsigned int g);

        /**
         * The outset distance for the font.
         */
        float outset;

        /* Internal generic Render() implementation */
        template <typename T>
        inline void RenderI(const T* string);
};

#endif // __FTOutlineFontImpl__

