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

#include "OpenGLUtils.h"

// system headers
#include <math.h>

// common headers
#include "bzfgl.h"
#include "bzfio.h"
#include "vectors.h"
#include "BZDBCache.h"
#include "BzMaterial.h"
#include "DynamicColor.h"
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"
#include "SceneRenderer.h"
#include "TextureManager.h"
#include "TextureMatrix.h"


#define INVALID_GL_ID 0xffffffff


//============================================================================//
//============================================================================//

void bzMat2gstate(const BzMaterial* bzmat, OpenGLGState& gstate,
		  fvec4& color, const fvec4*& colorPtr)
{
  // cheat a little
  ((BzMaterial*)bzmat)->setReference();

  OpenGLGStateBuilder builder;

  // ways of requiring blending
  bool colorAlpha   = false;
  bool textureAlpha = false;

  bool useDiffuseColor = true;

  // texturing
  if (BZDBCache::texture && (bzmat->getTextureCount() > 0)) {
    TextureManager &tm = TextureManager::instance();
    int texID = -1;

    const std::string& texName = bzmat->getTextureLocal(0);
    if (texName.size() > 0) {
      texID = tm.getTextureID(texName.c_str());
    }

    if (texID < 0) {
      texID = tm.getTextureID("mesh", false /* no failure reports */);
    }
    else {
      useDiffuseColor = bzmat->getUseColorOnTexture(0);
      if (bzmat->getUseTextureAlpha(0)) {
	const ImageInfo& imageInfo = tm.getInfo(texID);
	textureAlpha = imageInfo.alpha;
      }
    }

    if (texID >= 0) {
      // sphere mapping
      if (bzmat->getUseSphereMap(0)) {
	builder.enableSphereMap(true);
      }
      // texture matrix
      const int texMatId = bzmat->getTextureMatrix(0);
      const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texMatId);
      if (texmat != NULL) {
	const GLfloat* matrix = texmat->getMatrix();
	if (matrix != NULL) {
	  builder.setTextureMatrix(matrix);
	  builder.enableTextureMatrix(true);
	}
      }
      // setup the builder
      builder.setTexture(texID);
      builder.enableTexture(true);
    }
  }

  // lighting
  builder.setShading(GL_FLAT);
  if (BZDBCache::lighting && !bzmat->getNoLighting()) {
    OpenGLMaterial oglMaterial(bzmat->getSpecular(),
			       bzmat->getEmission(),
			       bzmat->getShininess());
    builder.setMaterial(oglMaterial);
  }

  // color
  if (useDiffuseColor) {
    color = bzmat->getDiffuse();
    colorAlpha = (color.a < 1.0f);
  } else {
    // set it to white, this should only happen when
    // we've gotten a user texture, and there's a
    // request to not use the material's diffuse color.
    color = fvec4(1.0f, 1.0f, 1.0f, 1.0f);
  }

  // dynamic color
  const DynamicColor* dyncol = DYNCOLORMGR.getColor(bzmat->getDynamicColor());
  if (dyncol != NULL) {
    const float* c = dyncol->getColor();
    colorPtr = new fvec4(c[0],c[1],c[2],c[3]);
    colorAlpha = dyncol->canHaveAlpha(); // override
  } else {
    colorPtr = &color;
  }

  // blending
  bool needsSorting   = false;
  bool blendingForced = false;

  if (!blendingForced) {
    const bool isAlpha = (colorAlpha || textureAlpha);
    if (isAlpha) {
      if (BZDBCache::blend) {
	builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	builder.setStipple(1.0f);
	needsSorting = true;
      } else {
	builder.resetBlending();
	if (dyncol != NULL) {
	  builder.setStipple(0.5f);
	} else {
	  builder.setStipple(color.a);
	}
      }
    }
  }

  // sorting  (do this after using setBlending())
  builder.setNeedsSorting(needsSorting && !bzmat->getNoSorting());

  // alpha thresholding
  float alphaThreshold = bzmat->getAlphaThreshold();
  if (alphaThreshold != 0.0f) {
    builder.setAlphaFunc(GL_GEQUAL, alphaThreshold);
  }

  // culling
  if (bzmat->getNoCulling()) {
    builder.setCulling(GL_NONE);
  }

  // generate the gstate
  gstate = builder.getState();
}


//============================================================================//
//============================================================================//

static bool parseBlendFactor(const std::string& s, GLenum& factor)
{
  static std::map<std::string, GLenum> factors;
  if (factors.empty()) {
    factors["one"]  = GL_ONE;
    factors["zero"] = GL_ZERO;
    factors["src_alpha"] = GL_SRC_ALPHA;
    factors["src_color"] = GL_SRC_COLOR;
    factors["dst_alpha"] = GL_DST_ALPHA;
    factors["dst_color"] = GL_DST_COLOR;
    factors["one_minus_src_alpha"] = GL_ONE_MINUS_SRC_ALPHA;
    factors["one_minus_src_color"] = GL_ONE_MINUS_SRC_COLOR;
    factors["one_minus_dst_alpha"] = GL_ONE_MINUS_DST_ALPHA;
    factors["one_minus_dst_color"] = GL_ONE_MINUS_DST_COLOR;
    factors["src_alpha_saturate"] = GL_SRC_ALPHA_SATURATE;
  }
  const std::map<std::string, GLenum>::const_iterator it = factors.find(s);
  if (it == factors.end()) {
    return false;
  }
  factor = it->second;
  return true;
}


bool parseBlendFactors(const std::string& s, GLenum& src, GLenum& dst)
{
  if (s == "disable") {
    src = GL_ONE; dst = GL_ZERO;
    return true;
  }
  else if (s == "add") {
    src = GL_ONE; dst = GL_ONE;
    return true;
  }
  else if (s == "addalpha") {
    src = GL_SRC_ALPHA; dst = GL_ONE;
    return true;
  }
  else if (s == "modulate") {
    src = GL_DST_COLOR; dst = GL_ZERO;
    return true;
  }
  else if (s == "alpha") {
    src = GL_SRC_ALPHA; dst = GL_ONE_MINUS_SRC_ALPHA;
    return true;
  }
  else if (s == "color") {
    src = GL_SRC_COLOR; dst = GL_ONE_MINUS_SRC_COLOR;
    return true;
  }

  const std::string::size_type pos = s.find('/');
  if (pos == std::string::npos) {
    return false;
  }
  const std::string srcStr = s.substr(0, pos);
  const std::string dstStr = s.substr(pos + 1);
  if (!parseBlendFactor(srcStr, src) ||
      !parseBlendFactor(dstStr, dst)) {
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//

float getFloatColor(int val)
{
  return val / 255.0f;
}

void setColor(float dst[3], int r, int g, int b)
{
  dst[0] = getFloatColor(r);
  dst[1] = getFloatColor(g);
  dst[2] = getFloatColor(b);
}

void glSetColor(const float c[3], float alpha)
{
  glColor4f(c[0], c[1], c[2], alpha);
}

void glTranslatefv (const float v[3] )
{
  glTranslatef(v[0], v[1], v[2]);
}

void glQuad ( float x, float y, eAlignment align, float scale )
{
  glPushMatrix();

  x *= scale;
  y *= scale;

  switch ( align )
    {
    case eCenter:
      glTranslatef(-x*0.5f,-y*0.5f,0);
      break;
    case eLowerLeft:
      break;
    case eLowerRight:
      glTranslatef(-x,0,0);
      break;
    case eUpperLeft:
      glTranslatef(0,-y,0);
      break;
    case eUpperRight:
      glTranslatef(-x,-y,0);
      break;
    case eCenterLeft:
      glTranslatef(0,-y*0.5f,0);
      break;
    case eCenterRight:
      glTranslatef(-x,-y*0.5f,0);
      break;
    case eCenterTop:
      glTranslatef(-x*0.5f,-y,0);
      break;
    case eCenterBottom:
      glTranslatef(-x*0.5f,0,0);
      break;
    }

  glBegin(GL_QUADS);

  glNormal3f(0,0,1);
  glTexCoord2f(0,1);
  glVertex3f(0,0,0);

  glTexCoord2f(1,1);
  glVertex3f(x,0,0);

  glTexCoord2f(1,0);
  glVertex3f(x,y,0);

  glTexCoord2f(0,0);
  glVertex3f(0,y,0);

  glEnd();

  glPopMatrix();
}

#define GL_RAD_CON 0.017453292519943295769236907684886f

void glLineRing ( float radius, float width )
{
  int segments = 180;

  glDisable(GL_TEXTURE_2D);
  glLineWidth(width);
  glBegin(GL_LINE_LOOP);
  for (float a = 0; a < 360.0f; a += segments/360.0f)
    glVertex3f(cosf(a*GL_RAD_CON)*radius,sinf(a*GL_RAD_CON)*radius,0);
  glEnd();
  glLineWidth(1);
  glEnable(GL_TEXTURE_2D);
}

void glOutlineBoxCP ( float thickness, float centerX, float centerY, float width, float height, float depth )
{
  glPushMatrix();
  glTranslatef(centerX,centerY,depth);

  if (thickness <= 0)
    thickness = 1.0f;

  // ok, now what we do here is make this sucker into quads, the sizes are the outer size, and we inset by the thickness

  glNormal3f(0,0,1);
  glBegin(GL_QUADS);

  // draw the top
  glVertex2f(width,height);
  glVertex2f(-width,height);
  glVertex2f(-width+thickness,height-thickness);
  glVertex2f(width-thickness,height-thickness);

  // draw the bottom
  glVertex2f(width,-height);
  glVertex2f(width-thickness,-height+thickness);
  glVertex2f(-width+thickness,-height+thickness);
  glVertex2f(-width,-height);

  // draw the right
  glVertex2f(width,height);
  glVertex2f(width-thickness,height-thickness);
  glVertex2f(width-thickness,-height+thickness);
  glVertex2f(width,-height);

  // draw the left
  glVertex2f(-width,height);
  glVertex2f(-width,-height);
  glVertex2f(-width+thickness,-height+thickness);
  glVertex2f(-width+thickness,height-thickness);

  glEnd();

  glPopMatrix();
}

void glOutlineBoxHV ( float thickness, float minX, float minY, float maxX, float maxY, float depth )
{
  glPushMatrix();
  glTranslatef(0,0,depth);

  if (thickness <= 0)
    thickness = 1.0f;

  // ok, now what we do here is make this sucker into quads, the sizes are the outer size, and we inset by the thickness

  glNormal3f(0,0,1);
  glBegin(GL_QUADS);

  // draw the top
  glVertex2f(maxX,maxY);
  glVertex2f(minX,maxY);
  glVertex2f(minX+thickness,maxY-thickness);
  glVertex2f(maxX-thickness,maxY-thickness);

  // draw the bottom
  glVertex2f(maxX,minY);
  glVertex2f(maxX-thickness,minY+thickness);
  glVertex2f(minX+thickness,minY+thickness);
  glVertex2f(minX,minY);

  // draw the right
  glVertex2f(maxX,maxY);
  glVertex2f(maxX-thickness,maxY-thickness);
  glVertex2f(maxX-thickness,minY+thickness);
  glVertex2f(maxX,minY);

  // draw the right
  glVertex2f(minX,maxY);
  glVertex2f(minX,minY);
  glVertex2f(minX+thickness,minY+thickness);
  glVertex2f(minX+thickness,maxY-thickness);

  glEnd();

  glPopMatrix();
}

void glOutlineTabbedBox ( float thickness, float minX, float minY, float maxX, float maxY, float tabInset, float tabWidth, float tabHeight, float depth )
{
  glPushMatrix();
  glTranslatef(0,0,depth);

  if (thickness <= 0)
    thickness = 1.0f;

  float width = maxX - minX;
  // can't have a tab larger then the width for now
  // this is realy just for errors
  if (tabWidth > width)
    tabWidth = width;

  // if for some reason the tab inset would push the tab off the end, clamp it to the end
  if (tabInset > width-tabWidth)
    tabInset = width-tabWidth;

  if (tabHeight < 0 )
    tabHeight = 0;

  // ok, now what we do here is make this sucker into quads, the sizes are the outer size, and we inset by the thickness

  glNormal3f(0,0,1);
  glBegin(GL_QUADS);

  // draw the bottom
  glVertex2f(maxX,minY);
  glVertex2f(maxX-thickness,minY+thickness);
  glVertex2f(minX+thickness,minY+thickness);
  glVertex2f(minX,minY);

  // draw the left
  glVertex2f(minX,maxY);
  glVertex2f(minX,minY);
  glVertex2f(minX+thickness,minY+thickness);
  glVertex2f(minX+thickness,maxY-thickness);

  // draw the right
  glVertex2f(maxX,maxY);
  glVertex2f(maxX-thickness,maxY-thickness);
  glVertex2f(maxX-thickness,minY+thickness);
  glVertex2f(maxX,minY);


  // draw the top
  glVertex2f(maxX,maxY);
  glVertex2f(minX,maxY);
  glVertex2f(minX+thickness,maxY-thickness);
  glVertex2f(maxX-thickness,maxY-thickness);

  glEnd();

  glPopMatrix();

}


// DisplayListSystem

DisplayListSystem::~DisplayListSystem()
{
  flushLists();
}

void DisplayListSystem::flushLists(void)
{
  std::map<GLDisplayList,DisplayList>::iterator itr = lists.begin();

  while (itr != lists.end()) {
    if (itr->second.list != INVALID_GL_ID)
      glDeleteLists(itr->second.list, 1);

    itr->second.list = INVALID_GL_ID;
    ++itr;
  }
}

GLDisplayList DisplayListSystem::newList (GLDisplayListCreator *creator)
{
  if (!creator)
    return 0;

  DisplayList displayList;
  displayList.creator = creator;
  displayList.list = INVALID_GL_ID;

  lastList++;
  lists[lastList] = displayList;
  return lastList;
}

void DisplayListSystem::freeList (GLDisplayList displayList)
{
  std::map<GLDisplayList,DisplayList>::iterator itr = lists.find(displayList);
  if (itr == lists.end())
    return;

  if (itr->second.list != INVALID_GL_ID)
    glDeleteLists(itr->second.list, 1);

  lists.erase(itr);
}

void DisplayListSystem::callList (GLDisplayList displayList)
{
  std::map<GLDisplayList,DisplayList>::iterator itr = lists.find(displayList);
  if (itr == lists.end())
    return;

  if (itr->second.list == INVALID_GL_ID)
    {
      itr->second.list = glGenLists(1);
      glNewList(itr->second.list,GL_COMPILE);
      itr->second.creator->buildGeometry(displayList);
      glEndList();
    }

  glCallList(itr->second.list);
}

void DisplayListSystem::callListsV (std::vector<GLDisplayList> &displayLists)
{
  for (unsigned int i = 0; i < displayLists.size(); i++)
    callList(displayLists[i]);
}

DisplayListSystem::DisplayListSystem()
{
  lastList = 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
