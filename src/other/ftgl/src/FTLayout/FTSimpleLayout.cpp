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

#include <ctype.h>

#include "FTInternals.h"

#include "FTGlyphContainer.h"
#include "FTSimpleLayoutImpl.h"


//
//  FTSimpleLayout
//


FTSimpleLayout::FTSimpleLayout()
{
    impl = new FTSimpleLayoutImpl();
}


void FTSimpleLayout::SetFont(FTFont *fontInit)
{
    dynamic_cast<FTSimpleLayoutImpl*>(impl)->currentFont = fontInit;
}


FTFont *FTSimpleLayout::GetFont()
{
    return dynamic_cast<FTSimpleLayoutImpl*>(impl)->currentFont;
}


void FTSimpleLayout::SetLineLength(const float LineLength)
{
    dynamic_cast<FTSimpleLayoutImpl*>(impl)->lineLength = LineLength;
}


float FTSimpleLayout::GetLineLength() const
{
    return dynamic_cast<FTSimpleLayoutImpl*>(impl)->lineLength;
}


void FTSimpleLayout::SetAlignment(const FTGL::TextAlignment Alignment)
{
    dynamic_cast<FTSimpleLayoutImpl*>(impl)->alignment = Alignment;
}


FTGL::TextAlignment FTSimpleLayout::GetAlignment() const
{
    return dynamic_cast<FTSimpleLayoutImpl*>(impl)->alignment;
}


void FTSimpleLayout::SetLineSpacing(const float LineSpacing)
{
    dynamic_cast<FTSimpleLayoutImpl*>(impl)->lineSpacing = LineSpacing;
}


float FTSimpleLayout::GetLineSpacing() const
{
    return dynamic_cast<FTSimpleLayoutImpl*>(impl)->lineSpacing;
}


void FTSimpleLayout::RenderSpace(const char *string, const float ExtraSpace)
{
    dynamic_cast<FTSimpleLayoutImpl*>(impl)->RenderSpace(string, ExtraSpace);
}


void FTSimpleLayout::RenderSpace(const wchar_t *string, const float ExtraSpace)
{
    dynamic_cast<FTSimpleLayoutImpl*>(impl)->RenderSpace(string, ExtraSpace);
}


//
//  FTSimpleLayoutImpl
//


FTSimpleLayoutImpl::FTSimpleLayoutImpl()
{
    currentFont = NULL;
    lineLength = 100.0f;
    alignment = FTGL::ALIGN_LEFT;
    lineSpacing = 1.0f;
}


template <typename T>
inline void FTSimpleLayoutImpl::BBoxI(const T* string,
                                      float& llx, float& lly, float& llz,
                                      float& urx, float& ury, float& urz)
{
    FTBBox bounds;

    WrapText(string, 0, &bounds);
    llx = bounds.Lower().X(); lly = bounds.Lower().Y(); llz = bounds.Lower().Z();
    urx = bounds.Upper().X(); ury = bounds.Upper().Y(); urz = bounds.Upper().Z();
}


void FTSimpleLayoutImpl::BBox(const char *string, float& llx, float& lly,
                              float& llz, float& urx, float& ury, float& urz)
{
    BBoxI(string, llx, lly, llz, urx, ury, urz);
}


void FTSimpleLayoutImpl::BBox(const wchar_t *string, float& llx, float& lly,
                              float& llz, float& urx, float& ury, float& urz)
{
    BBoxI(string, llx, lly, llz, urx, ury, urz);
}


template <typename T>
inline void FTSimpleLayoutImpl::RenderI(const T *string, int renderMode)
{
    pen = FTPoint(0.0f, 0.0f);
    WrapText(string, renderMode, NULL);
}


void FTSimpleLayoutImpl::Render(const char *string)
{
    RenderI(string, FTGL::RENDER_FRONT | FTGL::RENDER_BACK | FTGL::RENDER_SIDE);
}


void FTSimpleLayoutImpl::Render(const wchar_t* string)
{
    RenderI(string, FTGL::RENDER_FRONT | FTGL::RENDER_BACK | FTGL::RENDER_SIDE);
}


void FTSimpleLayoutImpl::Render(const char *string, int renderMode)
{
    RenderI(string, renderMode);
}


void FTSimpleLayoutImpl::Render(const wchar_t* string, int renderMode)
{
    RenderI(string, renderMode);
}


void FTSimpleLayoutImpl::RenderSpace(const char *string,
                                     const float ExtraSpace)
{
    pen.X(0);
    pen.Y(0);
    RenderSpace(string, 0, -1, 0, ExtraSpace);
}


void FTSimpleLayoutImpl::RenderSpace(const wchar_t *string,
                                     const float ExtraSpace)
{
    pen.X(0);
    pen.Y(0);
    RenderSpace(string, 0, -1, 0, ExtraSpace);
}


template <typename T>
inline void FTSimpleLayoutImpl::WrapTextI(const T *buf, int renderMode,
                                          FTBBox *bounds)
{
    int breakIdx = 0;          // index of the last break character
    int lineStart = 0;         // character index of the line start
    float nextStart = 0.0;     // total width of the current line
    float breakWidth = 0.0;    // width of the line up to the last word break
    float currentWidth = 0.0;  // width of all characters on the current line
    float prevWidth;           // width of all characters but the current glyph
    float wordLength = 0.0;    // length of the block since the last break char
    float glyphWidth, advance;
    FTBBox glyphBounds;

    // Reset the pen position
    pen.Y(0);

    // If we have bounds mark them invalid
    if(bounds)
    {
        bounds->Invalidate();
    }

    // Scan the input for all characters that need output
    for(int i = 0; buf[i]; i++)
    {
        // Find the width of the current glyph
        CheckGlyph(currentFont, buf[i]);
        glyphBounds = GetGlyphs(currentFont)->BBox(buf[i]);
        glyphWidth = glyphBounds.Upper().X() - glyphBounds.Lower().X();

        advance = GetGlyphs(currentFont)->Advance(buf[i], buf[i + 1]);
        prevWidth = currentWidth;
        // Compute the width of all glyphs up to the end of buf[i]
        currentWidth = nextStart + glyphWidth;
        // Compute the position of the next glyph
        nextStart += advance;

        // See if buf[i] is a space, a break or a regular character
        if((currentWidth > lineLength) || (buf[i] == '\n'))
        {
            // A non whitespace character has exceeded the line length.  Or a
            // newline character has forced a line break.  Output the last
            // line and start a new line after the break character.
            // If we have not yet found a break, break on the last character
            if(!breakIdx || (buf[i] == '\n'))
            {
                // Break on the previous character
                breakIdx = i - 1;
                breakWidth = prevWidth;
                // None of the previous words will be carried to the next line
                wordLength = 0;
                // If the current character is a newline discard its advance
                if(buf[i] == '\n') advance = 0;
            }

            float remainingWidth = lineLength - breakWidth;

            // Render the current substring
            // If the break character is a newline do not render it
            if(buf[breakIdx + 1] == '\n')
            {
                breakIdx++;
                OutputWrapped(buf, lineStart, breakIdx - 1,
                              remainingWidth, bounds, renderMode);
            }
            else
            {
                OutputWrapped(buf, lineStart, breakIdx, remainingWidth, bounds, renderMode);
            }

            // Store the start of the next line
            lineStart = breakIdx + 1;
            // TODO: Is Height() the right value here?
            pen -= FTPoint(0, GetCharSize(currentFont).Height() * lineSpacing);
            // The current width is the width since the last break
            nextStart = wordLength + advance;
            wordLength += advance;
            currentWidth = wordLength + advance;
            // Reset the safe break for the next line
            breakIdx = 0;
        }
        else if(isspace(buf[i]))
        {
            // This is the last word break position
            wordLength = 0;
            breakIdx = i;

            // Check to see if this is the first whitespace character in a run
            if(!i || !isspace(buf[i - 1]))
            {
                // Record the width of the start of the block
                breakWidth = currentWidth;
            }
        }
        else
        {
            wordLength += advance;
        }
    }

    float remainingWidth = lineLength - currentWidth;
    // Render any remaining text on the last line
    // Disable justification for the last row
    if(alignment == FTGL::ALIGN_JUSTIFY)
    {
        alignment = FTGL::ALIGN_LEFT;
        OutputWrapped(buf, lineStart, -1, remainingWidth, bounds, renderMode);
        alignment = FTGL::ALIGN_JUSTIFY;
    }
    else
    {
        OutputWrapped(buf, lineStart, -1, remainingWidth, bounds, renderMode);
    }
}


void FTSimpleLayoutImpl::WrapText(const char *buf, int renderMode,
                                  FTBBox *bounds)
{
    WrapTextI(buf, renderMode, bounds);
}


void FTSimpleLayoutImpl::WrapText(const wchar_t* buf, int renderMode,
                                  FTBBox *bounds)
{
    WrapTextI(buf, renderMode, bounds);
}


template <typename T>
inline void FTSimpleLayoutImpl::OutputWrappedI(const T *buf, const int start,
                                               const int end,
                                               const float remaining,
                                               FTBBox *bounds, int renderMode)
{
    float distributeWidth = 0.0;
    // Align the text according as specified by Alignment
    switch (alignment)
    {
        case FTGL::ALIGN_LEFT:
            pen.X(0);
            break;
        case FTGL::ALIGN_CENTER:
            pen.X(remaining / 2);
            break;
        case FTGL::ALIGN_RIGHT:
            pen.X(remaining);
            break;
        case FTGL::ALIGN_JUSTIFY:
            pen.X(0);
            distributeWidth = remaining;
            break;
    }

    // If we have bounds expand them by the line's bounds, otherwise render
    // the line.
    if(bounds)
    {
        float llx, lly, llz, urx, ury, urz;
        currentFont->BBox(buf, start, end, llx, lly, llz, urx, ury, urz);

        // Add the extra space to the upper x dimension
        urx += distributeWidth;
        // TODO: It's a little silly to convert from a FTBBox to floats and
        // back again, but I don't want to implement yet another method for
        // finding the bounding box as a BBox.
        FTBBox temp(llx, lly, llz, urx, ury, urz);
        temp.Move(FTPoint(pen.X(), pen.Y(), 0.0f));

        // See if this is the first area to be added to the bounds
        if(!bounds->IsValid())
        {
            *bounds = temp;
        }
        else
        {
            *bounds += temp;
        }
    }
    else
    {
        RenderSpace(buf, start, end, renderMode, distributeWidth);
    }
}


void FTSimpleLayoutImpl::OutputWrapped(const char *buf, const int start,
                                       const int end, const float remaining,
                                       FTBBox *bounds, int renderMode)
{
    OutputWrappedI(buf, start, end, remaining, bounds, renderMode);
}


void FTSimpleLayoutImpl::OutputWrapped(const wchar_t *buf, const int start,
                                       const int end, const float remaining,
                                       FTBBox *bounds, int renderMode)
{
    OutputWrappedI(buf, start, end, remaining, bounds, renderMode);
}


template <typename T>
inline void FTSimpleLayoutImpl::RenderSpaceI(const T *string, const int start,
                                             const int end, int renderMode,
                                             const float ExtraSpace)
{
    float space = 0.0;

    // If there is space to distribute, count the number of spaces
    if(ExtraSpace > 0.0)
    {
        int numSpaces = 0;

        // Count the number of space blocks in the input
        for(int i = start; ((end < 0) && string[i])
                              || ((end >= 0) && (i <= end)); i++)
        {
            // If this is the end of a space block, increment the counter
            if((i > start) && !isspace(string[i]) && isspace(string[i - 1]))
            {
                numSpaces++;
            }
        }

        space = ExtraSpace/numSpaces;
    }

    // Output all characters of the string
    for(int i = start; ((end < 0) && string[i])
                          || ((end >= 0) && (i <= end)); i++)
    {
        // If this is the end of a space block, distribute the extra space
        // inside it
        if((i > start) && !isspace(string[i]) && isspace(string[i - 1]))
        {
            pen += FTPoint(space, 0);
        }

        DoRender(currentFont, string[i], string[i + 1], renderMode);
    }
}


void FTSimpleLayoutImpl::RenderSpace(const char *string, const int start,
                                     const int end, int renderMode,
                                     const float ExtraSpace)
{
    RenderSpaceI(string, start, end, renderMode, ExtraSpace);
}


void FTSimpleLayoutImpl::RenderSpace(const wchar_t *string, const int start,
                                     const int end, int renderMode,
                                     const float ExtraSpace)
{
    RenderSpaceI(string, start, end, renderMode, ExtraSpace);
}

