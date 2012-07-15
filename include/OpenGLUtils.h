/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 * Writen By Jeffrey Myers
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __OPENGLUTILS_H__
#define __OPENGLUTILS_H__

#include "common.h"

// system headers
#include <string>
#include <map>
#include <vector>

// common headers
#include "bzfgl.h"
#include "vectors.h"


class BzMaterial;
class OpenGLGState;


#define INVALID_GL_ID 0xffffffff


extern void bzMat2gstate(const BzMaterial* bzmat, OpenGLGState& gstate,
			 fvec4& color, const fvec4*& colorPtr);

extern bool parseBlendFactors(const std::string& s, GLenum& src, GLenum& dst);

extern float getFloatColor(int val);
extern void setColor(float dstColor[3], int r, int g, int b);
extern void glSetColor(const float c[3], float alpha = 1.0f);
extern void glTranslatefv(const float v[3]);

typedef enum {
  eCenter,
  eLowerLeft,
  eLowerRight,
  eUpperLeft,
  eUpperRight,
  eCenterLeft,
  eCenterRight,
  eCenterTop,
  eCenterBottom
} eAlignment;

extern void glQuad(float x, float y, eAlignment align, float scale = 1.0f);
extern void glLineRing(float radius, float width = 1.0f);

// draw an outline box with the outside at the bounds, and inset by the thickness
extern void glOutlineBoxCP(float thickness, float centerX, float centerY,
		    float width, float height, float depth = 0.0f);
inline void glOutlineBoxCP(float t, int x, int y, int w, int h, int d = 0) {
  glOutlineBoxCP(t, (float)x, (float)y, (float)w, (float)h, (float)d);
}

extern void glOutlineBoxHV(float thickness, float minX, float minY,
			   float maxX, float maxY, float depth = 0.0f);
inline void glOutlineBoxHV(float t, int minX, int minY, int maxX, int maxY, int d = 0) {
  glOutlineBoxHV(t, (float)minX, (float)minY, (float)maxX, (float)maxY, (float)d);
}

// draw an outline tabbed box
extern void glOutlineTabbedBox(float thickness, float minX, float minY,
			       float maxX, float maxY,
			       float tabInset, float tabWidth, float tabHeight,
			       float depth = 0);

// display list system
typedef unsigned int GLDisplayList;

#define _INVALID_LIST INVALID_GL_ID


class GLDisplayListCreator
{
 public:
  virtual ~GLDisplayListCreator() {}

  virtual void buildGeometry(GLDisplayList displayList) = 0;
};


class DisplayListSystem
{
 public:
  static DisplayListSystem& Instance() {
    static DisplayListSystem dls;
    return dls;
  }

  ~DisplayListSystem();

  GLDisplayList newList(GLDisplayListCreator *creator);
  void freeList(GLDisplayList displayList);

  void flushLists();

  void callList(GLDisplayList displayList);
  void callListsV(std::vector<GLDisplayList> &displayLists);

 protected:
  DisplayListSystem();

  typedef struct _DisplayList {
    GLDisplayListCreator	*creator;
    unsigned int		list;
  } DisplayList;

  std::map<GLDisplayList,DisplayList>	lists;
  GLDisplayList				lastList;
};


#endif // __OPENGLUTILS_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
