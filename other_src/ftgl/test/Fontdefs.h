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

#ifndef    __Font_defs__
#define    __Font_defs__


const char* const BAD_FONT_FILE   = "missing_font.ttf";
const char* const GOOD_FONT_FILE  = "../../test/font_pack/MHei-Medium-Acro";
const char* const ARIAL_FONT_FILE = "../../test/font_pack/arial.ttf";
const char* const FONT_FILE       = "../../test/font_pack/times.ttf";
const char* const TYPE1_FONT_FILE = "../../test/font_pack/HPGCalc.pfb";
const char* const TYPE1_AFM_FILE  = "../../test/font_pack/HPGCalc.afm";

const char*    const GOOD_ASCII_TEST_STRING        = "test string";
const char*    const BAD_ASCII_TEST_STRING         = "";
const wchar_t        GOOD_UNICODE_TEST_STRING[4]   = { 0x6FB3, 0x9580, 0x0};
const wchar_t* const BAD_UNICODE_TEST_STRING       = L"";

const unsigned int FONT_POINT_SIZE  = 72;
const unsigned int RESOLUTION = 72;

const unsigned int CHARACTER_CODE_A        = 'A';
const unsigned int CHARACTER_CODE_G        = 'g';
const unsigned int BIG_CHARACTER_CODE      = 0x6FB3;
const unsigned int NULL_CHARACTER_CODE     = 512;
const unsigned int NULL_CHARACTER_INDEX    = ' ';
const unsigned int SIMPLE_CHARACTER_INDEX  = 'i';
const unsigned int COMPLEX_CHARACTER_INDEX = 'd';

const unsigned int FONT_INDEX_OF_A = 34;
const unsigned int BIG_FONT_INDEX  = 4838;
const unsigned int NULL_FONT_INDEX = 0;

const unsigned int NUMBER_OF_GLYPHS = 50;
const unsigned int TOO_MANY_GLYPHS  = 14100; // MHei-Medium-Acro has 14099


#include "HPGCalc_pfb.cpp"
#include "HPGCalc_afm.cpp"

#endif // __Font_defs__
