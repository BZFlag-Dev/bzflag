/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <string>
#include <string.h>

#include "common.h"
#include "bzfgl.h"
#include "TextureManager.h"
#include "TextureFont.h"
#include "bzfio.h"

#include "OpenGLGState.h"

TextureFont::TextureFont()
{
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    listIDs[i] = INVALID_GL_LIST_ID;
    fontMetrics[i].charWidth = -1;
  }

  size = -1;
  textureID = -1;

  textureXSize = -1;
  textureYSize = -1;
  textureZStep = -1;
  numberOfCharacters = -1;
}

TextureFont::~TextureFont()
{
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    if (listIDs[i] != INVALID_GL_LIST_ID) {
      glDeleteLists(listIDs[i], 1);
      listIDs[i] = INVALID_GL_LIST_ID;
    }
  }
}

int TextureFont::getSize()
{
  return size;
}

const char* TextureFont::getFaceName()
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
    DEBUG2("Unexpected line in font metrics file %s, line %d (expected %s)\n",
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
      DEBUG2("Unexpected character: %s, in font metrics file %s, line %d (expected \"%c\").\n",
             tmpBuf.c_str()+expsize, file.getFileName().c_str(), line, expected);
      return false;
    }
  } else {
    DEBUG2("Unexpected line in font metrics file %s, line %d (expected %s)\n",
      file.getFileName().c_str(), line, expectedLeft.c_str());
    return false;
  }
}

bool TextureFont::load(OSFile &file)
{
  std::string extension = file.getExtension();

  if (extension=="")
    return false;

  texture = file.getFileName();
  std::string::size_type underscore = texture.rfind('_');
  if (underscore == std::string::npos) {
    DEBUG1("Unexpected font file name: %s, no _size found\n", file.getStdName().c_str());
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
    DEBUG1("Too many characters (%i) in %s.\n",
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
  }

  file.close();

  return (numberOfCharacters > 0);
}

void TextureFont::build(void)
{
  preLoadLists();
}

void TextureFont::preLoadLists(void)
{
  if (texture.size() < 1) {
    DEBUG2("Font %s does not have an associated texture name, not loading\n", texture.c_str());
    return;
  }

  // load up the texture
  TextureManager &tm = TextureManager::instance();
  std::string textureAndDir = "fonts/" + texture;
  textureID = tm.getTextureID(textureAndDir.c_str());

  DEBUG4("Font %s (face %s) has texture ID %d\n", texture.c_str(), faceName.c_str(), textureID);

  if (textureID == -1) {
    DEBUG2("Font texture %s has invalid ID\n", texture.c_str());
    return;
  }

  for (int i = 0; i < numberOfCharacters; i++) {
    if (listIDs[i] != INVALID_GL_LIST_ID) {
      glDeleteLists(listIDs[i], 1);
      listIDs[i] = INVALID_GL_LIST_ID; // make it a habit
    }
    listIDs[i] = glGenLists(1);
    glNewList(listIDs[i], GL_COMPILE);
    {
      glTranslatef((float)fontMetrics[i].initialDist, 0, 0);

      float fFontY = (float)fontMetrics[i].endY - fontMetrics[i].startY;
      float fFontX = (float)fontMetrics[i].endX - fontMetrics[i].startX;

      glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f((float)fontMetrics[i].startX / (float)textureXSize,
		     1.0f - (float)fontMetrics[i].startY / (float)textureYSize);
	glVertex3f(0.0f, fFontY, 0.0f);

	glTexCoord2f((float)fontMetrics[i].startX / (float)textureXSize,
		     1.0f - (float)fontMetrics[i].endY / (float)textureYSize);
	glVertex3f(0.0f, 0.0f, 0.0f);

	glTexCoord2f((float)fontMetrics[i].endX / (float)textureXSize,
		     1.0f - (float)fontMetrics[i].endY / (float)textureYSize);
	glVertex3f(fFontX, 0.0f, 0.0f);

	glTexCoord2f((float)fontMetrics[i].endX / (float)textureXSize,
		     1.0f - (float)fontMetrics[i].startY / (float)textureYSize);
	glVertex3f(fFontX, fFontY, 0.0f);
      glEnd();

      glTranslatef(fFontX, 0.0f, 0.0f);
    }
    glEndList();
  }

  // create GState
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(textureID);
  builder.setBlending();
  builder.setAlphaFunc();
  builder.enableTextureReplace(false);
  gstate = builder.getState();
}

float TextureFont::getStrLength(float scale, const char *str, int len)
{
  int charToUse = 0;
  int lastCharacter = 0;

  float totalLen = 0;

  for (int i = 0; i < len; i++) {
    lastCharacter = charToUse;
    if (str[i] < 32)
      charToUse = 32;
    else if (str[i] > numberOfCharacters + 32)
      charToUse = 32;
    else
      charToUse = str[i];

    charToUse -= 32;

    if (charToUse == 0) {
      totalLen += fontMetrics[charToUse].initialDist +
		  fontMetrics[charToUse].charWidth +
		  fontMetrics[charToUse].whiteSpaceDist;
    } else {
      totalLen += fontMetrics[charToUse].endX -
		  fontMetrics[charToUse].startX +
		  fontMetrics[charToUse].initialDist;
    }
  }

  return totalLen * scale;
}

void TextureFont::free(void)
{
  textureID = -1;
}

void TextureFont::drawString(float scale, GLfloat color[3], const char *str,
			     int len)
{
  if (!str)
    return;

  if (textureID == -1)
    preLoadLists();

  if (textureID == -1)
    return;

  gstate.setState();

  TextureManager &tm = TextureManager::instance();
  if (!tm.bind(textureID))
    return;

  if (color[0] >= 0)
    glColor3fv(color);

  glPushMatrix();
  glScalef(scale, scale, 1);

  glPushMatrix();
  int charToUse = 0;
  int lastCharacter = 0;
  for (int i = 0; i < len; i++) {
    lastCharacter = charToUse;
    const char space = ' '; // decimal 32
    if (str[i] < space) {
      charToUse = space;
    } else if (str[i] > (numberOfCharacters + space)) {
      charToUse = space;
    } else {
      charToUse = str[i];
    }

    charToUse -= space;

    if (charToUse == 0) {
      glTranslatef((float)fontMetrics[charToUse].initialDist +
		   (float)fontMetrics[charToUse].charWidth +
		   (float)fontMetrics[charToUse].whiteSpaceDist, 0.0f, 0.0f);
    } else {
      glCallList(listIDs[charToUse]);
    }
  }
  glPopMatrix();
  if (color[0] >= 0)
    glColor4f(1, 1, 1, 1);
  glPopMatrix();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
