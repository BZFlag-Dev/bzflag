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

// BZFlag common header
#include "common.h"

// Interface header
#include "FontManager.h"

// System headers
#include <math.h>
#include <string>

// Global implementation headers
#include "bzfgl.h"
#include "bzfio.h"
#include "AnsiCodes.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLGState.h"
#include "TimeKeeper.h"
#include "TextUtils.h"

// Local implementation headers
#include "ImageFont.h"
#include "BitmapFont.h"
#include "TextureFont.h"

// initialize the singleton
template <>
FontManager* Singleton<FontManager>::_instance = (FontManager*)0;


// ANSI code GLFloat equivalents - these should line up with the enums in AnsiCodes.h
static GLfloat BrightColors[9][3] = {
  {1.0f,1.0f,0.0f}, // yellow
  {1.0f,0.0f,0.0f}, // red
  {0.0f,1.0f,0.0f}, // green
  {0.1f,0.2f,1.0f}, // blue
  {1.0f,0.0f,1.0f}, // purple
  {1.0f,1.0f,1.0f}, // white
  {0.5f,0.5f,0.5f}, // grey
  {1.0f,0.5f,0.0f}, // orange (nonstandard)
  {0.0f,1.0f,1.0f}  // cyan
};

GLfloat FontManager::underlineColor[4];
void FontManager::callback(const std::string &, void *)
{
  // set underline color
  const std::string uColor = BZDB.get("underlineColor");
  if (strcasecmp(uColor.c_str(), "text") == 0) {
    underlineColor[0] = -1.0f;
    underlineColor[1] = -1.0f;
    underlineColor[2] = -1.0f;
  } else if (strcasecmp(uColor.c_str(), "cyan") == 0) {
    underlineColor[0] = BrightColors[CyanColor][0];
    underlineColor[1] = BrightColors[CyanColor][1];
    underlineColor[2] = BrightColors[CyanColor][2];
  } else if (strcasecmp(uColor.c_str(), "grey") == 0) {
    underlineColor[0] = BrightColors[GreyColor][0];
    underlineColor[1] = BrightColors[GreyColor][1];
    underlineColor[2] = BrightColors[GreyColor][2];
  }
}

FontManager::FontManager() : Singleton<FontManager>(),
			     opacity(1.0f),
			     dimFactor(0.2f),
			     darkness(1.0f)
{
  faceNames.clear();
  fontFaces.clear();
  BZDB.addCallback(std::string("underlineColor"), callback, NULL);
  BZDB.touch("underlineColor");
  OpenGLGState::registerContextInitializer(freeContext, initContext,
					   (void*)this);
}

FontManager::~FontManager()
{
  clear();
  OpenGLGState::unregisterContextInitializer(freeContext, initContext,
					     (void*)this);
  return;
}


void FontManager::freeContext(void* data)
{
  ((FontManager*)data)->clear();
  return;
}


void FontManager::initContext(void* data)
{
  ((FontManager*)data)->rebuild();
  return;
}


void FontManager::clear(void)	// clear all the lists
{
  // destroy all the fonts
  faceNames.clear();
  FontFaceList::iterator faceItr = fontFaces.begin();
  while (faceItr != fontFaces.end()) {
    FontSizeMap::iterator itr = faceItr->begin();
    while (itr != faceItr->end()) {
      delete(itr->second);
      itr++;
    }
    faceItr++;
  }
  fontFaces.clear();
  return;
}


void FontManager::rebuild(void)	// rebuild all the lists
{
  clear();
  loadAll(fontDirectory);
}


void FontManager::loadAll(std::string directory)
{
  if (directory.size() == 0)
    return;

  const bool bitmapRenderer = BZDB.isTrue("useBitmapFontRenderer");
  canScale = !bitmapRenderer;

  // save this in case we have to rebuild
  fontDirectory = directory;

  OSFile file;

  OSDir dir(directory);

  while (dir.getNextFile(file, true)) {
    std::string ext = file.getExtension();

    if (TextUtils::compare_nocase(ext, "fmt") == 0) {
      ImageFont *pFont;
      if (bitmapRenderer)
	pFont = new BitmapFont;
      else
	pFont = new TextureFont;
      if (pFont) {
	if (pFont->load(file)) {
	  std::string  str = TextUtils::toupper(pFont->getFaceName());

	  FontFaceMap::iterator faceItr = faceNames.find(str);

	  int faceID = 0;
	  if (faceItr == faceNames.end()) {
	    // it's new
	    FontSizeMap faceList;
	    fontFaces.push_back(faceList);
	    faceID = (int)fontFaces.size() - 1;
	    faceNames[str] = faceID;
	  } else {
	    faceID = faceItr->second;
	  }

	  fontFaces[faceID][pFont->getSize()] = pFont;
	} else {
	  logDebugMessage(4,"Font Texture load failed: %s\n", file.getOSName().c_str());
	  delete(pFont);
	}
      }
    }
  }
}

int FontManager::getFaceID(std::string faceName)
{
  if (faceName.size() == 0)
    return -1;

  faceName = TextUtils::toupper(faceName);

  FontFaceMap::iterator faceItr = faceNames.find(faceName);

  if (faceItr == faceNames.end()) {
    // see if there is a default
    logDebugMessage(4,"Requested font %s not found, trying Default\n", faceName.c_str());
    faceName = "DEFAULT";
    faceItr = faceNames.find(faceName);
    if (faceItr == faceNames.end()) {
      // see if we have arial
      logDebugMessage(4,"Requested font %s not found, trying Arial\n", faceName.c_str());
      faceName = "ARIAL";
      faceItr = faceNames.find(faceName);
      if (faceItr == faceNames.end()) {
	// hell we are outta luck, you just get the first one
	logDebugMessage(4,"Requested font %s not found, trying first-loaded\n", faceName.c_str());
	faceItr = faceNames.begin();
	if (faceItr == faceNames.end()) {
	  logDebugMessage(2,"No fonts loaded\n");
	  return -1;	// we must have NO fonts, so you are screwed no matter what
	}
      }
    }
  }

  return faceItr->second;
}

int FontManager::getNumFaces(void)
{
  return (int)fontFaces.size();
}

const char* FontManager::getFaceName(int faceID)
{
  if ((faceID < 0) || (faceID > getNumFaces())) {
    logDebugMessage(2,"Trying to fetch name for invalid Font Face ID %d\n", faceID);
    return NULL;
  }

  return fontFaces[faceID].begin()->second->getFaceName();
}

void FontManager::drawString(float x, float y, float z, int faceID, float size,
			     const std::string &text, const float* resetColor)
{
  if (text.size() == 0)
    return;

  if ((faceID < 0) || (faceID > getNumFaces())) {
    logDebugMessage(2,"Trying to draw with invalid Font Face ID %d\n", faceID);
    return;
  }

  ImageFont* pFont = getClosestRealSize(faceID, size, size);

  if (!pFont) {
    logDebugMessage(2,"Could not find applicable font size for rendering; font face ID %d, "
	   "requested size %f\n", faceID, size);
    return;
  }

  float scale = size / (float)pFont->getSize();

  // filtering is off by default for fonts.
  // if the font is large enough, and the scaling factor
  // is not an integer, then request filtering
  bool filtering = false;
  if ((size > 12.0f) && (fabsf(scale - floorf(scale + 0.5f)) > 0.001f)) {
    pFont->filter(true);
    filtering = true;
  } else {
    // no filtering - clamp to aligned coordinates
    x = floorf(x);
    y = floorf(y);
    z = floorf(z);
  }


  /*
   * Colorize text based on ANSI codes embedded in it
   * Break the text every time an ANSI code
   * is encountered and do a separate pFont->drawString code for
   * each segment, with the appropriate color parameter
   */

  // sane defaults
  bool bright = true;
  bool pulsating = false;
  bool underline = false;
  // negatives are invalid, we use them to signal "no change"
  GLfloat color[4] = {-1.0f, -1.0f, -1.0f, opacity};
  if (resetColor != NULL) {
    color[0] = resetColor[0] * darkness;
    color[1] = resetColor[1] * darkness;
    color[2] = resetColor[2] * darkness;
  } else {
    resetColor = BrightColors[WhiteColor];
  }

  const float darkDim = dimFactor * darkness;

  // underline color changes for bright == false
  GLfloat dimUnderlineColor[4] = { underlineColor[0] * darkDim,
				   underlineColor[1] * darkDim,
				   underlineColor[2] * darkDim,
				   opacity };
  underlineColor[3] = opacity;

  // FIXME - this should not be necessary, but the bitmap font renderer needs it
  //  OpenGLGState::resetState();

  /*
   * ANSI code interpretation is somewhat limited, we only accept values
   * which have been defined in AnsiCodes.h
   */
  bool doneLastSection = false;
  int startSend = 0;
  int endSend = (int)text.find("\033[", startSend);
  bool tookCareOfANSICode = false;
  float width = 0;
  // run at least once
  if (endSend == -1) {
    endSend = (int)text.size();
    doneLastSection = true;
  }

  // split string into parts based on the embedded ANSI codes, render each separately
  // there has got to be a faster way to do this
  while (endSend >= 0) {
    // pulsate the text, if desired
    if (pulsating)
      getPulseColor(color, color);
    // render text
    int len = endSend - startSend;
    if (len > 0) {
      const char* tmpText = text.c_str();
      // get substr width, we may need it a couple times
      width = pFont->getStrLength(scale, &tmpText[startSend], len);
      glPushMatrix();
      glTranslatef(x, y, z);
      glDepthMask(0);
      pFont->drawString(scale, color, &tmpText[startSend], len);
      if (underline) {
	if (canScale) {
	  glDisable(GL_TEXTURE_2D);
	}
	glEnable(GL_BLEND);
	if (bright && underlineColor[0] >= 0) {
	  glColor4fv(underlineColor);
	} else if (underlineColor[0] >= 0) {
	  glColor4fv(dimUnderlineColor);
	} else if (color[0] >= 0) {
	  glColor4fv(color);
	}
	// still have a translated matrix, these coordinates are
	// with respect to the string just drawn
	glBegin(GL_LINES);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(width, 0.0f);
	glEnd();
	if (canScale) {
	  glEnable(GL_TEXTURE_2D);
	}
      }
      glDepthMask(BZDBCache::zbuffer);
      glPopMatrix();
      // x transform for next substr
      x += width;
    }
    if (!doneLastSection) {
      startSend = (int)text.find('m', endSend) + 1;
    }
    // we stopped sending text at an ANSI code, find out what it is
    // and do something about it
    if (endSend != (int)text.size()) {
      tookCareOfANSICode = false;
      std::string tmpText = text.substr(endSend, (text.find('m', endSend) - endSend) + 1);
      // colors
      for (int i = 0; i <= LastColor; i++) {
	if (tmpText == ColorStrings[i]) {
	  if (bright) {
	    color[0] = BrightColors[i][0] * darkness;
	    color[1] = BrightColors[i][1] * darkness;
	    color[2] = BrightColors[i][2] * darkness;
	  } else {
	    color[0] = BrightColors[i][0] * darkDim;
	    color[1] = BrightColors[i][1] * darkDim;
	    color[2] = BrightColors[i][2] * darkDim;
	  }
	  tookCareOfANSICode = true;
	  break;
	}
      }
      // didn't find a matching color
      if (!tookCareOfANSICode) {
	// settings other than color
	if (tmpText == ANSI_STR_RESET) {
	  bright = true;
	  pulsating = false;
	  underline = false;
	  color[0] = resetColor[0] * darkness;
	  color[1] = resetColor[1] * darkness;
	  color[2] = resetColor[2] * darkness;
	} else if (tmpText == ANSI_STR_RESET_FINAL) {
	  bright = false;
	  pulsating = false;
	  underline = false;
	  color[0] = resetColor[0] * darkDim;
	  color[1] = resetColor[1] * darkDim;
	  color[2] = resetColor[2] * darkDim;
	} else if (tmpText == ANSI_STR_BRIGHT) {
	  bright = true;
	} else if (tmpText == ANSI_STR_DIM) {
	  bright = false;
	} else if (tmpText == ANSI_STR_UNDERLINE) {
	  underline = true;
	} else if (tmpText == ANSI_STR_PULSATING) {
	  pulsating = true;
	} else if (tmpText == ANSI_STR_NO_UNDERLINE) {
	  underline = false;
	} else if (tmpText == ANSI_STR_NO_PULSATE) {
	  pulsating = false;
	} else {
	  logDebugMessage(2,"ANSI Code %s not supported\n", tmpText.c_str());
	}
      }
    }
    endSend = (int)text.find("\033[", startSend);
    if ((endSend == -1) && !doneLastSection) {
      endSend = (int)text.size();
      doneLastSection = true;
    }
  }

  // revert the filtering state
  if (filtering) {
    pFont->filter(false);
  }

  return;
}

void FontManager::drawString(float x, float y, float z,
			     const std::string &face, float size,
			     const std::string &text,
			     const float* resetColor)
{
  drawString(x, y, z, getFaceID(face), size, text, resetColor);
}

float FontManager::getStrLength(int faceID, float size,	const std::string &text,
				bool alreadyStripped)
{
  if (text.size() == 0)
    return 0.0f;

  if ((faceID < 0) || (faceID > getNumFaces())) {
    logDebugMessage(2,"Trying to find length of string for invalid Font Face ID %d\n", faceID);
    return 0.0f;
  }

  ImageFont* pFont = getClosestRealSize(faceID, size, size);

  if (!pFont) {
    logDebugMessage(2,"Could not find applicable font size for sizing; font face ID %d, "
	   "requested size %f\n", faceID, size);
    return 0.0f;
  }

  float scale = size / (float)pFont->getSize();

  // don't include ansi codes in the length, but allow outside funcs to skip this step
  const std::string &stripped = alreadyStripped ? text : stripAnsiCodes(text);

  return pFont->getStrLength(scale, stripped.c_str(), (int)stripped.size());
}

float FontManager::getStrLength(const std::string &face, float size,
				const std::string &text, bool alreadyStripped)
{
  return getStrLength(getFaceID(face), size, text, alreadyStripped);
}

float FontManager::getStrHeight(int faceID, float size,
				const std::string & /* text */)
{
  // don't scale tiny fonts
  getClosestRealSize(faceID, size, size);

  return (size * 1.5f);
}

float FontManager::getStrHeight(std::string face, float size,
				const std::string &text)
{
  return getStrHeight(getFaceID(face), size, text);
}

void FontManager::unloadAll(void)
{
  FontFaceList::iterator faceItr = fontFaces.begin();

  while (faceItr != fontFaces.end()) {
    FontSizeMap::iterator itr = faceItr->begin();
    while (itr != faceItr->end()) {
      itr->second->free();
      itr++;
    }
    faceItr++;
  }
}

ImageFont* FontManager::getClosestSize(int faceID, float size, bool bigger)
{
  if (fontFaces[faceID].size() == 0)
    return NULL;

  const int rsize = int(size + 0.5f);

  const FontSizeMap &sizes = fontFaces[faceID];
  FontSizeMap::const_iterator itr = sizes.lower_bound(rsize);
  if (bigger) {
    if (itr == sizes.end())
      itr--;
  } else {
    if (itr != sizes.begin() && itr->first != rsize)
      itr--;
  }

  return itr->second;
}

ImageFont*    FontManager::getClosestRealSize(int faceID, float desiredSize, float &actualSize)
{
  /*
   * tiny fonts scale poorly, this function will return the nearest unscaled size of a font
   * if the font is too tiny to scale, and a scaled size if it's big enough.
   */

  ImageFont* font = getClosestSize(faceID, desiredSize, canScale ? true : false);
  if (!canScale || desiredSize < 14.0f) {
    // get the next biggest font size from requested
    if (!font) {
      logDebugMessage(2,"Could not find applicable font size for sizing; font face ID %d, "
	     "requested size %f\n", faceID, desiredSize);
      return NULL;
    }
    actualSize = (float)font->getSize();
  } else {
    actualSize = desiredSize;
  }
  return font;
}

void	    FontManager::getPulseColor(const GLfloat *color, GLfloat *pulseColor) const
{
  float pulseTime = (float)TimeKeeper::getCurrent().getSeconds();

  // depth is how dark it should get (1.0 is to black)
  float pulseDepth = BZDBCache::pulseDepth;
  // rate is how fast it should pulsate (smaller is faster)
  float pulseRate = BZDBCache::pulseRate;

  float pulseFactor = fmodf(pulseTime, pulseRate) - pulseRate /2.0f;
  pulseFactor = fabsf(pulseFactor) / (pulseRate/2.0f);
  pulseFactor = pulseDepth * pulseFactor + (1.0f - pulseDepth);

  pulseColor[0] = color[0] * pulseFactor;
  pulseColor[1] = color[1] * pulseFactor;
  pulseColor[2] = color[2] * pulseFactor;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
