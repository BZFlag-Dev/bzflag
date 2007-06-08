/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// Interface header
#include "ImageFont.h"

// System implementation headers
#include <string.h>

// Common implementation headers
#include "bzfio.h"
#include "MediaFile.h"


ImageFont::ImageFont()
{
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    fontMetrics[i].charWidth = -1;
  }

  size = -1;

  textureXSize = -1;
  textureYSize = -1;
  textureZStep = -1;
  numberOfCharacters = -1;
}

ImageFont::~ImageFont()
{
}

int ImageFont::getSize() const
{
  return size;
}

const char* ImageFont::getFaceName() const
{
  return faceName.c_str();
}

// keep a line counter for debugging
static int line;

/* read values in Key: Value form from font metrics (.fmt) files */
bool readKeyInt(OSFile &file, std::string expectedLeft, int &retval, bool newfile=false)
{
  if (newfile)
    line = 0;

  const int expsize = int(expectedLeft.size());
  std::string tmpBuf;

  // allow for blank lines with native or foreign linebreaks, comment lines
  while (tmpBuf.size() == 0 || tmpBuf[0] == '#' || tmpBuf[0] == 10 || tmpBuf[0] == 13) {
    tmpBuf = file.readLine();
    line++;
  }

  if (tmpBuf.substr(0, expsize) == expectedLeft && tmpBuf[expsize]==':') {
    retval = atoi(tmpBuf.c_str()+expsize+1);
    return true;
  } else {
    logDebugMessage(2,"Unexpected line in font metrics file %s, line %d (expected %s)\n",
      file.getFileName().c_str(), line, expectedLeft.c_str());
    return false;
  }
}

// read Char: "x" entry
bool readLetter(OSFile &file, char expected)
{
  const std::string expectedLeft = "Char:";
  const int expsize = int(expectedLeft.size());
  std::string tmpBuf;

  // allow for blank lines with native or foreign linebreaks, comment lines
  while (tmpBuf.size() == 0 || tmpBuf[0] == '#' || tmpBuf[0] == 10 || tmpBuf[0] == 13) {
    tmpBuf = file.readLine();
    // keep a line counter
    line++;
  }

  if (tmpBuf.substr(0, expsize) == expectedLeft) {
    if (int(tmpBuf.size()) >= expsize+4 && tmpBuf[expsize+1]=='"' && tmpBuf[expsize+3]=='"' &&
	tmpBuf[expsize+2]==expected) {
      return true;
    } else {
      logDebugMessage(2,"Unexpected character: %s, in font metrics file %s, line %d (expected \"%c\").\n",
	     tmpBuf.c_str()+expsize, file.getFileName().c_str(), line, expected);
      return false;
    }
  } else {
    logDebugMessage(2,"Unexpected line in font metrics file %s, line %d (expected %s)\n",
      file.getFileName().c_str(), line, expectedLeft.c_str());
    return false;
  }
}

bool ImageFont::load(OSFile &file)
{
  std::string extension = file.getExtension();

  if (extension=="")
    return false;

  texture = file.getFileName();
  std::string::size_type underscore = texture.rfind('_');
  if (underscore == std::string::npos) {
    logDebugMessage(1,"Unexpected font file name: %s, no _size found\n", file.getStdName().c_str());
    return false;
  }
  faceName = texture.substr(0, underscore);
  size = atoi(texture.c_str() + underscore + 1);

  if (!file.open("rb"))
    return false;

  if (!readKeyInt(file, "NumChars", numberOfCharacters, true)) return false;
  if (!readKeyInt(file, "TextureWidth", textureXSize)) return false;
  if (!readKeyInt(file, "TextureHeight", textureYSize)) return false;
  if (!readKeyInt(file, "TextZStep", textureZStep)) return false;

  // clamp the maximum char count
  if (numberOfCharacters > MAX_TEXTURE_FONT_CHARS) {
    logDebugMessage(1,"Too many characters (%i) in %s.\n",
	   numberOfCharacters, file.getFileName().c_str());
    numberOfCharacters = MAX_TEXTURE_FONT_CHARS;
  }

  int i;
  for (i = 0; i < numberOfCharacters; i++) {
    // check character
    if (!readLetter(file, i + 32)) return false;

    // read metrics
    if (!readKeyInt(file, "InitialDist", fontMetrics[i].initialDist)) return false;
    if (!readKeyInt(file, "Width", fontMetrics[i].charWidth)) return false;
    if (!readKeyInt(file, "Whitespace", fontMetrics[i].whiteSpaceDist)) return false;
    if (!readKeyInt(file, "StartX", fontMetrics[i].startX)) return false;
    if (!readKeyInt(file, "EndX", fontMetrics[i].endX)) return false;
    if (!readKeyInt(file, "StartY", fontMetrics[i].startY)) return false;
    if (!readKeyInt(file, "EndY", fontMetrics[i].endY)) return false;
    fontMetrics[i].fullWidth = fontMetrics[i].initialDist +
			       fontMetrics[i].charWidth +
			       fontMetrics[i].whiteSpaceDist;
  }

  file.close();

  return (numberOfCharacters > 0);
}

float ImageFont::getStrLength(float scale, const char *str, int len) const
{
  int charToUse = 0;

  float totalLen = 0;

  for (int i = 0; i < len; i++) {
    if (str[i] < 32)
      charToUse = 32;
    else if (str[i] > numberOfCharacters + 32)
      charToUse = 32;
    else
      charToUse = str[i];

    charToUse -= 32;

    totalLen += (float)(fontMetrics[charToUse].fullWidth);
  }

  return totalLen * scale;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
