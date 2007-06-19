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

#ifndef __FONTMANAGER_H__
#define __FONTMANAGER_H__

#include "common.h"

/* interface header */
#include "Singleton.h"

/* system interface headers */
#include <map>
#include <string>
#include <vector>
#include "FTGLTextureFont.h"

/* common interface headers */
#include "bzfgl.h"
#include "AnsiCodes.h"


typedef enum
{
  AlignLeft,
  AlignCenter,
  AlignRight
} fontJustification;


/**
 * Jeff's FTGL-based font manager system, merged with BZFlag's
 * previous Font Manager (which DTR and Jeff also worked on).
 */
class FontManager : public Singleton<FontManager> {
public:
  int load ( const char* file );
  int loadAll(std::string dir);

  void clear(int font, int size);
  void clear();

  void preloadSize ( int font, int size );
  void rebuildSize ( int font, int size );
  void rebuild(void);

  std::vector<std::string> getFontList ( void );
  int getFaceID(std::string faceName);
  int getNumFaces(void);
  const char* getFaceName(int faceID);

  void drawString(float x, float y, float z, int faceID, float size, const std::string &text, const float* resetColor = NULL, fontJustification align = AlignLeft);
  void drawString(float x, float y, float z, const std::string &face, float size, const std::string &text, const float* resetColor = NULL, fontJustification align = AlignLeft);

  float getStringWidth(int faceID, float size, const std::string &text, bool alreadyStripped = false);
  float getStringWidth(const std::string &face, float size, const std::string &text, bool alreadyStripped = false);

  float getStringHeight(int faceID, float size);
  float getStringHeight(std::string face, float size);

  void setDimFactor(float newDimFactor);
  void setOpacity(float newOpacity);
  void setDarkness(float newDimFactor);


protected:

  friend class Singleton<FontManager>;

  FTGLTextureFont* getGLFont(int face, int size); 

  typedef struct _FontFace {
    std::string name;
    std::string path;
    std::map<int,FTGLTextureFont*> sizes;
  } FontFace;
  
  std::map<std::string,int>	faceNames;
  std::vector<FontFace>		fontFaces;

private:

  FontManager();
  ~FontManager();

  void		getPulseColor(const GLfloat* color, GLfloat* pulseColor) const;

  std::string	fontDirectory;

  float		opacity;
  float		dimFactor; // ANSI code dimming
  float		darkness;  // darkening of all colors

  static void	underlineCallback(const std::string& name, void *);
  static void	freeContext(void *data);
  static void	initContext(void *data);
  static GLfloat underlineColor[4];
};

inline void FontManager::setDimFactor(float newDimFactor)
{
  dimFactor = newDimFactor;
}

inline void FontManager::setOpacity(float newOpacity)
{
  opacity = newOpacity;
  underlineColor[3] = opacity;
}

inline void FontManager::setDarkness(float newDarkness)
{
  darkness = newDarkness;
}

#endif /* __FONTMANAGER_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
