/* bzflag
* Copyright (c) 1993 - 2004 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <assert.h>
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
  for (int i = 0; i < 128; i++) {
    listIDs[i] = _GL_INVALID_ID;
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
  for (int i = 0; i < 128; i++) {
    if (listIDs[i] != _GL_INVALID_ID)
      glDeleteLists(listIDs[i], 1);
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

/* read values in Key: Value form from font metrics (.fmt) files */
bool TextureFont::fmtRead(OSFile &file, std::string expectedLeft, std::string &retval)
{
  static std::string workingFile;
  static int line = 0;

  // reset line number if we've switched files
  if (workingFile != file.getFileName()) {
    workingFile = file.getFileName();
    line = 0;
  }

  std::string tmpBuf;

  // allow for blank lines with native or foreign linebreaks, comment lines
  while (tmpBuf.size() == 0 || tmpBuf[0] == '#' || tmpBuf[0] == 10 || tmpBuf[0] == 13) {
    tmpBuf = file.readLine();
    // keep a line counter
    line++;
  }

  if (tmpBuf.substr(0, tmpBuf.find(":")) == expectedLeft) {
    retval = tmpBuf.substr(tmpBuf.find(":") + 1, tmpBuf.size());
    return true;
  } else {
    DEBUG2("Unexpected line in font metrics file %s, line %d (expected %s)\n",
      file.getFileName(), line, expectedLeft.c_str());
    return false;
  }
}

bool TextureFont::load(OSFile &file)
{
  const char *extension = file.getExtension();

  if (!extension)
    return false;

  if (!file.open("rb"))
    return false;

  std::string tmpBuf;

  if (!fmtRead(file, "NumChars", tmpBuf)) return false;
  sscanf(tmpBuf.c_str(), " %d", &numberOfCharacters);
  if (!fmtRead(file, "TextureWidth", tmpBuf)) return false;
  sscanf(tmpBuf.c_str(), " %d", &textureXSize);
  if (!fmtRead(file, "TextureHeight", tmpBuf)) return false;
  sscanf(tmpBuf.c_str(), " %d", &textureYSize);
  if (!fmtRead(file, "TextZStep", tmpBuf)) return false;
  sscanf(tmpBuf.c_str(), " %d", &textureZStep);

  int i;
  for (i = 0; i < numberOfCharacters; i++) {
    // check character
    if (!fmtRead(file, "Char", tmpBuf)) return false;
    if ((tmpBuf.size() < 3) || 
	(tmpBuf[1] != '\"' || tmpBuf[2] != (i + 32) || tmpBuf[3] != '\"')) {
      DEBUG2("Unexpected character: %s, in font metrics file %s (expected \"%c\").\n",
	tmpBuf.c_str(), file.getFileName(), (char)(i + 32));
      return false;
    }
    // read metrics
    if (!fmtRead(file, "InitialDist", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].initialDist);
    if (!fmtRead(file, "Width", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].charWidth);
    if (!fmtRead(file, "Whitespace", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].whiteSpaceDist);
    if (!fmtRead(file, "StartX", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].startX);
    if (!fmtRead(file, "EndX", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].endX);
    if (!fmtRead(file, "StartY", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].startY);
    if (!fmtRead(file, "EndY", tmpBuf)) return false;
    sscanf(tmpBuf.c_str(), " %d", &fontMetrics[i].endY);
  }

  file.close();

  // now compute the names
  std::string fullName = file.getStdName();
  char *temp;

  // get just the file part
  temp = strrchr(fullName.c_str(), '/');
  if (temp)
    faceName = temp + 1;
  else
    faceName = fullName;

  // now get the texture name
  texture = faceName;

  // now wack off the extension;
  if (extension)
    faceName.erase(faceName.size() - strlen(extension), faceName.size());

  temp = strrchr(faceName.c_str(), '_');

  if (temp) {
    size = atoi(temp+1);
    faceName.resize(temp - faceName.c_str());
  }

  // faceName.erase(faceName.size()-strlen(temp),faceName.size());

  if (extension)
    texture.erase(texture.size() - strlen(extension), texture.size());

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

  glPushMatrix();
  for (int i = 0; i < numberOfCharacters; i++) {
    if (listIDs[i] != _GL_INVALID_ID)
      glDeleteLists(listIDs[i], 1);
    listIDs[i] = glGenLists(1);
    glLoadIdentity();

    glNewList(listIDs[i], GL_COMPILE);

    glTranslatef((float)fontMetrics[i].initialDist, 0, 0);

    float fFontY = (float)fontMetrics[i].endY - fontMetrics[i].startY;
    float fFontX = (float)fontMetrics[i].endX - fontMetrics[i].startX;

    glBegin(GL_QUADS);
      glNormal3f(0, 0, 1);
      glTexCoord2f((float)fontMetrics[i].startX / (float)textureXSize, 1.0f - (float)fontMetrics[i].startY / (float)textureYSize);
      glVertex3f(0, fFontY, 0);

      glTexCoord2f((float)fontMetrics[i].startX / (float)textureXSize, 1.0f - (float)fontMetrics[i].endY / (float)textureYSize);
      glVertex3f(0, 0, 0);

      glTexCoord2f((float)fontMetrics[i].endX / (float)textureXSize, 1.0f - (float)fontMetrics[i].endY / (float)textureYSize);
      glVertex3f(fFontX, 0, 0);

      glTexCoord2f((float)fontMetrics[i].endX / (float)textureXSize, 1.0f - (float)fontMetrics[i].startY / (float)textureYSize);
      glVertex3f(fFontX, fFontY, 0);
    glEnd();

    glTranslatef(fFontX, 0, 0);

    glEndList();
  }
  glPopMatrix();

  // create GState
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(textureID);
  builder.setBlending();
  builder.setAlphaFunc();
  builder.enableTextureReplace(false);
  gstate = builder.getState();
}

float TextureFont::getStrLength(float scale, const char *str)
{
  int len = (int)strlen(str);
  int charToUse = 0;
  int lastCharacter = 0;

  float totalLen = 0;
  float thisPassLen = 0;

  for (int i = 0; i < len; i++) {
    if (str[i] == '\n') {	// newline, get back to the intial X and push down
      thisPassLen = 0;
    } else {
      lastCharacter = charToUse;
      if ((str[i] < 32) || (str[i] < 9))
	charToUse = 32;
      else if (str[i] > numberOfCharacters + 32)
	charToUse = 32;
      else
	charToUse = str[i];

      charToUse -= 32;

      if (charToUse == 0) {
	if (i == 0)
	  thisPassLen += fontMetrics[charToUse].initialDist + fontMetrics[charToUse].charWidth+fontMetrics[charToUse].whiteSpaceDist;
	else
	  thisPassLen += fontMetrics[lastCharacter].whiteSpaceDist + fontMetrics[charToUse].whiteSpaceDist+fontMetrics[charToUse].initialDist + fontMetrics[charToUse].charWidth;
      } else {
	float fFontX = (float)fontMetrics[charToUse].endX - fontMetrics[charToUse].startX;
	thisPassLen += fFontX + (float)fontMetrics[charToUse].initialDist;
      }
    }
    if (thisPassLen > totalLen)
      totalLen = thisPassLen;
  }

  return totalLen * scale;
}

void TextureFont::free(void)
{
  textureID = -1;
}

void TextureFont::drawString(float scale, GLfloat color[3], const char *str)
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
  int len = (int)strlen(str);
  int charToUse = 0;
  int lastCharacter = 0;
  for (int i = 0; i < len; i++) {
    if (str[i] == '\n') {	// newline, get back to the intial X and push down
      glPopMatrix();
      glTranslatef(0, -(float)textureZStep, 0);
      glPushMatrix();
    } else {
      lastCharacter = charToUse;
      if ((str[i] < 32) || (str[i] < 9))
	charToUse = 32;
      else if (str[i] > numberOfCharacters + 32)
	charToUse = 32;
      else
	charToUse = str[i];

      charToUse -= 32;

      if (charToUse == 0) {
	if (i == 0)
	  glTranslatef((float)fontMetrics[charToUse].initialDist + (float)fontMetrics[charToUse].charWidth + (float)fontMetrics[charToUse].whiteSpaceDist, 0, 0);
	else
	  glTranslatef((float)fontMetrics[lastCharacter].whiteSpaceDist + (float)fontMetrics[charToUse].whiteSpaceDist + fontMetrics[charToUse].initialDist + (float)fontMetrics[charToUse].charWidth, 0, 0);
      } else {
	glCallList(listIDs[charToUse]);
      }
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
