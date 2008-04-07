/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FONTMANAGER_H__
#define __FONTMANAGER_H__

#include "common.h"

/* system interface headers */
#include <map>
#include <string>
#include <vector>

/* common interface headers */
#include "bzfgl.h"
#include "AnsiCodes.h"
#include "Singleton.h"

typedef enum
{
  AlignLeft,
  AlignCenter,
  AlignRight
} fontJustification;

class BZFontFace_impl;
/**
 * This class (currently just an interface) will become the token that
 * is passed around instead of font id. The functions defined are
 * those of FontManager used by other classes
 */
class BZFontFace
{
public:
  BZFontFace();
  BZFontFace(std::string const& name_, std::string const& path_);

  /**
   * Rather than let the destructor release all the fonts, make an
   * explicit call here, until I can prove that there isn't an issue
   * with dangling references
   */
  void destroy();

  /**
   * returns the height of the given font size
   */
  float getStringHeight(float size);

  /**
   * returns the width of the given text string for the specifed font
   */
  float getStringWidth(float size, const char *text, bool alreadyStripped = false);

  /**
   * main work-horse.  render the provided text with the specified
   * font size, optionally justifying to a particular alignment.
   */
  void drawString(float x, float y, float z, float size,
		  const char *text,
		  const float* resetColor = NULL, fontJustification align = AlignLeft);
private:
  BZFontFace_impl*      impl;
};

/**
 * An FTGL-based font manager system with enhancements for font
 * decorations like underline and opacity factors.
 */
class FontManager : public Singleton<FontManager> {

public:

  /**
   * load all fonts from a given directory, returns the number of
   * fonts that were loaded
   */
  int loadAll(std::string dir);

  /**
   * return an index to the requested font if it's been loaded or the
   * first loaded font otherwise.
   */
  int getFaceID(const std::string faceName);

  /**
   * main work-horse.  render the provided text with the specified
   * font size, optionally justifying to a particular alignment.
   */
  void drawString(float x, float y, float z, int faceID, float size,
		  const char *text, 
		  const float* resetColor = NULL, fontJustification align = AlignLeft);

  /**
   * returns the width of the given text string for the specifed font
   */
  float getStringWidth(int faceID, float size, const char *text, bool alreadyStripped = false);

  /**
   * returns the height of the given font size
   */
  float getStringHeight(int faceID, float size);

  void setDimFactor(float newDimFactor);
  void setOpacity(float newOpacity);
  void setDarkness(float newDimFactor);

protected:

  friend class Singleton<FontManager>;

  /**
   * lookup the font ID for a given font, or -1 on failure
   */
  int lookupID(const std::string faceName);

  /**
   * return the pulse color
   */
  void getPulseColor(const GLfloat* color, GLfloat* pulseColor) const;

  /**
   * load a specified font
   */
  int load(const char* file);

  /**
   * clear/erase a particular font size
   */
  void clear(int font, int size);

  /**
   * destroy all the fonts, clear all the lists
   */
  void clear();

  /**
   * ask ftgl to compute their width so that the textures are loaded,
   * gives small performance boost by loading all glyphs at once
   * upfront.
   */
  void preloadSize(int font, int size);

  /**
   * rebuild just one size of a given font
   */
  void rebuildSize(int font, int size);

  /**
   * rebuild all the lists
   */
  void rebuild(void);

  /**
   * returns the number of fonts loaded
   */
  int getNumFaces(void);

  /**
   * given a font ID, return that font's name
   */
  const char* getFaceName(int faceID);

private:

  /**
   * default constructor, protected because of singleton
   */
  FontManager();

  /**
   * default destructor, protected because of singleton
   */
  ~FontManager();


  /** location of fonts */
  std::string	fontDirectory;
  /** font opacity */
  float		opacity;
  /** ANSI code dimming */
  float		dimFactor; 
  /** darkening of all colors */
  float		darkness;
  /** precompute colors on dim/darkness changes */
  GLfloat dimUnderlineColor[4];

  /**
   * STATIC: called during "underline"
   */
  static void	underlineCallback(const std::string& name, void *);

  /** STATIC: underline color */
  static GLfloat underlineColor[4];

  /**
   * STATIC: GL Context callbacks
   */
  static void initContext(void*);
  static void freeContext(void*);
};


inline void FontManager::setDimFactor(float newDimFactor)
{
  const float darkDim = newDimFactor * darkness;
  dimFactor = newDimFactor;
  dimUnderlineColor[0] = underlineColor[0] * darkDim;  
  dimUnderlineColor[1] = underlineColor[1] * darkDim;
  dimUnderlineColor[2] = underlineColor[2] * darkDim;
  dimUnderlineColor[3] = opacity;
}

inline void FontManager::setOpacity(float newOpacity)
{
  opacity = newOpacity;
  underlineColor[3] = opacity;
}

inline void FontManager::setDarkness(float newDarkness)
{
  darkness = newDarkness;
  const float darkDim = dimFactor * darkness;
  dimUnderlineColor[0] = underlineColor[0] * darkDim;  
  dimUnderlineColor[1] = underlineColor[1] * darkDim;
  dimUnderlineColor[2] = underlineColor[2] * darkDim;
  dimUnderlineColor[3] = opacity;
}

#endif /* __FONTMANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
