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

#ifndef _TEXTURE_FONT_H_
#define _TEXTURE_FONT_H_

#ifdef _MSC_VER
  #pragma warning(disable : 4786)  // Disable warning message
#endif

#include "OSFile.h"
#include "Singleton.h"
#include "OpenGLGState.h"

#include <map>
#include <string>
#include <vector>

#define MAX_TEXTURE_FONT_CHARS	(128)

class ImageFont {
public:
  ImageFont();
  virtual ~ImageFont();

  int getSize() const;
  const char* getFaceName() const;

  bool load(OSFile &file);

  virtual void build() = 0;
  virtual bool isBuilt() const = 0;

  virtual void filter(bool dofilter) = 0;
  virtual void drawString(float scale, GLfloat color[3], const char *str, int len) = 0;

  float getStrLength(float scale, const char *str, int len) const;

  virtual void free() = 0;

protected:
  struct FontMetrics {
    int initialDist;
    int charWidth;
    int whiteSpaceDist;
    int fullWidth; // initialDist + charWidth + whiteSpaceDist
    int startX;
    int endX;
    int startY;
    int endY;
  };
  FontMetrics	fontMetrics[MAX_TEXTURE_FONT_CHARS];

  std::string faceName;
  std::string texture;
  int	      size;
  int	      textureXSize;
  int	      textureYSize;
  int	      textureZStep;
  int	      numberOfCharacters;

private:
  // don't copy me
  ImageFont(const ImageFont&);
  ImageFont &operator=(const ImageFont&);
};

///////////////////////

class TextureFont : public ImageFont {
public:
  TextureFont();
  virtual ~TextureFont();

  virtual void build();
  virtual bool isBuilt() const {return textureID != -1;}

  virtual void filter(bool dofilter);
  virtual void drawString(float scale, GLfloat color[3], const char *str, int len);

  virtual void free();

private:
  void preLoadLists();

  unsigned int	listIDs[MAX_TEXTURE_FONT_CHARS];

  int	      textureID;
  OpenGLGState gstate;
};

///////////////////////

class BitmapFont : public ImageFont {
public:
  BitmapFont();
  virtual ~BitmapFont();

  virtual void build();
  virtual bool isBuilt() const {return loaded;}

  virtual void filter(bool dofilter);
  virtual void drawString(float scale, GLfloat color[3], const char *str, int len);

  virtual void free();

private:
  unsigned char *bitmaps[MAX_TEXTURE_FONT_CHARS];
  bool        loaded;
};

#endif //_TEXTURE_FONT_H_
