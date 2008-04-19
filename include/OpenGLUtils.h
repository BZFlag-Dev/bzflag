/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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

/* interface system headers */
#include <map>
#include <vector>

/* common interface headers */
#include "bzfgl.h"

#define GL_INVALID_ID 0xffffffff

#define deg2Rad 0.017453292519943295769236907684886f

float getFloatColor ( int val );
void setColor ( float c[3], int r, int g, int b );
void glSetColor ( float c[3], float alpha = 1.0f );
void glTranslatefv ( float v[3] );

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

void glQuad ( float x, float y, eAlignment align, float scale = 1.0f );
void glLineRing ( float radius, float width = 1 );

// draw an outline box with the outside at the bounds, and inset by the thickness
void glOutlineBoxCP ( float thickness, float centerX, float centerY, float width, float height, float depth = 0 );
inline void glOutlineBoxCP ( float t, int x, int y, int w, int h, int d = 0 ){glOutlineBoxCP(t,(float)x,(float)y,(float)w,(float)h,(float)d);}
void glOutlineBoxHV ( float thickness, float minX, float minY, float maxX, float maxY, float depth = 0 );
inline void glOutlineBoxHV ( float t, int minX, int minY, int maxX, int maxY, int d = 0 ){glOutlineBoxHV(t,(float)minX,(float)minY,(float)maxX,(float)maxY,(float)d);}

typedef unsigned int GLDisplayList;

#define _INVALID_LIST GL_INVALID_ID

class GLDisplayListCreator
{
 public:
  virtual ~GLDisplayListCreator(){};

  virtual void buildGeometry ( GLDisplayList displayList ) = 0;
};

class DisplayListSystem
{
 public:
  static DisplayListSystem& Instance() {
    static DisplayListSystem dls;
    return dls;
  }

  ~DisplayListSystem();

  GLDisplayList newList (GLDisplayListCreator *creator);
  void freeList (GLDisplayList displayList);

  void flushLists ( void );

  void callList (GLDisplayList displayList);
  void callListsV (std::vector<GLDisplayList> &displayLists);

 protected:
  DisplayListSystem();

  typedef struct _DisplayList {
    GLDisplayListCreator	*creator;
    GLuint			glList;
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
