/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ParseColor.h"

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <string>
#include <stdio.h>

typedef struct {
  const char* name;
  const float value[4];
} ColorSet;

static const ColorSet colorSets[] = {
  { "red",	{1.0f, 0.0f, 0.0f, 1.0f}},
  { "blue",	{0.0f, 0.0f, 1.0f, 1.0f}},
  { "green",	{0.0f, 1.0f, 0.0f, 1.0f}},
  { "yellow",	{1.0f, 1.0f, 0.0f, 1.0f}},
  { "purple",	{1.0f, 0.0f, 1.0f, 1.0f}},
  { "purple",	{0.0f, 1.0f, 1.0f, 1.0f}},
  { "black",	{0.0f, 0.0f, 0.0f, 1.0f}},
  { "white",	{1.0f, 1.0f, 1.0f, 1.0f}},
  { "grey",	{0.5f, 0.5f, 0.5f, 1.0f}},
};
static const int colorCount = sizeof(colorSets) / sizeof(ColorSet);


bool parseColorStream(std::istream& input, float color[4])
{
  std::string line;
  std::getline(input, line);
  input.putback('\n');
  
  return parseColorString(line.c_str(), color);
}


bool parseColorString(const char* str, float color[4])
{
  int i;
  const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  
  // set to opaque white
  memcpy (color, white, sizeof(float[4]));
  
  // strip leading space
  while ((*str != '\0') && isspace(*str)) {
    str++;
  }

  // no string  
  if (*str == '\0') {
    return false;
  }
  
  
  // #FF2343A9 format
  if (*str == '#') {
    // FIXME - complete this ?
    return false;
  }

  // numeric format (either 3 or 4 floating point values)
  else if (((*str >= '0') && (*str <= '9'))
           || (*str == '.')
           || (*str == '+')
           || (*str == '-')) {
    int count;
    float tmp[4];
    count = sscanf(str, "%f %f %f %f", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
    if (count < 3) {
      return false;
    } else {
      memcpy (color, tmp, count * sizeof(float));
      return true;
    }
  }

  // text string format  ("red 0.2" format is accepted for alpha values)
  else {
    for (i = 0; i < colorCount; i++) {
      int len = strlen (colorSets[i].name);
      const char* end = str + len;
      if ((strncasecmp (colorSets[i].name, str, len) == 0)
          && ((*end == '\0') || isspace(*end))) {
        memcpy (color, colorSets[i].value, sizeof(float[3]));

        int count;
        float alpha;
        count = sscanf (str + len, "%f", &alpha);
        if (count > 0) {
          color[3] = alpha;
        }
        
        return true;
      }
    }
  }  
  
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
