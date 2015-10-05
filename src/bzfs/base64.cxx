/* base64.hpp - base64 encoder/decoder implementing section 6.8 of RFC2045
 *
 * Copyright (C) 2002 Ryan Petrie (ryanpetrie@netscape.net)
 * and released under the zlib license:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

namespace base64 {
  const char _to_table[64] =  {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
  };
  const char* to_table = _to_table;
  const char* to_table_end = _to_table + sizeof(_to_table);

  const signed char _from_table[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, // 0
    -1, -1, -1, -1, -1, -1, -1, -1, // 8
    -1, -1, -1, -1, -1, -1, -1, -1, // 16
    -1, -1, -1, -1, -1, -1, -1, -1, // 24
    -1, -1, -1, -1, -1, -1, -1, -1, // 32
    -1, -1, -1, 62, -1, -1, -1, 63, // 40
    52, 53, 54, 55, 56, 57, 58, 59, // 48
    60, 61, -1, -1, -1,  0, -1, -1, // 56
    -1,  0,  1,  2,  3,  4,  5,  6, // 64
     7,  8,  9, 10, 11, 12, 13, 14, // 72
    15, 16, 17, 18, 19, 20, 21, 22, // 80
    23, 24, 25, -1, -1, -1, -1, -1, // 88
    -1, 26, 27, 28, 29, 30, 31, 32, // 96
    33, 34, 35, 36, 37, 38, 39, 40, // 104
    41, 42, 43, 44, 45, 46, 47, 48, // 112
    49, 50, 51, -1, -1, -1, -1, -1  // 120
  };
  const signed char* from_table = _from_table;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
