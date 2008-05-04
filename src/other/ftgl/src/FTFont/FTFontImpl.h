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

#ifndef __FTFontImpl__
#define __FTFontImpl__

#include "FTGL/ftgl.h"

#include "FTFace.h"

class FTGlyphContainer;
class FTGlyph;
class FTLayout;

class FTFontImpl
{
        /* Allow FTLayout classes to access DoRender and CheckGlyph */
        friend class FTLayoutImpl;
        friend class FTFont;

    protected:
        FTFontImpl(char const *fontFilePath);


        FTFontImpl(const unsigned char *pBufferBytes, size_t bufferSizeInBytes);

        virtual ~FTFontImpl();

        virtual void Render(const char* string);

        virtual void Render(const char* string, int renderMode);

        virtual void Render(const wchar_t* string);

        virtual void Render(const wchar_t *string, int renderMode);

        virtual bool FaceSize(const unsigned int size,
                              const unsigned int res);

        virtual unsigned int FaceSize() const;

        virtual void Depth(float depth);

        virtual void Outset(float outset);

        virtual void Outset(float front, float back);

        void BBox(const char *string, const int start, const int end,
                  float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz);

        void BBox(const wchar_t *string, const int start, const int end,
                  float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz);

        float Advance(const wchar_t* string);

        float Advance(const char* string);

        /**
         * Construct a glyph of the correct type.
         *
         * Clients must override the function and return their specialised
         * FTGlyph.
         *
         * @param g The glyph index NOT the char code.
         * @return  An FT****Glyph or <code>null</code> on failure.
         */
        virtual FTGlyph* MakeGlyph(unsigned int g) = 0;

        /**
         * Current face object
         */
        FTFace face;

        /**
         * Current size object
         */
        FTSize charSize;

        /**
         * Flag to enable or disable the use of Display Lists inside FTGL
         * <code>true</code> turns ON display lists.
         * <code>false</code> turns OFF display lists.
         */
        bool useDisplayLists;

        /**
         * Current error code. Zero means no error.
         */
        FT_Error err;

    private:
        /**
         * Render a character.
         * This function does an implicit conversion on its arguments.
         *
         * @param chr       current character
         * @param nextChr   next character
         * @param origin       The position of the origin of the character.
         *                  After rendering the point referenced by origin
         *                  will be incremented by the kerning advance of
         *                  char and nextChr.
         * @param renderMode    Render mode to display
         */
        void DoRender(const unsigned int chr,
                      const unsigned int nextChr, FTPoint &origin,
                      int renderMode);

        /**
         * Check that the glyph at <code>chr</code> exist. If not load it.
         *
         * @param chr  character index
         * @return <code>true</code> if the glyph can be created.
         */
        bool CheckGlyph(const unsigned int chr);

        /**
         * An object that holds a list of glyphs
         */
        FTGlyphContainer* glyphList;

        /**
         * Current pen or cursor position;
         */
        FTPoint pen;

        /* Internal generic BBox() implementation */
        template <typename T>
        inline void BBoxI(const T *string, const int start, const int end,
                          float& llx, float& lly, float& llz,
                          float& urx, float& ury, float& urz);

        /* Internal generic BBox() implementation */
        template <typename T>
        inline float AdvanceI(const T* string);

        /* Internal generic Render() implementation */
        template <typename T>
        inline void RenderI(const T* string, int renderMode);
};

#endif  //  __FTFontImpl__

