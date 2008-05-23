/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
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
#   warning Please use <FTGL/ftgl.h> instead of <FTBuffer.h>.
#   include <FTGL/ftgl.h>
#endif

#ifndef __FTBuffer__
#define __FTBuffer__

#ifdef __cplusplus

/**
 * FTBuffer is a helper class for pixel buffers.
 *
 * It provides the interface between FTBufferFont and FTBufferGlyph to
 * optimise rendering operations.
 *
 * @see FTBufferGlyph
 * @see FTBufferFont
 */
class FTGL_EXPORT FTBuffer
{
    public:
        /**
         * Default constructor.
         */
        FTBuffer();

        /**
         * Destructor
         */
        ~FTBuffer();

        inline FTPoint Pos() const
        {
            return pos;
        }

        inline void Pos(FTPoint arg)
        {
            pos = arg;
        }

        int width, height, pitch;
        unsigned char *pixels;

    private:
        FTPoint pos;
};

#endif //__cplusplus

#endif // __FTBuffer__

