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

#ifndef _FONT_MANAGER_H_
#define _FONT_MANAGER_H_

#ifdef _MSC_VER
  #pragma warning(disable : 4786)  // Disable warning message
#endif

#include <map>
#include <string>
#include <vector>

#include "Singleton.h"
#include "TextureFont.h"
#include "AnsiCodes.h"

typedef std::map<int, TextureFont*> FontSizeMap;
typedef std::vector<FontSizeMap>    FontFaceList;
typedef std::map<std::string, int>  FontFaceMap;

class FontManager : public Singleton<FontManager> {
public:
  FontManager();
  ~FontManager();

  void loadAll(std::string dir);

  void clear();

  void rebuild(void);

  int getFaceID(std::string faceName);

  int getNumFaces(void);
  const char* getFaceName(int faceID);

  void drawString(float x, float y, float z, int faceID, float size,
		  const std::string &text);
  void drawString(float x, float y, float z, const std::string &face,
		  float size, const std::string &text);

  float getStrLength(int faceID, float size, const std::string &text,
		     bool alreadyStripped = false);
  float getStrLength(const std::string &face, float size,
		     const std::string &text, bool alreadyStripped = false);

  float getStrHeight(int faceID, float size, const std::string &text);
  float getStrHeight(std::string face, float size, const std::string &text);

  void setDimFactor(float newDimFactor);

  void unloadAll(void);

protected:
  friend class Singleton<FontManager>;

private:
  void		getPulseColor(const GLfloat* color, GLfloat* pulseColor) const;
  TextureFont*	getClosestSize(int faceID, float size);
  TextureFont*	getClosestRealSize(int faceID, float desiredSize, float &actualSize);
  FontFaceMap	faceNames;
  FontFaceList  fontFaces;

  /* FIXME: this string needs to go away when the problem detailed in
   * FontManager::rebuild() is fixed.
   */
  std::string	fontDirectory;

  static void	callback(const std::string& name, void *);
  static void	freeContext(void *data);
  static void	initContext(void *data);
  static GLfloat underlineColor[3];
  float		 dimFactor;
};

inline void FontManager::setDimFactor(float newDimFactor)
{
  dimFactor = newDimFactor;
}

#endif //_FONT_MANAGER_H_
