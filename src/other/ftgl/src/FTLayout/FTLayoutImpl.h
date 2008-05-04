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

#ifndef __FTLayoutImpl__
#define __FTLayoutImpl__

#include "FTSize.h"
#include "FTGlyphContainer.h"


class FTLayoutImpl
{
        friend class FTLayout;

    protected:
        FTLayoutImpl();

        virtual ~FTLayoutImpl();

        /**
         * Get the bounding box for a formatted string.
         *
         * @param string    a char string
         * @param llx       lower left near x coord
         * @param lly       lower left near y coord
         * @param llz       lower left near z coord
         * @param urx       upper right far x coord
         * @param ury       upper right far y coord
         * @param urz       upper right far z coord
         */
        virtual void BBox(const char* string, float& llx, float& lly,
                          float& llz, float& urx, float& ury, float& urz) = 0;

        /**
         * Get the bounding box for a formatted string.
         *
         * @param string    a wchar_t string
         * @param llx       lower left near x coord
         * @param lly       lower left near y coord
         * @param llz       lower left near z coord
         * @param urx       upper right far x coord
         * @param ury       upper right far y coord
         * @param urz       upper right far z coord
         */
        virtual void BBox(const wchar_t* string, float& llx, float& lly,
                          float& llz, float& urx, float& ury, float& urz) = 0;

        /**
         * Render a string of characters
         *
         * @param string    'C' style string to be output.
         */
        virtual void Render(const char *string) = 0;

        /**
         * Render a string of characters
         *
         * @param string    'C' style string to be output.
         * @param renderMode  Render mode to diplay
         */
        virtual void Render(const char *string, int renderMode) = 0;

        /**
         * Render a string of characters
         *
         * @param string    wchar_t string to be output.
         */
        virtual void Render(const wchar_t *string) = 0;

        /**
         * Render a string of characters
         *
         * @param string    wchar_t string to be output.
         * @param renderMode  Render mode to diplay
         */
        virtual void Render(const wchar_t *string, int renderMode) = 0;

    protected:
        /**
         * Current pen or cursor position;
         */
        FTPoint pen;

        /**
         * Expose <code>FTFont::DoRender</code> method to derived classes.
         *
         * @param font      The font that contains the glyph.
         * @param chr       current character
         * @param nextChr   next character
         * @param renderMode  Render mode to diplay
         * @see FTFont::DoRender
         */
        void DoRender(FTFont *font, const unsigned int chr,
                      const unsigned int nextChr, int renderMode);

        /**
         * Expose <code>FTFont::CheckGlyph</code> method to derived classes.
         *
         * @param font The font that contains the glyph.
         * @param chr  character index
         */
        void CheckGlyph(FTFont *font, const unsigned int Chr);

        /**
         * Expose the FTFont <code>glyphList</code> to our derived classes.
         *
         * @param font The font to perform the query on.
         * @param Char The character corresponding to the glyph to query.
         *
         * @return A pointer to the glyphList of font.
         */
        FTGlyphContainer *GetGlyphs(FTFont *font);

        /**
         * Expose the FTFont <code>charSize</code> to our derived classes.
         *
         * @param The font to perform the query on.
         *
         * @return A reference to the charSize object of font.
         */
        FTSize &GetCharSize(FTFont *font);

        /**
         * Current error code. Zero means no error.
         */
        FT_Error err;
};

#endif  //  __FTLayoutImpl__

