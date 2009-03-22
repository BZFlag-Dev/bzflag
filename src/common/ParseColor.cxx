/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ParseColor.h"

/* common system headers */
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <map>

/* common implementation headers */
#include "TextUtils.h"


//============================================================================//
//============================================================================//

struct ColorData {
  ColorData()
  {
    data[0] = data[1] = data[2] = data[3] = 1.0f; 
  }
  ColorData(float r, float g, float b, float a = 1.0f)
  {
    data[0] = r;
    data[1] = g;
    data[2] = b;
    data[3] = a;
  }
  float data[4];
};

typedef std::map<std::string, ColorData> ColorMap;

static const ColorMap& getColorMap();


//============================================================================//
//============================================================================//

static int parseHexChar(char c)
{
  if ((c >= '0') && (c <= '9')) {
    return (c - '0');
  }
  c = tolower(c);
  if ((c >= 'a') && (c <= 'f')) {
    return (c - 'a') + 10;
  }
  return -1;
}


static bool parseHexFormat(const char* str, float color[4])
{
  str++; // skip the '#'

  int bytes[8];
  int index;
  for (index = 0; index < 8; index++) {
    const char c = str[index];
    if ((c == 0) || isspace(c)) {
      break;
    }
    const int byte = parseHexChar(c);
    if (byte < 0) {
      return false; // not a hex character
    }
    bytes[index] = byte;
  }

  // check for a valid termination
  if (index == 8) {
    const char c = str[8];
    if ((c != 0) && !isspace(c)) {
      return false;
    }
  }

  switch (index) {
    case 3: { // #rgb
      color[0] = (float)bytes[0] / 15.0f;
      color[1] = (float)bytes[1] / 15.0f;
      color[2] = (float)bytes[2] / 15.0f;
      return true;
    }
    case 4: { // #rgba
      color[0] = (float)bytes[0] / 15.0f;
      color[1] = (float)bytes[1] / 15.0f;
      color[2] = (float)bytes[2] / 15.0f;
      color[3] = (float)bytes[3] / 15.0f;
      return true;
    }
    case 6: { // #rrggbb
      color[0] = (float)((bytes[0] << 4) + bytes[1]) / 255.0f;
      color[1] = (float)((bytes[2] << 4) + bytes[3]) / 255.0f;
      color[2] = (float)((bytes[4] << 4) + bytes[5]) / 255.0f;
      return true;
    }
    case 8: { // #rrggbbaa
      color[0] = (float)((bytes[0] << 4) + bytes[1]) / 255.0f;
      color[1] = (float)((bytes[2] << 4) + bytes[3]) / 255.0f;
      color[2] = (float)((bytes[4] << 4) + bytes[5]) / 255.0f;
      color[3] = (float)((bytes[6] << 4) + bytes[7]) / 255.0f;
      return true;
    }
  }

  return false;
}


//============================================================================//
//============================================================================//

static bool parseFloatFormat(const char* str, float color[4])
{
  int count;
  float tmp[4];
  count = sscanf(str, "%f %f %f %f", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
  if (count < 3) {
    return false;
  }
  memcpy(color, tmp, count * sizeof(float));
  return true;
}


//============================================================================//
//============================================================================//

static bool parseNamedFormat(const char* str, float color[4])
{
  const char* end = TextUtils::skipNonWhitespace(str);
  const size_t nameLen = (end - str);
  if (nameLen <= 0) {
    return false;
  }
  const std::string name(str, nameLen);

  const ColorMap& colorMap = getColorMap();
  ColorMap::const_iterator it = colorMap.find(TextUtils::tolower(name));
  if (it == colorMap.end()) {
    return false;
  }
  memcpy(color, it->second.data, sizeof(float[3]));

  str = TextUtils::skipWhitespace(end);
  if (*str == 0) {
    return true;
  }
    
  float alpha;
  if (sscanf(str, "%f", &alpha) > 0) {
    color[3] = alpha;
  }

  return true;
}


//============================================================================//
//============================================================================//

bool parseColorCString(const char* str, float color[4])
{
  const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

  // default to opaque white
  memcpy(color, white, sizeof(float[4]));

  // strip leading space
  str = TextUtils::skipWhitespace(str);

  // no string
  if (*str == 0) {
    return false;
  }

  // hexadecimal format (#rgb, #rgba, #rrggbb #rrggbbaa)
  if (*str == '#') {
    return parseHexFormat(str, color);
  }

  // numeric format (either 3 or 4 floating point values)
  if (((*str >= '0') && (*str <= '9'))
      || (*str == '.')
      || (*str == '+')
      || (*str == '-')) {
    return parseFloatFormat(str, color);
  }

  // named string format  ("red 0.2" format is accepted for alpha values)
  return parseNamedFormat(str, color);
}


bool parseColorStream(std::istream& input, float color[4])
{
  std::string line;
  std::getline(input, line);
  input.putback('\n');

  return parseColorString(line, color);
}


bool parseColorString(const std::string& str, float color[4])
{
  return parseColorCString(str.c_str(), color);
}


//============================================================================//
//============================================================================//

static const ColorMap& getColorMap()
{
  static ColorMap colorMap;
  if (!colorMap.empty()) {
    return colorMap;
  }

  // the name/color pairs were generated using the X11 rgb.txt file

#undef  ADD_COLOR
#define ADD_COLOR(n, r, g, b) \
  colorMap[TextUtils::tolower(n)] = ColorData((r), (g), (b))

  ADD_COLOR("snow",                   1.000000f, 0.980392f, 0.980392f);
  ADD_COLOR("ghost_white",            0.972549f, 0.972549f, 1.000000f);
  ADD_COLOR("GhostWhite",             0.972549f, 0.972549f, 1.000000f);
  ADD_COLOR("white_smoke",            0.960784f, 0.960784f, 0.960784f);
  ADD_COLOR("WhiteSmoke",             0.960784f, 0.960784f, 0.960784f);
  ADD_COLOR("gainsboro",              0.862745f, 0.862745f, 0.862745f);
  ADD_COLOR("floral_white",           1.000000f, 0.980392f, 0.941176f);
  ADD_COLOR("FloralWhite",            1.000000f, 0.980392f, 0.941176f);
  ADD_COLOR("old_lace",               0.992157f, 0.960784f, 0.901961f);
  ADD_COLOR("OldLace",                0.992157f, 0.960784f, 0.901961f);
  ADD_COLOR("linen",                  0.980392f, 0.941176f, 0.901961f);
  ADD_COLOR("antique_white",          0.980392f, 0.921569f, 0.843137f);
  ADD_COLOR("AntiqueWhite",           0.980392f, 0.921569f, 0.843137f);
  ADD_COLOR("papaya_whip",            1.000000f, 0.937255f, 0.835294f);
  ADD_COLOR("PapayaWhip",             1.000000f, 0.937255f, 0.835294f);
  ADD_COLOR("blanched_almond",        1.000000f, 0.921569f, 0.803922f);
  ADD_COLOR("BlanchedAlmond",         1.000000f, 0.921569f, 0.803922f);
  ADD_COLOR("bisque",                 1.000000f, 0.894118f, 0.768627f);
  ADD_COLOR("peach_puff",             1.000000f, 0.854902f, 0.725490f);
  ADD_COLOR("PeachPuff",              1.000000f, 0.854902f, 0.725490f);
  ADD_COLOR("navajo_white",           1.000000f, 0.870588f, 0.678431f);
  ADD_COLOR("NavajoWhite",            1.000000f, 0.870588f, 0.678431f);
  ADD_COLOR("moccasin",               1.000000f, 0.894118f, 0.709804f);
  ADD_COLOR("cornsilk",               1.000000f, 0.972549f, 0.862745f);
  ADD_COLOR("ivory",                  1.000000f, 1.000000f, 0.941176f);
  ADD_COLOR("lemon_chiffon",          1.000000f, 0.980392f, 0.803922f);
  ADD_COLOR("LemonChiffon",           1.000000f, 0.980392f, 0.803922f);
  ADD_COLOR("seashell",               1.000000f, 0.960784f, 0.933333f);
  ADD_COLOR("honeydew",               0.941176f, 1.000000f, 0.941176f);
  ADD_COLOR("mint_cream",             0.960784f, 1.000000f, 0.980392f);
  ADD_COLOR("MintCream",              0.960784f, 1.000000f, 0.980392f);
  ADD_COLOR("azure",                  0.941176f, 1.000000f, 1.000000f);
  ADD_COLOR("alice_blue",             0.941176f, 0.972549f, 1.000000f);
  ADD_COLOR("AliceBlue",              0.941176f, 0.972549f, 1.000000f);
  ADD_COLOR("lavender",               0.901961f, 0.901961f, 0.980392f);
  ADD_COLOR("lavender_blush",         1.000000f, 0.941176f, 0.960784f);
  ADD_COLOR("LavenderBlush",          1.000000f, 0.941176f, 0.960784f);
  ADD_COLOR("misty_rose",             1.000000f, 0.894118f, 0.882353f);
  ADD_COLOR("MistyRose",              1.000000f, 0.894118f, 0.882353f);
  ADD_COLOR("white",                  1.000000f, 1.000000f, 1.000000f);
  ADD_COLOR("black",                  0.000000f, 0.000000f, 0.000000f);
  ADD_COLOR("dark_slate_gray",        0.184314f, 0.309804f, 0.309804f);
  ADD_COLOR("DarkSlateGray",          0.184314f, 0.309804f, 0.309804f);
  ADD_COLOR("dark_slate_grey",        0.184314f, 0.309804f, 0.309804f);
  ADD_COLOR("DarkSlateGrey",          0.184314f, 0.309804f, 0.309804f);
  ADD_COLOR("dim_gray",               0.411765f, 0.411765f, 0.411765f);
  ADD_COLOR("DimGray",                0.411765f, 0.411765f, 0.411765f);
  ADD_COLOR("dim_grey",               0.411765f, 0.411765f, 0.411765f);
  ADD_COLOR("DimGrey",                0.411765f, 0.411765f, 0.411765f);
  ADD_COLOR("slate_gray",             0.439216f, 0.501961f, 0.564706f);
  ADD_COLOR("SlateGray",              0.439216f, 0.501961f, 0.564706f);
  ADD_COLOR("slate_grey",             0.439216f, 0.501961f, 0.564706f);
  ADD_COLOR("SlateGrey",              0.439216f, 0.501961f, 0.564706f);
  ADD_COLOR("light_slate_gray",       0.466667f, 0.533333f, 0.600000f);
  ADD_COLOR("LightSlateGray",         0.466667f, 0.533333f, 0.600000f);
  ADD_COLOR("light_slate_grey",       0.466667f, 0.533333f, 0.600000f);
  ADD_COLOR("LightSlateGrey",         0.466667f, 0.533333f, 0.600000f);
  ADD_COLOR("gray",                   0.745098f, 0.745098f, 0.745098f);
  ADD_COLOR("grey",                   0.745098f, 0.745098f, 0.745098f);
  ADD_COLOR("light_grey",             0.827451f, 0.827451f, 0.827451f);
  ADD_COLOR("LightGrey",              0.827451f, 0.827451f, 0.827451f);
  ADD_COLOR("light_gray",             0.827451f, 0.827451f, 0.827451f);
  ADD_COLOR("LightGray",              0.827451f, 0.827451f, 0.827451f);
  ADD_COLOR("midnight_blue",          0.098039f, 0.098039f, 0.439216f);
  ADD_COLOR("MidnightBlue",           0.098039f, 0.098039f, 0.439216f);
  ADD_COLOR("navy",                   0.000000f, 0.000000f, 0.501961f);
  ADD_COLOR("navy_blue",              0.000000f, 0.000000f, 0.501961f);
  ADD_COLOR("NavyBlue",               0.000000f, 0.000000f, 0.501961f);
  ADD_COLOR("cornflower_blue",        0.392157f, 0.584314f, 0.929412f);
  ADD_COLOR("CornflowerBlue",         0.392157f, 0.584314f, 0.929412f);
  ADD_COLOR("dark_slate_blue",        0.282353f, 0.239216f, 0.545098f);
  ADD_COLOR("DarkSlateBlue",          0.282353f, 0.239216f, 0.545098f);
  ADD_COLOR("slate_blue",             0.415686f, 0.352941f, 0.803922f);
  ADD_COLOR("SlateBlue",              0.415686f, 0.352941f, 0.803922f);
  ADD_COLOR("medium_slate_blue",      0.482353f, 0.407843f, 0.933333f);
  ADD_COLOR("MediumSlateBlue",        0.482353f, 0.407843f, 0.933333f);
  ADD_COLOR("light_slate_blue",       0.517647f, 0.439216f, 1.000000f);
  ADD_COLOR("LightSlateBlue",         0.517647f, 0.439216f, 1.000000f);
  ADD_COLOR("medium_blue",            0.000000f, 0.000000f, 0.803922f);
  ADD_COLOR("MediumBlue",             0.000000f, 0.000000f, 0.803922f);
  ADD_COLOR("royal_blue",             0.254902f, 0.411765f, 0.882353f);
  ADD_COLOR("RoyalBlue",              0.254902f, 0.411765f, 0.882353f);
  ADD_COLOR("blue",                   0.000000f, 0.000000f, 1.000000f);
  ADD_COLOR("dodger_blue",            0.117647f, 0.564706f, 1.000000f);
  ADD_COLOR("DodgerBlue",             0.117647f, 0.564706f, 1.000000f);
  ADD_COLOR("deep_sky_blue",          0.000000f, 0.749020f, 1.000000f);
  ADD_COLOR("DeepSkyBlue",            0.000000f, 0.749020f, 1.000000f);
  ADD_COLOR("sky_blue",               0.529412f, 0.807843f, 0.921569f);
  ADD_COLOR("SkyBlue",                0.529412f, 0.807843f, 0.921569f);
  ADD_COLOR("light_sky_blue",         0.529412f, 0.807843f, 0.980392f);
  ADD_COLOR("LightSkyBlue",           0.529412f, 0.807843f, 0.980392f);
  ADD_COLOR("steel_blue",             0.274510f, 0.509804f, 0.705882f);
  ADD_COLOR("SteelBlue",              0.274510f, 0.509804f, 0.705882f);
  ADD_COLOR("light_steel_blue",       0.690196f, 0.768627f, 0.870588f);
  ADD_COLOR("LightSteelBlue",         0.690196f, 0.768627f, 0.870588f);
  ADD_COLOR("light_blue",             0.678431f, 0.847059f, 0.901961f);
  ADD_COLOR("LightBlue",              0.678431f, 0.847059f, 0.901961f);
  ADD_COLOR("powder_blue",            0.690196f, 0.878431f, 0.901961f);
  ADD_COLOR("PowderBlue",             0.690196f, 0.878431f, 0.901961f);
  ADD_COLOR("pale_turquoise",         0.686275f, 0.933333f, 0.933333f);
  ADD_COLOR("PaleTurquoise",          0.686275f, 0.933333f, 0.933333f);
  ADD_COLOR("dark_turquoise",         0.000000f, 0.807843f, 0.819608f);
  ADD_COLOR("DarkTurquoise",          0.000000f, 0.807843f, 0.819608f);
  ADD_COLOR("medium_turquoise",       0.282353f, 0.819608f, 0.800000f);
  ADD_COLOR("MediumTurquoise",        0.282353f, 0.819608f, 0.800000f);
  ADD_COLOR("turquoise",              0.250980f, 0.878431f, 0.815686f);
  ADD_COLOR("cyan",                   0.000000f, 1.000000f, 1.000000f);
  ADD_COLOR("light_cyan",             0.878431f, 1.000000f, 1.000000f);
  ADD_COLOR("LightCyan",              0.878431f, 1.000000f, 1.000000f);
  ADD_COLOR("cadet_blue",             0.372549f, 0.619608f, 0.627451f);
  ADD_COLOR("CadetBlue",              0.372549f, 0.619608f, 0.627451f);
  ADD_COLOR("medium_aquamarine",      0.400000f, 0.803922f, 0.666667f);
  ADD_COLOR("MediumAquamarine",       0.400000f, 0.803922f, 0.666667f);
  ADD_COLOR("aquamarine",             0.498039f, 1.000000f, 0.831373f);
  ADD_COLOR("dark_green",             0.000000f, 0.392157f, 0.000000f);
  ADD_COLOR("DarkGreen",              0.000000f, 0.392157f, 0.000000f);
  ADD_COLOR("dark_olive_green",       0.333333f, 0.419608f, 0.184314f);
  ADD_COLOR("DarkOliveGreen",         0.333333f, 0.419608f, 0.184314f);
  ADD_COLOR("dark_sea_green",         0.560784f, 0.737255f, 0.560784f);
  ADD_COLOR("DarkSeaGreen",           0.560784f, 0.737255f, 0.560784f);
  ADD_COLOR("sea_green",              0.180392f, 0.545098f, 0.341176f);
  ADD_COLOR("SeaGreen",               0.180392f, 0.545098f, 0.341176f);
  ADD_COLOR("medium_sea_green",       0.235294f, 0.701961f, 0.443137f);
  ADD_COLOR("MediumSeaGreen",         0.235294f, 0.701961f, 0.443137f);
  ADD_COLOR("light_sea_green",        0.125490f, 0.698039f, 0.666667f);
  ADD_COLOR("LightSeaGreen",          0.125490f, 0.698039f, 0.666667f);
  ADD_COLOR("pale_green",             0.596078f, 0.984314f, 0.596078f);
  ADD_COLOR("PaleGreen",              0.596078f, 0.984314f, 0.596078f);
  ADD_COLOR("spring_green",           0.000000f, 1.000000f, 0.498039f);
  ADD_COLOR("SpringGreen",            0.000000f, 1.000000f, 0.498039f);
  ADD_COLOR("lawn_green",             0.486275f, 0.988235f, 0.000000f);
  ADD_COLOR("LawnGreen",              0.486275f, 0.988235f, 0.000000f);
  ADD_COLOR("green",                  0.000000f, 1.000000f, 0.000000f);
  ADD_COLOR("chartreuse",             0.498039f, 1.000000f, 0.000000f);
  ADD_COLOR("medium_spring_green",    0.000000f, 0.980392f, 0.603922f);
  ADD_COLOR("MediumSpringGreen",      0.000000f, 0.980392f, 0.603922f);
  ADD_COLOR("green_yellow",           0.678431f, 1.000000f, 0.184314f);
  ADD_COLOR("GreenYellow",            0.678431f, 1.000000f, 0.184314f);
  ADD_COLOR("lime_green",             0.196078f, 0.803922f, 0.196078f);
  ADD_COLOR("LimeGreen",              0.196078f, 0.803922f, 0.196078f);
  ADD_COLOR("yellow_green",           0.603922f, 0.803922f, 0.196078f);
  ADD_COLOR("YellowGreen",            0.603922f, 0.803922f, 0.196078f);
  ADD_COLOR("forest_green",           0.133333f, 0.545098f, 0.133333f);
  ADD_COLOR("ForestGreen",            0.133333f, 0.545098f, 0.133333f);
  ADD_COLOR("olive_drab",             0.419608f, 0.556863f, 0.137255f);
  ADD_COLOR("OliveDrab",              0.419608f, 0.556863f, 0.137255f);
  ADD_COLOR("dark_khaki",             0.741176f, 0.717647f, 0.419608f);
  ADD_COLOR("DarkKhaki",              0.741176f, 0.717647f, 0.419608f);
  ADD_COLOR("khaki",                  0.941176f, 0.901961f, 0.549020f);
  ADD_COLOR("pale_goldenrod",         0.933333f, 0.909804f, 0.666667f);
  ADD_COLOR("PaleGoldenrod",          0.933333f, 0.909804f, 0.666667f);
  ADD_COLOR("light_goldenrod_yellow", 0.980392f, 0.980392f, 0.823529f);
  ADD_COLOR("LightGoldenrodYellow",   0.980392f, 0.980392f, 0.823529f);
  ADD_COLOR("light_yellow",           1.000000f, 1.000000f, 0.878431f);
  ADD_COLOR("LightYellow",            1.000000f, 1.000000f, 0.878431f);
  ADD_COLOR("yellow",                 1.000000f, 1.000000f, 0.000000f);
  ADD_COLOR("gold",                   1.000000f, 0.843137f, 0.000000f);
  ADD_COLOR("light_goldenrod",        0.933333f, 0.866667f, 0.509804f);
  ADD_COLOR("LightGoldenrod",         0.933333f, 0.866667f, 0.509804f);
  ADD_COLOR("goldenrod",              0.854902f, 0.647059f, 0.125490f);
  ADD_COLOR("dark_goldenrod",         0.721569f, 0.525490f, 0.043137f);
  ADD_COLOR("DarkGoldenrod",          0.721569f, 0.525490f, 0.043137f);
  ADD_COLOR("rosy_brown",             0.737255f, 0.560784f, 0.560784f);
  ADD_COLOR("RosyBrown",              0.737255f, 0.560784f, 0.560784f);
  ADD_COLOR("indian_red",             0.803922f, 0.360784f, 0.360784f);
  ADD_COLOR("IndianRed",              0.803922f, 0.360784f, 0.360784f);
  ADD_COLOR("saddle_brown",           0.545098f, 0.270588f, 0.074510f);
  ADD_COLOR("SaddleBrown",            0.545098f, 0.270588f, 0.074510f);
  ADD_COLOR("sienna",                 0.627451f, 0.321569f, 0.176471f);
  ADD_COLOR("peru",                   0.803922f, 0.521569f, 0.247059f);
  ADD_COLOR("burlywood",              0.870588f, 0.721569f, 0.529412f);
  ADD_COLOR("beige",                  0.960784f, 0.960784f, 0.862745f);
  ADD_COLOR("wheat",                  0.960784f, 0.870588f, 0.701961f);
  ADD_COLOR("sandy_brown",            0.956863f, 0.643137f, 0.376471f);
  ADD_COLOR("SandyBrown",             0.956863f, 0.643137f, 0.376471f);
  ADD_COLOR("tan",                    0.823529f, 0.705882f, 0.549020f);
  ADD_COLOR("chocolate",              0.823529f, 0.411765f, 0.117647f);
  ADD_COLOR("firebrick",              0.698039f, 0.133333f, 0.133333f);
  ADD_COLOR("brown",                  0.647059f, 0.164706f, 0.164706f);
  ADD_COLOR("dark_salmon",            0.913725f, 0.588235f, 0.478431f);
  ADD_COLOR("DarkSalmon",             0.913725f, 0.588235f, 0.478431f);
  ADD_COLOR("salmon",                 0.980392f, 0.501961f, 0.447059f);
  ADD_COLOR("light_salmon",           1.000000f, 0.627451f, 0.478431f);
  ADD_COLOR("LightSalmon",            1.000000f, 0.627451f, 0.478431f);
  ADD_COLOR("orange",                 1.000000f, 0.647059f, 0.000000f);
  ADD_COLOR("dark_orange",            1.000000f, 0.549020f, 0.000000f);
  ADD_COLOR("DarkOrange",             1.000000f, 0.549020f, 0.000000f);
  ADD_COLOR("coral",                  1.000000f, 0.498039f, 0.313726f);
  ADD_COLOR("light_coral",            0.941176f, 0.501961f, 0.501961f);
  ADD_COLOR("LightCoral",             0.941176f, 0.501961f, 0.501961f);
  ADD_COLOR("tomato",                 1.000000f, 0.388235f, 0.278431f);
  ADD_COLOR("orange_red",             1.000000f, 0.270588f, 0.000000f);
  ADD_COLOR("OrangeRed",              1.000000f, 0.270588f, 0.000000f);
  ADD_COLOR("red",                    1.000000f, 0.000000f, 0.000000f);
  ADD_COLOR("hot_pink",               1.000000f, 0.411765f, 0.705882f);
  ADD_COLOR("HotPink",                1.000000f, 0.411765f, 0.705882f);
  ADD_COLOR("deep_pink",              1.000000f, 0.078431f, 0.576471f);
  ADD_COLOR("DeepPink",               1.000000f, 0.078431f, 0.576471f);
  ADD_COLOR("pink",                   1.000000f, 0.752941f, 0.796078f);
  ADD_COLOR("light_pink",             1.000000f, 0.713726f, 0.756863f);
  ADD_COLOR("LightPink",              1.000000f, 0.713726f, 0.756863f);
  ADD_COLOR("pale_violet_red",        0.858824f, 0.439216f, 0.576471f);
  ADD_COLOR("PaleVioletRed",          0.858824f, 0.439216f, 0.576471f);
  ADD_COLOR("maroon",                 0.690196f, 0.188235f, 0.376471f);
  ADD_COLOR("medium_violet_red",      0.780392f, 0.082353f, 0.521569f);
  ADD_COLOR("MediumVioletRed",        0.780392f, 0.082353f, 0.521569f);
  ADD_COLOR("violet_red",             0.815686f, 0.125490f, 0.564706f);
  ADD_COLOR("VioletRed",              0.815686f, 0.125490f, 0.564706f);
  ADD_COLOR("magenta",                1.000000f, 0.000000f, 1.000000f);
  ADD_COLOR("violet",                 0.933333f, 0.509804f, 0.933333f);
  ADD_COLOR("plum",                   0.866667f, 0.627451f, 0.866667f);
  ADD_COLOR("orchid",                 0.854902f, 0.439216f, 0.839216f);
  ADD_COLOR("medium_orchid",          0.729412f, 0.333333f, 0.827451f);
  ADD_COLOR("MediumOrchid",           0.729412f, 0.333333f, 0.827451f);
  ADD_COLOR("dark_orchid",            0.600000f, 0.196078f, 0.800000f);
  ADD_COLOR("DarkOrchid",             0.600000f, 0.196078f, 0.800000f);
  ADD_COLOR("dark_violet",            0.580392f, 0.000000f, 0.827451f);
  ADD_COLOR("DarkViolet",             0.580392f, 0.000000f, 0.827451f);
  ADD_COLOR("blue_violet",            0.541176f, 0.168627f, 0.886275f);
  ADD_COLOR("BlueViolet",             0.541176f, 0.168627f, 0.886275f);
  ADD_COLOR("purple",                 0.627451f, 0.125490f, 0.941176f);
  ADD_COLOR("medium_purple",          0.576471f, 0.439216f, 0.858824f);
  ADD_COLOR("MediumPurple",           0.576471f, 0.439216f, 0.858824f);
  ADD_COLOR("thistle",                0.847059f, 0.749020f, 0.847059f);
  ADD_COLOR("snow1",                  1.000000f, 0.980392f, 0.980392f);
  ADD_COLOR("snow2",                  0.933333f, 0.913725f, 0.913725f);
  ADD_COLOR("snow3",                  0.803922f, 0.788235f, 0.788235f);
  ADD_COLOR("snow4",                  0.545098f, 0.537255f, 0.537255f);
  ADD_COLOR("seashell1",              1.000000f, 0.960784f, 0.933333f);
  ADD_COLOR("seashell2",              0.933333f, 0.898039f, 0.870588f);
  ADD_COLOR("seashell3",              0.803922f, 0.772549f, 0.749020f);
  ADD_COLOR("seashell4",              0.545098f, 0.525490f, 0.509804f);
  ADD_COLOR("AntiqueWhite1",          1.000000f, 0.937255f, 0.858824f);
  ADD_COLOR("AntiqueWhite2",          0.933333f, 0.874510f, 0.800000f);
  ADD_COLOR("AntiqueWhite3",          0.803922f, 0.752941f, 0.690196f);
  ADD_COLOR("AntiqueWhite4",          0.545098f, 0.513726f, 0.470588f);
  ADD_COLOR("bisque1",                1.000000f, 0.894118f, 0.768627f);
  ADD_COLOR("bisque2",                0.933333f, 0.835294f, 0.717647f);
  ADD_COLOR("bisque3",                0.803922f, 0.717647f, 0.619608f);
  ADD_COLOR("bisque4",                0.545098f, 0.490196f, 0.419608f);
  ADD_COLOR("PeachPuff1",             1.000000f, 0.854902f, 0.725490f);
  ADD_COLOR("PeachPuff2",             0.933333f, 0.796078f, 0.678431f);
  ADD_COLOR("PeachPuff3",             0.803922f, 0.686275f, 0.584314f);
  ADD_COLOR("PeachPuff4",             0.545098f, 0.466667f, 0.396078f);
  ADD_COLOR("NavajoWhite1",           1.000000f, 0.870588f, 0.678431f);
  ADD_COLOR("NavajoWhite2",           0.933333f, 0.811765f, 0.631373f);
  ADD_COLOR("NavajoWhite3",           0.803922f, 0.701961f, 0.545098f);
  ADD_COLOR("NavajoWhite4",           0.545098f, 0.474510f, 0.368627f);
  ADD_COLOR("LemonChiffon1",          1.000000f, 0.980392f, 0.803922f);
  ADD_COLOR("LemonChiffon2",          0.933333f, 0.913725f, 0.749020f);
  ADD_COLOR("LemonChiffon3",          0.803922f, 0.788235f, 0.647059f);
  ADD_COLOR("LemonChiffon4",          0.545098f, 0.537255f, 0.439216f);
  ADD_COLOR("cornsilk1",              1.000000f, 0.972549f, 0.862745f);
  ADD_COLOR("cornsilk2",              0.933333f, 0.909804f, 0.803922f);
  ADD_COLOR("cornsilk3",              0.803922f, 0.784314f, 0.694118f);
  ADD_COLOR("cornsilk4",              0.545098f, 0.533333f, 0.470588f);
  ADD_COLOR("ivory1",                 1.000000f, 1.000000f, 0.941176f);
  ADD_COLOR("ivory2",                 0.933333f, 0.933333f, 0.878431f);
  ADD_COLOR("ivory3",                 0.803922f, 0.803922f, 0.756863f);
  ADD_COLOR("ivory4",                 0.545098f, 0.545098f, 0.513726f);
  ADD_COLOR("honeydew1",              0.941176f, 1.000000f, 0.941176f);
  ADD_COLOR("honeydew2",              0.878431f, 0.933333f, 0.878431f);
  ADD_COLOR("honeydew3",              0.756863f, 0.803922f, 0.756863f);
  ADD_COLOR("honeydew4",              0.513726f, 0.545098f, 0.513726f);
  ADD_COLOR("LavenderBlush1",         1.000000f, 0.941176f, 0.960784f);
  ADD_COLOR("LavenderBlush2",         0.933333f, 0.878431f, 0.898039f);
  ADD_COLOR("LavenderBlush3",         0.803922f, 0.756863f, 0.772549f);
  ADD_COLOR("LavenderBlush4",         0.545098f, 0.513726f, 0.525490f);
  ADD_COLOR("MistyRose1",             1.000000f, 0.894118f, 0.882353f);
  ADD_COLOR("MistyRose2",             0.933333f, 0.835294f, 0.823529f);
  ADD_COLOR("MistyRose3",             0.803922f, 0.717647f, 0.709804f);
  ADD_COLOR("MistyRose4",             0.545098f, 0.490196f, 0.482353f);
  ADD_COLOR("azure1",                 0.941176f, 1.000000f, 1.000000f);
  ADD_COLOR("azure2",                 0.878431f, 0.933333f, 0.933333f);
  ADD_COLOR("azure3",                 0.756863f, 0.803922f, 0.803922f);
  ADD_COLOR("azure4",                 0.513726f, 0.545098f, 0.545098f);
  ADD_COLOR("SlateBlue1",             0.513726f, 0.435294f, 1.000000f);
  ADD_COLOR("SlateBlue2",             0.478431f, 0.403922f, 0.933333f);
  ADD_COLOR("SlateBlue3",             0.411765f, 0.349020f, 0.803922f);
  ADD_COLOR("SlateBlue4",             0.278431f, 0.235294f, 0.545098f);
  ADD_COLOR("RoyalBlue1",             0.282353f, 0.462745f, 1.000000f);
  ADD_COLOR("RoyalBlue2",             0.262745f, 0.431373f, 0.933333f);
  ADD_COLOR("RoyalBlue3",             0.227451f, 0.372549f, 0.803922f);
  ADD_COLOR("RoyalBlue4",             0.152941f, 0.250980f, 0.545098f);
  ADD_COLOR("blue1",                  0.000000f, 0.000000f, 1.000000f);
  ADD_COLOR("blue2",                  0.000000f, 0.000000f, 0.933333f);
  ADD_COLOR("blue3",                  0.000000f, 0.000000f, 0.803922f);
  ADD_COLOR("blue4",                  0.000000f, 0.000000f, 0.545098f);
  ADD_COLOR("DodgerBlue1",            0.117647f, 0.564706f, 1.000000f);
  ADD_COLOR("DodgerBlue2",            0.109804f, 0.525490f, 0.933333f);
  ADD_COLOR("DodgerBlue3",            0.094118f, 0.454902f, 0.803922f);
  ADD_COLOR("DodgerBlue4",            0.062745f, 0.305882f, 0.545098f);
  ADD_COLOR("SteelBlue1",             0.388235f, 0.721569f, 1.000000f);
  ADD_COLOR("SteelBlue2",             0.360784f, 0.674510f, 0.933333f);
  ADD_COLOR("SteelBlue3",             0.309804f, 0.580392f, 0.803922f);
  ADD_COLOR("SteelBlue4",             0.211765f, 0.392157f, 0.545098f);
  ADD_COLOR("DeepSkyBlue1",           0.000000f, 0.749020f, 1.000000f);
  ADD_COLOR("DeepSkyBlue2",           0.000000f, 0.698039f, 0.933333f);
  ADD_COLOR("DeepSkyBlue3",           0.000000f, 0.603922f, 0.803922f);
  ADD_COLOR("DeepSkyBlue4",           0.000000f, 0.407843f, 0.545098f);
  ADD_COLOR("SkyBlue1",               0.529412f, 0.807843f, 1.000000f);
  ADD_COLOR("SkyBlue2",               0.494118f, 0.752941f, 0.933333f);
  ADD_COLOR("SkyBlue3",               0.423529f, 0.650980f, 0.803922f);
  ADD_COLOR("SkyBlue4",               0.290196f, 0.439216f, 0.545098f);
  ADD_COLOR("LightSkyBlue1",          0.690196f, 0.886275f, 1.000000f);
  ADD_COLOR("LightSkyBlue2",          0.643137f, 0.827451f, 0.933333f);
  ADD_COLOR("LightSkyBlue3",          0.552941f, 0.713726f, 0.803922f);
  ADD_COLOR("LightSkyBlue4",          0.376471f, 0.482353f, 0.545098f);
  ADD_COLOR("SlateGray1",             0.776471f, 0.886275f, 1.000000f);
  ADD_COLOR("SlateGray2",             0.725490f, 0.827451f, 0.933333f);
  ADD_COLOR("SlateGray3",             0.623529f, 0.713726f, 0.803922f);
  ADD_COLOR("SlateGray4",             0.423529f, 0.482353f, 0.545098f);
  ADD_COLOR("LightSteelBlue1",        0.792157f, 0.882353f, 1.000000f);
  ADD_COLOR("LightSteelBlue2",        0.737255f, 0.823529f, 0.933333f);
  ADD_COLOR("LightSteelBlue3",        0.635294f, 0.709804f, 0.803922f);
  ADD_COLOR("LightSteelBlue4",        0.431373f, 0.482353f, 0.545098f);
  ADD_COLOR("LightBlue1",             0.749020f, 0.937255f, 1.000000f);
  ADD_COLOR("LightBlue2",             0.698039f, 0.874510f, 0.933333f);
  ADD_COLOR("LightBlue3",             0.603922f, 0.752941f, 0.803922f);
  ADD_COLOR("LightBlue4",             0.407843f, 0.513726f, 0.545098f);
  ADD_COLOR("LightCyan1",             0.878431f, 1.000000f, 1.000000f);
  ADD_COLOR("LightCyan2",             0.819608f, 0.933333f, 0.933333f);
  ADD_COLOR("LightCyan3",             0.705882f, 0.803922f, 0.803922f);
  ADD_COLOR("LightCyan4",             0.478431f, 0.545098f, 0.545098f);
  ADD_COLOR("PaleTurquoise1",         0.733333f, 1.000000f, 1.000000f);
  ADD_COLOR("PaleTurquoise2",         0.682353f, 0.933333f, 0.933333f);
  ADD_COLOR("PaleTurquoise3",         0.588235f, 0.803922f, 0.803922f);
  ADD_COLOR("PaleTurquoise4",         0.400000f, 0.545098f, 0.545098f);
  ADD_COLOR("CadetBlue1",             0.596078f, 0.960784f, 1.000000f);
  ADD_COLOR("CadetBlue2",             0.556863f, 0.898039f, 0.933333f);
  ADD_COLOR("CadetBlue3",             0.478431f, 0.772549f, 0.803922f);
  ADD_COLOR("CadetBlue4",             0.325490f, 0.525490f, 0.545098f);
  ADD_COLOR("turquoise1",             0.000000f, 0.960784f, 1.000000f);
  ADD_COLOR("turquoise2",             0.000000f, 0.898039f, 0.933333f);
  ADD_COLOR("turquoise3",             0.000000f, 0.772549f, 0.803922f);
  ADD_COLOR("turquoise4",             0.000000f, 0.525490f, 0.545098f);
  ADD_COLOR("cyan1",                  0.000000f, 1.000000f, 1.000000f);
  ADD_COLOR("cyan2",                  0.000000f, 0.933333f, 0.933333f);
  ADD_COLOR("cyan3",                  0.000000f, 0.803922f, 0.803922f);
  ADD_COLOR("cyan4",                  0.000000f, 0.545098f, 0.545098f);
  ADD_COLOR("DarkSlateGray1",         0.592157f, 1.000000f, 1.000000f);
  ADD_COLOR("DarkSlateGray2",         0.552941f, 0.933333f, 0.933333f);
  ADD_COLOR("DarkSlateGray3",         0.474510f, 0.803922f, 0.803922f);
  ADD_COLOR("DarkSlateGray4",         0.321569f, 0.545098f, 0.545098f);
  ADD_COLOR("aquamarine1",            0.498039f, 1.000000f, 0.831373f);
  ADD_COLOR("aquamarine2",            0.462745f, 0.933333f, 0.776471f);
  ADD_COLOR("aquamarine3",            0.400000f, 0.803922f, 0.666667f);
  ADD_COLOR("aquamarine4",            0.270588f, 0.545098f, 0.454902f);
  ADD_COLOR("DarkSeaGreen1",          0.756863f, 1.000000f, 0.756863f);
  ADD_COLOR("DarkSeaGreen2",          0.705882f, 0.933333f, 0.705882f);
  ADD_COLOR("DarkSeaGreen3",          0.607843f, 0.803922f, 0.607843f);
  ADD_COLOR("DarkSeaGreen4",          0.411765f, 0.545098f, 0.411765f);
  ADD_COLOR("SeaGreen1",              0.329412f, 1.000000f, 0.623529f);
  ADD_COLOR("SeaGreen2",              0.305882f, 0.933333f, 0.580392f);
  ADD_COLOR("SeaGreen3",              0.262745f, 0.803922f, 0.501961f);
  ADD_COLOR("SeaGreen4",              0.180392f, 0.545098f, 0.341176f);
  ADD_COLOR("PaleGreen1",             0.603922f, 1.000000f, 0.603922f);
  ADD_COLOR("PaleGreen2",             0.564706f, 0.933333f, 0.564706f);
  ADD_COLOR("PaleGreen3",             0.486275f, 0.803922f, 0.486275f);
  ADD_COLOR("PaleGreen4",             0.329412f, 0.545098f, 0.329412f);
  ADD_COLOR("SpringGreen1",           0.000000f, 1.000000f, 0.498039f);
  ADD_COLOR("SpringGreen2",           0.000000f, 0.933333f, 0.462745f);
  ADD_COLOR("SpringGreen3",           0.000000f, 0.803922f, 0.400000f);
  ADD_COLOR("SpringGreen4",           0.000000f, 0.545098f, 0.270588f);
  ADD_COLOR("green1",                 0.000000f, 1.000000f, 0.000000f);
  ADD_COLOR("green2",                 0.000000f, 0.933333f, 0.000000f);
  ADD_COLOR("green3",                 0.000000f, 0.803922f, 0.000000f);
  ADD_COLOR("green4",                 0.000000f, 0.545098f, 0.000000f);
  ADD_COLOR("chartreuse1",            0.498039f, 1.000000f, 0.000000f);
  ADD_COLOR("chartreuse2",            0.462745f, 0.933333f, 0.000000f);
  ADD_COLOR("chartreuse3",            0.400000f, 0.803922f, 0.000000f);
  ADD_COLOR("chartreuse4",            0.270588f, 0.545098f, 0.000000f);
  ADD_COLOR("OliveDrab1",             0.752941f, 1.000000f, 0.243137f);
  ADD_COLOR("OliveDrab2",             0.701961f, 0.933333f, 0.227451f);
  ADD_COLOR("OliveDrab3",             0.603922f, 0.803922f, 0.196078f);
  ADD_COLOR("OliveDrab4",             0.411765f, 0.545098f, 0.133333f);
  ADD_COLOR("DarkOliveGreen1",        0.792157f, 1.000000f, 0.439216f);
  ADD_COLOR("DarkOliveGreen2",        0.737255f, 0.933333f, 0.407843f);
  ADD_COLOR("DarkOliveGreen3",        0.635294f, 0.803922f, 0.352941f);
  ADD_COLOR("DarkOliveGreen4",        0.431373f, 0.545098f, 0.239216f);
  ADD_COLOR("khaki1",                 1.000000f, 0.964706f, 0.560784f);
  ADD_COLOR("khaki2",                 0.933333f, 0.901961f, 0.521569f);
  ADD_COLOR("khaki3",                 0.803922f, 0.776471f, 0.450980f);
  ADD_COLOR("khaki4",                 0.545098f, 0.525490f, 0.305882f);
  ADD_COLOR("LightGoldenrod1",        1.000000f, 0.925490f, 0.545098f);
  ADD_COLOR("LightGoldenrod2",        0.933333f, 0.862745f, 0.509804f);
  ADD_COLOR("LightGoldenrod3",        0.803922f, 0.745098f, 0.439216f);
  ADD_COLOR("LightGoldenrod4",        0.545098f, 0.505882f, 0.298039f);
  ADD_COLOR("LightYellow1",           1.000000f, 1.000000f, 0.878431f);
  ADD_COLOR("LightYellow2",           0.933333f, 0.933333f, 0.819608f);
  ADD_COLOR("LightYellow3",           0.803922f, 0.803922f, 0.705882f);
  ADD_COLOR("LightYellow4",           0.545098f, 0.545098f, 0.478431f);
  ADD_COLOR("yellow1",                1.000000f, 1.000000f, 0.000000f);
  ADD_COLOR("yellow2",                0.933333f, 0.933333f, 0.000000f);
  ADD_COLOR("yellow3",                0.803922f, 0.803922f, 0.000000f);
  ADD_COLOR("yellow4",                0.545098f, 0.545098f, 0.000000f);
  ADD_COLOR("gold1",                  1.000000f, 0.843137f, 0.000000f);
  ADD_COLOR("gold2",                  0.933333f, 0.788235f, 0.000000f);
  ADD_COLOR("gold3",                  0.803922f, 0.678431f, 0.000000f);
  ADD_COLOR("gold4",                  0.545098f, 0.458824f, 0.000000f);
  ADD_COLOR("goldenrod1",             1.000000f, 0.756863f, 0.145098f);
  ADD_COLOR("goldenrod2",             0.933333f, 0.705882f, 0.133333f);
  ADD_COLOR("goldenrod3",             0.803922f, 0.607843f, 0.113725f);
  ADD_COLOR("goldenrod4",             0.545098f, 0.411765f, 0.078431f);
  ADD_COLOR("DarkGoldenrod1",         1.000000f, 0.725490f, 0.058824f);
  ADD_COLOR("DarkGoldenrod2",         0.933333f, 0.678431f, 0.054902f);
  ADD_COLOR("DarkGoldenrod3",         0.803922f, 0.584314f, 0.047059f);
  ADD_COLOR("DarkGoldenrod4",         0.545098f, 0.396078f, 0.031373f);
  ADD_COLOR("RosyBrown1",             1.000000f, 0.756863f, 0.756863f);
  ADD_COLOR("RosyBrown2",             0.933333f, 0.705882f, 0.705882f);
  ADD_COLOR("RosyBrown3",             0.803922f, 0.607843f, 0.607843f);
  ADD_COLOR("RosyBrown4",             0.545098f, 0.411765f, 0.411765f);
  ADD_COLOR("IndianRed1",             1.000000f, 0.415686f, 0.415686f);
  ADD_COLOR("IndianRed2",             0.933333f, 0.388235f, 0.388235f);
  ADD_COLOR("IndianRed3",             0.803922f, 0.333333f, 0.333333f);
  ADD_COLOR("IndianRed4",             0.545098f, 0.227451f, 0.227451f);
  ADD_COLOR("sienna1",                1.000000f, 0.509804f, 0.278431f);
  ADD_COLOR("sienna2",                0.933333f, 0.474510f, 0.258824f);
  ADD_COLOR("sienna3",                0.803922f, 0.407843f, 0.223529f);
  ADD_COLOR("sienna4",                0.545098f, 0.278431f, 0.149020f);
  ADD_COLOR("burlywood1",             1.000000f, 0.827451f, 0.607843f);
  ADD_COLOR("burlywood2",             0.933333f, 0.772549f, 0.568627f);
  ADD_COLOR("burlywood3",             0.803922f, 0.666667f, 0.490196f);
  ADD_COLOR("burlywood4",             0.545098f, 0.450980f, 0.333333f);
  ADD_COLOR("wheat1",                 1.000000f, 0.905882f, 0.729412f);
  ADD_COLOR("wheat2",                 0.933333f, 0.847059f, 0.682353f);
  ADD_COLOR("wheat3",                 0.803922f, 0.729412f, 0.588235f);
  ADD_COLOR("wheat4",                 0.545098f, 0.494118f, 0.400000f);
  ADD_COLOR("tan1",                   1.000000f, 0.647059f, 0.309804f);
  ADD_COLOR("tan2",                   0.933333f, 0.603922f, 0.286275f);
  ADD_COLOR("tan3",                   0.803922f, 0.521569f, 0.247059f);
  ADD_COLOR("tan4",                   0.545098f, 0.352941f, 0.168627f);
  ADD_COLOR("chocolate1",             1.000000f, 0.498039f, 0.141176f);
  ADD_COLOR("chocolate2",             0.933333f, 0.462745f, 0.129412f);
  ADD_COLOR("chocolate3",             0.803922f, 0.400000f, 0.113725f);
  ADD_COLOR("chocolate4",             0.545098f, 0.270588f, 0.074510f);
  ADD_COLOR("firebrick1",             1.000000f, 0.188235f, 0.188235f);
  ADD_COLOR("firebrick2",             0.933333f, 0.172549f, 0.172549f);
  ADD_COLOR("firebrick3",             0.803922f, 0.149020f, 0.149020f);
  ADD_COLOR("firebrick4",             0.545098f, 0.101961f, 0.101961f);
  ADD_COLOR("brown1",                 1.000000f, 0.250980f, 0.250980f);
  ADD_COLOR("brown2",                 0.933333f, 0.231373f, 0.231373f);
  ADD_COLOR("brown3",                 0.803922f, 0.200000f, 0.200000f);
  ADD_COLOR("brown4",                 0.545098f, 0.137255f, 0.137255f);
  ADD_COLOR("salmon1",                1.000000f, 0.549020f, 0.411765f);
  ADD_COLOR("salmon2",                0.933333f, 0.509804f, 0.384314f);
  ADD_COLOR("salmon3",                0.803922f, 0.439216f, 0.329412f);
  ADD_COLOR("salmon4",                0.545098f, 0.298039f, 0.223529f);
  ADD_COLOR("LightSalmon1",           1.000000f, 0.627451f, 0.478431f);
  ADD_COLOR("LightSalmon2",           0.933333f, 0.584314f, 0.447059f);
  ADD_COLOR("LightSalmon3",           0.803922f, 0.505882f, 0.384314f);
  ADD_COLOR("LightSalmon4",           0.545098f, 0.341176f, 0.258824f);
  ADD_COLOR("orange1",                1.000000f, 0.647059f, 0.000000f);
  ADD_COLOR("orange2",                0.933333f, 0.603922f, 0.000000f);
  ADD_COLOR("orange3",                0.803922f, 0.521569f, 0.000000f);
  ADD_COLOR("orange4",                0.545098f, 0.352941f, 0.000000f);
  ADD_COLOR("DarkOrange1",            1.000000f, 0.498039f, 0.000000f);
  ADD_COLOR("DarkOrange2",            0.933333f, 0.462745f, 0.000000f);
  ADD_COLOR("DarkOrange3",            0.803922f, 0.400000f, 0.000000f);
  ADD_COLOR("DarkOrange4",            0.545098f, 0.270588f, 0.000000f);
  ADD_COLOR("coral1",                 1.000000f, 0.447059f, 0.337255f);
  ADD_COLOR("coral2",                 0.933333f, 0.415686f, 0.313726f);
  ADD_COLOR("coral3",                 0.803922f, 0.356863f, 0.270588f);
  ADD_COLOR("coral4",                 0.545098f, 0.243137f, 0.184314f);
  ADD_COLOR("tomato1",                1.000000f, 0.388235f, 0.278431f);
  ADD_COLOR("tomato2",                0.933333f, 0.360784f, 0.258824f);
  ADD_COLOR("tomato3",                0.803922f, 0.309804f, 0.223529f);
  ADD_COLOR("tomato4",                0.545098f, 0.211765f, 0.149020f);
  ADD_COLOR("OrangeRed1",             1.000000f, 0.270588f, 0.000000f);
  ADD_COLOR("OrangeRed2",             0.933333f, 0.250980f, 0.000000f);
  ADD_COLOR("OrangeRed3",             0.803922f, 0.215686f, 0.000000f);
  ADD_COLOR("OrangeRed4",             0.545098f, 0.145098f, 0.000000f);
  ADD_COLOR("red1",                   1.000000f, 0.000000f, 0.000000f);
  ADD_COLOR("red2",                   0.933333f, 0.000000f, 0.000000f);
  ADD_COLOR("red3",                   0.803922f, 0.000000f, 0.000000f);
  ADD_COLOR("red4",                   0.545098f, 0.000000f, 0.000000f);
  ADD_COLOR("DeepPink1",              1.000000f, 0.078431f, 0.576471f);
  ADD_COLOR("DeepPink2",              0.933333f, 0.070588f, 0.537255f);
  ADD_COLOR("DeepPink3",              0.803922f, 0.062745f, 0.462745f);
  ADD_COLOR("DeepPink4",              0.545098f, 0.039216f, 0.313726f);
  ADD_COLOR("HotPink1",               1.000000f, 0.431373f, 0.705882f);
  ADD_COLOR("HotPink2",               0.933333f, 0.415686f, 0.654902f);
  ADD_COLOR("HotPink3",               0.803922f, 0.376471f, 0.564706f);
  ADD_COLOR("HotPink4",               0.545098f, 0.227451f, 0.384314f);
  ADD_COLOR("pink1",                  1.000000f, 0.709804f, 0.772549f);
  ADD_COLOR("pink2",                  0.933333f, 0.662745f, 0.721569f);
  ADD_COLOR("pink3",                  0.803922f, 0.568627f, 0.619608f);
  ADD_COLOR("pink4",                  0.545098f, 0.388235f, 0.423529f);
  ADD_COLOR("LightPink1",             1.000000f, 0.682353f, 0.725490f);
  ADD_COLOR("LightPink2",             0.933333f, 0.635294f, 0.678431f);
  ADD_COLOR("LightPink3",             0.803922f, 0.549020f, 0.584314f);
  ADD_COLOR("LightPink4",             0.545098f, 0.372549f, 0.396078f);
  ADD_COLOR("PaleVioletRed1",         1.000000f, 0.509804f, 0.670588f);
  ADD_COLOR("PaleVioletRed2",         0.933333f, 0.474510f, 0.623529f);
  ADD_COLOR("PaleVioletRed3",         0.803922f, 0.407843f, 0.537255f);
  ADD_COLOR("PaleVioletRed4",         0.545098f, 0.278431f, 0.364706f);
  ADD_COLOR("maroon1",                1.000000f, 0.203922f, 0.701961f);
  ADD_COLOR("maroon2",                0.933333f, 0.188235f, 0.654902f);
  ADD_COLOR("maroon3",                0.803922f, 0.160784f, 0.564706f);
  ADD_COLOR("maroon4",                0.545098f, 0.109804f, 0.384314f);
  ADD_COLOR("VioletRed1",             1.000000f, 0.243137f, 0.588235f);
  ADD_COLOR("VioletRed2",             0.933333f, 0.227451f, 0.549020f);
  ADD_COLOR("VioletRed3",             0.803922f, 0.196078f, 0.470588f);
  ADD_COLOR("VioletRed4",             0.545098f, 0.133333f, 0.321569f);
  ADD_COLOR("magenta1",               1.000000f, 0.000000f, 1.000000f);
  ADD_COLOR("magenta2",               0.933333f, 0.000000f, 0.933333f);
  ADD_COLOR("magenta3",               0.803922f, 0.000000f, 0.803922f);
  ADD_COLOR("magenta4",               0.545098f, 0.000000f, 0.545098f);
  ADD_COLOR("orchid1",                1.000000f, 0.513726f, 0.980392f);
  ADD_COLOR("orchid2",                0.933333f, 0.478431f, 0.913725f);
  ADD_COLOR("orchid3",                0.803922f, 0.411765f, 0.788235f);
  ADD_COLOR("orchid4",                0.545098f, 0.278431f, 0.537255f);
  ADD_COLOR("plum1",                  1.000000f, 0.733333f, 1.000000f);
  ADD_COLOR("plum2",                  0.933333f, 0.682353f, 0.933333f);
  ADD_COLOR("plum3",                  0.803922f, 0.588235f, 0.803922f);
  ADD_COLOR("plum4",                  0.545098f, 0.400000f, 0.545098f);
  ADD_COLOR("MediumOrchid1",          0.878431f, 0.400000f, 1.000000f);
  ADD_COLOR("MediumOrchid2",          0.819608f, 0.372549f, 0.933333f);
  ADD_COLOR("MediumOrchid3",          0.705882f, 0.321569f, 0.803922f);
  ADD_COLOR("MediumOrchid4",          0.478431f, 0.215686f, 0.545098f);
  ADD_COLOR("DarkOrchid1",            0.749020f, 0.243137f, 1.000000f);
  ADD_COLOR("DarkOrchid2",            0.698039f, 0.227451f, 0.933333f);
  ADD_COLOR("DarkOrchid3",            0.603922f, 0.196078f, 0.803922f);
  ADD_COLOR("DarkOrchid4",            0.407843f, 0.133333f, 0.545098f);
  ADD_COLOR("purple1",                0.607843f, 0.188235f, 1.000000f);
  ADD_COLOR("purple2",                0.568627f, 0.172549f, 0.933333f);
  ADD_COLOR("purple3",                0.490196f, 0.149020f, 0.803922f);
  ADD_COLOR("purple4",                0.333333f, 0.101961f, 0.545098f);
  ADD_COLOR("MediumPurple1",          0.670588f, 0.509804f, 1.000000f);
  ADD_COLOR("MediumPurple2",          0.623529f, 0.474510f, 0.933333f);
  ADD_COLOR("MediumPurple3",          0.537255f, 0.407843f, 0.803922f);
  ADD_COLOR("MediumPurple4",          0.364706f, 0.278431f, 0.545098f);
  ADD_COLOR("thistle1",               1.000000f, 0.882353f, 1.000000f);
  ADD_COLOR("thistle2",               0.933333f, 0.823529f, 0.933333f);
  ADD_COLOR("thistle3",               0.803922f, 0.709804f, 0.803922f);
  ADD_COLOR("thistle4",               0.545098f, 0.482353f, 0.545098f);
  ADD_COLOR("gray0",                  0.000000f, 0.000000f, 0.000000f);
  ADD_COLOR("grey0",                  0.000000f, 0.000000f, 0.000000f);
  ADD_COLOR("gray1",                  0.011765f, 0.011765f, 0.011765f);
  ADD_COLOR("grey1",                  0.011765f, 0.011765f, 0.011765f);
  ADD_COLOR("gray2",                  0.019608f, 0.019608f, 0.019608f);
  ADD_COLOR("grey2",                  0.019608f, 0.019608f, 0.019608f);
  ADD_COLOR("gray3",                  0.031373f, 0.031373f, 0.031373f);
  ADD_COLOR("grey3",                  0.031373f, 0.031373f, 0.031373f);
  ADD_COLOR("gray4",                  0.039216f, 0.039216f, 0.039216f);
  ADD_COLOR("grey4",                  0.039216f, 0.039216f, 0.039216f);
  ADD_COLOR("gray5",                  0.050980f, 0.050980f, 0.050980f);
  ADD_COLOR("grey5",                  0.050980f, 0.050980f, 0.050980f);
  ADD_COLOR("gray6",                  0.058824f, 0.058824f, 0.058824f);
  ADD_COLOR("grey6",                  0.058824f, 0.058824f, 0.058824f);
  ADD_COLOR("gray7",                  0.070588f, 0.070588f, 0.070588f);
  ADD_COLOR("grey7",                  0.070588f, 0.070588f, 0.070588f);
  ADD_COLOR("gray8",                  0.078431f, 0.078431f, 0.078431f);
  ADD_COLOR("grey8",                  0.078431f, 0.078431f, 0.078431f);
  ADD_COLOR("gray9",                  0.090196f, 0.090196f, 0.090196f);
  ADD_COLOR("grey9",                  0.090196f, 0.090196f, 0.090196f);
  ADD_COLOR("gray10",                 0.101961f, 0.101961f, 0.101961f);
  ADD_COLOR("grey10",                 0.101961f, 0.101961f, 0.101961f);
  ADD_COLOR("gray11",                 0.109804f, 0.109804f, 0.109804f);
  ADD_COLOR("grey11",                 0.109804f, 0.109804f, 0.109804f);
  ADD_COLOR("gray12",                 0.121569f, 0.121569f, 0.121569f);
  ADD_COLOR("grey12",                 0.121569f, 0.121569f, 0.121569f);
  ADD_COLOR("gray13",                 0.129412f, 0.129412f, 0.129412f);
  ADD_COLOR("grey13",                 0.129412f, 0.129412f, 0.129412f);
  ADD_COLOR("gray14",                 0.141176f, 0.141176f, 0.141176f);
  ADD_COLOR("grey14",                 0.141176f, 0.141176f, 0.141176f);
  ADD_COLOR("gray15",                 0.149020f, 0.149020f, 0.149020f);
  ADD_COLOR("grey15",                 0.149020f, 0.149020f, 0.149020f);
  ADD_COLOR("gray16",                 0.160784f, 0.160784f, 0.160784f);
  ADD_COLOR("grey16",                 0.160784f, 0.160784f, 0.160784f);
  ADD_COLOR("gray17",                 0.168627f, 0.168627f, 0.168627f);
  ADD_COLOR("grey17",                 0.168627f, 0.168627f, 0.168627f);
  ADD_COLOR("gray18",                 0.180392f, 0.180392f, 0.180392f);
  ADD_COLOR("grey18",                 0.180392f, 0.180392f, 0.180392f);
  ADD_COLOR("gray19",                 0.188235f, 0.188235f, 0.188235f);
  ADD_COLOR("grey19",                 0.188235f, 0.188235f, 0.188235f);
  ADD_COLOR("gray20",                 0.200000f, 0.200000f, 0.200000f);
  ADD_COLOR("grey20",                 0.200000f, 0.200000f, 0.200000f);
  ADD_COLOR("gray21",                 0.211765f, 0.211765f, 0.211765f);
  ADD_COLOR("grey21",                 0.211765f, 0.211765f, 0.211765f);
  ADD_COLOR("gray22",                 0.219608f, 0.219608f, 0.219608f);
  ADD_COLOR("grey22",                 0.219608f, 0.219608f, 0.219608f);
  ADD_COLOR("gray23",                 0.231373f, 0.231373f, 0.231373f);
  ADD_COLOR("grey23",                 0.231373f, 0.231373f, 0.231373f);
  ADD_COLOR("gray24",                 0.239216f, 0.239216f, 0.239216f);
  ADD_COLOR("grey24",                 0.239216f, 0.239216f, 0.239216f);
  ADD_COLOR("gray25",                 0.250980f, 0.250980f, 0.250980f);
  ADD_COLOR("grey25",                 0.250980f, 0.250980f, 0.250980f);
  ADD_COLOR("gray26",                 0.258824f, 0.258824f, 0.258824f);
  ADD_COLOR("grey26",                 0.258824f, 0.258824f, 0.258824f);
  ADD_COLOR("gray27",                 0.270588f, 0.270588f, 0.270588f);
  ADD_COLOR("grey27",                 0.270588f, 0.270588f, 0.270588f);
  ADD_COLOR("gray28",                 0.278431f, 0.278431f, 0.278431f);
  ADD_COLOR("grey28",                 0.278431f, 0.278431f, 0.278431f);
  ADD_COLOR("gray29",                 0.290196f, 0.290196f, 0.290196f);
  ADD_COLOR("grey29",                 0.290196f, 0.290196f, 0.290196f);
  ADD_COLOR("gray30",                 0.301961f, 0.301961f, 0.301961f);
  ADD_COLOR("grey30",                 0.301961f, 0.301961f, 0.301961f);
  ADD_COLOR("gray31",                 0.309804f, 0.309804f, 0.309804f);
  ADD_COLOR("grey31",                 0.309804f, 0.309804f, 0.309804f);
  ADD_COLOR("gray32",                 0.321569f, 0.321569f, 0.321569f);
  ADD_COLOR("grey32",                 0.321569f, 0.321569f, 0.321569f);
  ADD_COLOR("gray33",                 0.329412f, 0.329412f, 0.329412f);
  ADD_COLOR("grey33",                 0.329412f, 0.329412f, 0.329412f);
  ADD_COLOR("gray34",                 0.341176f, 0.341176f, 0.341176f);
  ADD_COLOR("grey34",                 0.341176f, 0.341176f, 0.341176f);
  ADD_COLOR("gray35",                 0.349020f, 0.349020f, 0.349020f);
  ADD_COLOR("grey35",                 0.349020f, 0.349020f, 0.349020f);
  ADD_COLOR("gray36",                 0.360784f, 0.360784f, 0.360784f);
  ADD_COLOR("grey36",                 0.360784f, 0.360784f, 0.360784f);
  ADD_COLOR("gray37",                 0.368627f, 0.368627f, 0.368627f);
  ADD_COLOR("grey37",                 0.368627f, 0.368627f, 0.368627f);
  ADD_COLOR("gray38",                 0.380392f, 0.380392f, 0.380392f);
  ADD_COLOR("grey38",                 0.380392f, 0.380392f, 0.380392f);
  ADD_COLOR("gray39",                 0.388235f, 0.388235f, 0.388235f);
  ADD_COLOR("grey39",                 0.388235f, 0.388235f, 0.388235f);
  ADD_COLOR("gray40",                 0.400000f, 0.400000f, 0.400000f);
  ADD_COLOR("grey40",                 0.400000f, 0.400000f, 0.400000f);
  ADD_COLOR("gray41",                 0.411765f, 0.411765f, 0.411765f);
  ADD_COLOR("grey41",                 0.411765f, 0.411765f, 0.411765f);
  ADD_COLOR("gray42",                 0.419608f, 0.419608f, 0.419608f);
  ADD_COLOR("grey42",                 0.419608f, 0.419608f, 0.419608f);
  ADD_COLOR("gray43",                 0.431373f, 0.431373f, 0.431373f);
  ADD_COLOR("grey43",                 0.431373f, 0.431373f, 0.431373f);
  ADD_COLOR("gray44",                 0.439216f, 0.439216f, 0.439216f);
  ADD_COLOR("grey44",                 0.439216f, 0.439216f, 0.439216f);
  ADD_COLOR("gray45",                 0.450980f, 0.450980f, 0.450980f);
  ADD_COLOR("grey45",                 0.450980f, 0.450980f, 0.450980f);
  ADD_COLOR("gray46",                 0.458824f, 0.458824f, 0.458824f);
  ADD_COLOR("grey46",                 0.458824f, 0.458824f, 0.458824f);
  ADD_COLOR("gray47",                 0.470588f, 0.470588f, 0.470588f);
  ADD_COLOR("grey47",                 0.470588f, 0.470588f, 0.470588f);
  ADD_COLOR("gray48",                 0.478431f, 0.478431f, 0.478431f);
  ADD_COLOR("grey48",                 0.478431f, 0.478431f, 0.478431f);
  ADD_COLOR("gray49",                 0.490196f, 0.490196f, 0.490196f);
  ADD_COLOR("grey49",                 0.490196f, 0.490196f, 0.490196f);
  ADD_COLOR("gray50",                 0.498039f, 0.498039f, 0.498039f);
  ADD_COLOR("grey50",                 0.498039f, 0.498039f, 0.498039f);
  ADD_COLOR("gray51",                 0.509804f, 0.509804f, 0.509804f);
  ADD_COLOR("grey51",                 0.509804f, 0.509804f, 0.509804f);
  ADD_COLOR("gray52",                 0.521569f, 0.521569f, 0.521569f);
  ADD_COLOR("grey52",                 0.521569f, 0.521569f, 0.521569f);
  ADD_COLOR("gray53",                 0.529412f, 0.529412f, 0.529412f);
  ADD_COLOR("grey53",                 0.529412f, 0.529412f, 0.529412f);
  ADD_COLOR("gray54",                 0.541176f, 0.541176f, 0.541176f);
  ADD_COLOR("grey54",                 0.541176f, 0.541176f, 0.541176f);
  ADD_COLOR("gray55",                 0.549020f, 0.549020f, 0.549020f);
  ADD_COLOR("grey55",                 0.549020f, 0.549020f, 0.549020f);
  ADD_COLOR("gray56",                 0.560784f, 0.560784f, 0.560784f);
  ADD_COLOR("grey56",                 0.560784f, 0.560784f, 0.560784f);
  ADD_COLOR("gray57",                 0.568627f, 0.568627f, 0.568627f);
  ADD_COLOR("grey57",                 0.568627f, 0.568627f, 0.568627f);
  ADD_COLOR("gray58",                 0.580392f, 0.580392f, 0.580392f);
  ADD_COLOR("grey58",                 0.580392f, 0.580392f, 0.580392f);
  ADD_COLOR("gray59",                 0.588235f, 0.588235f, 0.588235f);
  ADD_COLOR("grey59",                 0.588235f, 0.588235f, 0.588235f);
  ADD_COLOR("gray60",                 0.600000f, 0.600000f, 0.600000f);
  ADD_COLOR("grey60",                 0.600000f, 0.600000f, 0.600000f);
  ADD_COLOR("gray61",                 0.611765f, 0.611765f, 0.611765f);
  ADD_COLOR("grey61",                 0.611765f, 0.611765f, 0.611765f);
  ADD_COLOR("gray62",                 0.619608f, 0.619608f, 0.619608f);
  ADD_COLOR("grey62",                 0.619608f, 0.619608f, 0.619608f);
  ADD_COLOR("gray63",                 0.631373f, 0.631373f, 0.631373f);
  ADD_COLOR("grey63",                 0.631373f, 0.631373f, 0.631373f);
  ADD_COLOR("gray64",                 0.639216f, 0.639216f, 0.639216f);
  ADD_COLOR("grey64",                 0.639216f, 0.639216f, 0.639216f);
  ADD_COLOR("gray65",                 0.650980f, 0.650980f, 0.650980f);
  ADD_COLOR("grey65",                 0.650980f, 0.650980f, 0.650980f);
  ADD_COLOR("gray66",                 0.658824f, 0.658824f, 0.658824f);
  ADD_COLOR("grey66",                 0.658824f, 0.658824f, 0.658824f);
  ADD_COLOR("gray67",                 0.670588f, 0.670588f, 0.670588f);
  ADD_COLOR("grey67",                 0.670588f, 0.670588f, 0.670588f);
  ADD_COLOR("gray68",                 0.678431f, 0.678431f, 0.678431f);
  ADD_COLOR("grey68",                 0.678431f, 0.678431f, 0.678431f);
  ADD_COLOR("gray69",                 0.690196f, 0.690196f, 0.690196f);
  ADD_COLOR("grey69",                 0.690196f, 0.690196f, 0.690196f);
  ADD_COLOR("gray70",                 0.701961f, 0.701961f, 0.701961f);
  ADD_COLOR("grey70",                 0.701961f, 0.701961f, 0.701961f);
  ADD_COLOR("gray71",                 0.709804f, 0.709804f, 0.709804f);
  ADD_COLOR("grey71",                 0.709804f, 0.709804f, 0.709804f);
  ADD_COLOR("gray72",                 0.721569f, 0.721569f, 0.721569f);
  ADD_COLOR("grey72",                 0.721569f, 0.721569f, 0.721569f);
  ADD_COLOR("gray73",                 0.729412f, 0.729412f, 0.729412f);
  ADD_COLOR("grey73",                 0.729412f, 0.729412f, 0.729412f);
  ADD_COLOR("gray74",                 0.741176f, 0.741176f, 0.741176f);
  ADD_COLOR("grey74",                 0.741176f, 0.741176f, 0.741176f);
  ADD_COLOR("gray75",                 0.749020f, 0.749020f, 0.749020f);
  ADD_COLOR("grey75",                 0.749020f, 0.749020f, 0.749020f);
  ADD_COLOR("gray76",                 0.760784f, 0.760784f, 0.760784f);
  ADD_COLOR("grey76",                 0.760784f, 0.760784f, 0.760784f);
  ADD_COLOR("gray77",                 0.768627f, 0.768627f, 0.768627f);
  ADD_COLOR("grey77",                 0.768627f, 0.768627f, 0.768627f);
  ADD_COLOR("gray78",                 0.780392f, 0.780392f, 0.780392f);
  ADD_COLOR("grey78",                 0.780392f, 0.780392f, 0.780392f);
  ADD_COLOR("gray79",                 0.788235f, 0.788235f, 0.788235f);
  ADD_COLOR("grey79",                 0.788235f, 0.788235f, 0.788235f);
  ADD_COLOR("gray80",                 0.800000f, 0.800000f, 0.800000f);
  ADD_COLOR("grey80",                 0.800000f, 0.800000f, 0.800000f);
  ADD_COLOR("gray81",                 0.811765f, 0.811765f, 0.811765f);
  ADD_COLOR("grey81",                 0.811765f, 0.811765f, 0.811765f);
  ADD_COLOR("gray82",                 0.819608f, 0.819608f, 0.819608f);
  ADD_COLOR("grey82",                 0.819608f, 0.819608f, 0.819608f);
  ADD_COLOR("gray83",                 0.831373f, 0.831373f, 0.831373f);
  ADD_COLOR("grey83",                 0.831373f, 0.831373f, 0.831373f);
  ADD_COLOR("gray84",                 0.839216f, 0.839216f, 0.839216f);
  ADD_COLOR("grey84",                 0.839216f, 0.839216f, 0.839216f);
  ADD_COLOR("gray85",                 0.850980f, 0.850980f, 0.850980f);
  ADD_COLOR("grey85",                 0.850980f, 0.850980f, 0.850980f);
  ADD_COLOR("gray86",                 0.858824f, 0.858824f, 0.858824f);
  ADD_COLOR("grey86",                 0.858824f, 0.858824f, 0.858824f);
  ADD_COLOR("gray87",                 0.870588f, 0.870588f, 0.870588f);
  ADD_COLOR("grey87",                 0.870588f, 0.870588f, 0.870588f);
  ADD_COLOR("gray88",                 0.878431f, 0.878431f, 0.878431f);
  ADD_COLOR("grey88",                 0.878431f, 0.878431f, 0.878431f);
  ADD_COLOR("gray89",                 0.890196f, 0.890196f, 0.890196f);
  ADD_COLOR("grey89",                 0.890196f, 0.890196f, 0.890196f);
  ADD_COLOR("gray90",                 0.898039f, 0.898039f, 0.898039f);
  ADD_COLOR("grey90",                 0.898039f, 0.898039f, 0.898039f);
  ADD_COLOR("gray91",                 0.909804f, 0.909804f, 0.909804f);
  ADD_COLOR("grey91",                 0.909804f, 0.909804f, 0.909804f);
  ADD_COLOR("gray92",                 0.921569f, 0.921569f, 0.921569f);
  ADD_COLOR("grey92",                 0.921569f, 0.921569f, 0.921569f);
  ADD_COLOR("gray93",                 0.929412f, 0.929412f, 0.929412f);
  ADD_COLOR("grey93",                 0.929412f, 0.929412f, 0.929412f);
  ADD_COLOR("gray94",                 0.941176f, 0.941176f, 0.941176f);
  ADD_COLOR("grey94",                 0.941176f, 0.941176f, 0.941176f);
  ADD_COLOR("gray95",                 0.949020f, 0.949020f, 0.949020f);
  ADD_COLOR("grey95",                 0.949020f, 0.949020f, 0.949020f);
  ADD_COLOR("gray96",                 0.960784f, 0.960784f, 0.960784f);
  ADD_COLOR("grey96",                 0.960784f, 0.960784f, 0.960784f);
  ADD_COLOR("gray97",                 0.968627f, 0.968627f, 0.968627f);
  ADD_COLOR("grey97",                 0.968627f, 0.968627f, 0.968627f);
  ADD_COLOR("gray98",                 0.980392f, 0.980392f, 0.980392f);
  ADD_COLOR("grey98",                 0.980392f, 0.980392f, 0.980392f);
  ADD_COLOR("gray99",                 0.988235f, 0.988235f, 0.988235f);
  ADD_COLOR("grey99",                 0.988235f, 0.988235f, 0.988235f);
  ADD_COLOR("gray100",                1.000000f, 1.000000f, 1.000000f);
  ADD_COLOR("grey100",                1.000000f, 1.000000f, 1.000000f);
  ADD_COLOR("dark_grey",              0.662745f, 0.662745f, 0.662745f);
  ADD_COLOR("DarkGrey",               0.662745f, 0.662745f, 0.662745f);
  ADD_COLOR("dark_gray",              0.662745f, 0.662745f, 0.662745f);
  ADD_COLOR("DarkGray",               0.662745f, 0.662745f, 0.662745f);
  ADD_COLOR("dark_blue",              0.000000f, 0.000000f, 0.545098f);
  ADD_COLOR("DarkBlue",               0.000000f, 0.000000f, 0.545098f);
  ADD_COLOR("dark_cyan",              0.000000f, 0.545098f, 0.545098f);
  ADD_COLOR("DarkCyan",               0.000000f, 0.545098f, 0.545098f);
  ADD_COLOR("dark_magenta",           0.545098f, 0.000000f, 0.545098f);
  ADD_COLOR("DarkMagenta",            0.545098f, 0.000000f, 0.545098f);
  ADD_COLOR("dark_red",               0.545098f, 0.000000f, 0.000000f);
  ADD_COLOR("DarkRed",                0.545098f, 0.000000f, 0.000000f);
  ADD_COLOR("light_green",            0.564706f, 0.933333f, 0.564706f);
  ADD_COLOR("LightGreen",             0.564706f, 0.933333f, 0.564706f);

  return colorMap;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
