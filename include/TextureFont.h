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

typedef struct {
  int initialDist;
  int charWidth;
  int whiteSpaceDist;
  int startX;
  int endX;
  int startY;
  int endY;
} trFontMetrics;

class TextureFont {
public:
  TextureFont();
  ~TextureFont();

  int getSize(void);
  const char* getFaceName(void);

  bool load(OSFile &file);

  void build(void);

  bool isBuilt(void) {return !(textureID == -1);};

  void drawString(float scale, GLfloat color[3], const char *str, int len);

  float getStrLength(float scale, const char *str, int len);

  void free(void);

private:
  void preLoadLists(void);
  bool fmtRead(OSFile &file, std::string expectedLeft, std::string &retval);

  unsigned int	listIDs[128];
  trFontMetrics	fontMetrics[128];

  std::string faceName;
  std::string texture;
  int	      size;
  int	      textureID;
  int	      textureXSize;
  int	      textureYSize;
  int	      textureZStep;
  int	      numberOfCharacters;
  OpenGLGState gstate;
};

#endif //_TEXTURE_FONT_H_
