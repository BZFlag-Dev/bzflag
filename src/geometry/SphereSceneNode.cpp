/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface header
#include "SphereSceneNode.h"

// system headers
#include <math.h>

// common headers
#include "bzfgl.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLMaterial.h"
#include "TextureManager.h"

// local headers
#include "ViewFrustum.h"


//============================================================================//

//
// SphereSceneNode
//

SphereSceneNode::SphereSceneNode(const fvec3& pos, float _radius) {
  transparent = false;

  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_NONE);
  gstate = builder.getState();

  setColor(1.0f, 1.0f, 1.0f, 1.0f);

  // position sphere
  move(pos, _radius);

  return;
}


SphereSceneNode::~SphereSceneNode() {
  // do nothing
}


void SphereSceneNode::setColor(float r, float g, float b, float a) {
  color = fvec4(r, g, b, a);
  transparent = (color.a != 1.0f);
}


void SphereSceneNode::setColor(const fvec4& rgba) {
  color = rgba;
  transparent = (color.a != 1.0f);
}


void SphereSceneNode::move(const fvec3& pos, float _radius) {
  radius = _radius;
  setCenter(pos);
  setRadius(radius * radius);
}


void SphereSceneNode::notifyStyleChange() {
  OpenGLGStateBuilder builder(gstate);
  if (transparent) {
    if (BZDBCache::blend) {
      builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      builder.setStipple(1.0f);
      builder.setNeedsSorting(true);
    }
    else {
      builder.resetBlending();
      builder.setStipple(0.5f);
      builder.setNeedsSorting(true);
    }
  }
  else {
    builder.resetBlending();
    builder.setStipple(1.0f);
    builder.setNeedsSorting(false);
  }
  gstate = builder.getState();
}


//============================================================================//

//
// SphereLodSceneNode
//


bool         SphereLodSceneNode::initialized = false;
unsigned int SphereLodSceneNode::lodLists[sphereLods];
float        SphereLodSceneNode::lodPixelsSqr[sphereLods];
int          SphereLodSceneNode::listTriangleCount[sphereLods];


static GLuint buildSphereList(GLdouble radius, GLint slices, GLint stacks) {
  GLuint list;

  GLUquadric* quad = gluNewQuadric();
  gluQuadricDrawStyle(quad, GLU_FILL);
  gluQuadricTexture(quad, GL_TRUE);
  gluQuadricNormals(quad, GL_SMOOTH);
  gluQuadricOrientation(quad, GLU_OUTSIDE);

  list = glGenLists(1);
  glNewList(list, GL_COMPILE);
  {
    gluSphere(quad, radius, slices, stacks);
  }
  glEndList();

  gluDeleteQuadric(quad);

  return list;
}


void SphereLodSceneNode::freeContext(void*) {
  for (int i = 0; i < sphereLods; i++) {
    if (lodLists[i] != INVALID_GL_LIST_ID) {
      glDeleteLists(lodLists[i], 1);
      lodLists[i] = INVALID_GL_LIST_ID;
    }
  }
  return;
}


static int calcTriCount(int slices, int stacks) {
  const int trifans = 2 * slices;
  const int quads = 2 * (slices * (stacks - 2));
  return (trifans + quads);
}

void SphereLodSceneNode::initContext(void*) {
  initialized = true;

  lodLists[0] = buildSphereList(1.0, 32, 32);
  lodPixelsSqr[0] = 80.0f * 80.0f;
  listTriangleCount[0] = calcTriCount(32, 32);

  lodLists[1] = buildSphereList(1.0, 16, 16);
  lodPixelsSqr[1] = 40.0f * 40.0f;
  listTriangleCount[1] = calcTriCount(16, 16);

  lodLists[2] = buildSphereList(1.0,  8, 8);
  lodPixelsSqr[2] = 20.0f * 20.0f;
  listTriangleCount[2] = calcTriCount(8, 8);

  lodLists[3] = buildSphereList(1.0,  6, 6);
  lodPixelsSqr[3] = 10.0f * 10.0f;
  listTriangleCount[3] = calcTriCount(6, 6);

  lodLists[4] = buildSphereList(1.0,  4, 4);
  lodPixelsSqr[4] = 5.0f * 5.0f;
  listTriangleCount[4] = calcTriCount(4, 4);

  return;
}


void SphereLodSceneNode::init() {
  initialized = false; // no lists yet
  for (int i = 0; i < sphereLods; i++) {
    lodLists[i] = INVALID_GL_LIST_ID;
    lodPixelsSqr[i] = 0.0f;
  }
  return;
}


void SphereLodSceneNode::kill() {
  if (initialized) {
    freeContext(NULL);
    OpenGLGState::unregisterContextInitializer(freeContext, initContext, NULL);
  }
  return;
}


SphereLodSceneNode::SphereLodSceneNode(const fvec3& pos, float _radius)
  : SphereSceneNode(pos, _radius)
  , renderNode(this) {
  if (!initialized) {
    initialized = true;
    initContext(NULL);
    OpenGLGState::registerContextInitializer(freeContext, initContext, NULL);
  }

  inside = false;
  shockWave = false;

  renderNode.setLod(0);

  // adjust the gstate for this type of sphere
  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_BACK);
  builder.setShading(GL_SMOOTH);
  const fvec4 spec(1.0f, 1.0f, 1.0f, 1.0f);
  const fvec4 emis(0.0f, 0.0f, 0.0f, 1.0f);
  OpenGLMaterial glmat(spec, emis, 64.0f);
  builder.setMaterial(glmat, RENDERER.useQuality() > _LOW_QUALITY);
  gstate = builder.getState();
  return;
}


SphereLodSceneNode::~SphereLodSceneNode() {
  return;
}


void SphereLodSceneNode::setShockWave(bool value) {
  shockWave = value;
  if (BZDBCache::texture && false) { //FIXME
    OpenGLGStateBuilder builder(gstate);
    TextureManager& tm = TextureManager::instance();
    int texId = tm.getTextureID("mesh");
    builder.setTexture(texId);
    gstate = builder.getState();
  }
  return;
}


void SphereLodSceneNode::addRenderNodes(SceneRenderer& renderer) {
  const ViewFrustum& view = renderer.getViewFrustum();
  const fvec4& s = getSphere();
  const fvec3& e = view.getEye();
  float distSqr = (e - s.xyz()).lengthSq();
  if (distSqr <= 0.0f) {
    distSqr = 1.0e-6f;
  }

  const float lpp = renderer.getLengthPerPixel();
  float ppl;
  if (lpp <= 0.0f) {
    ppl = +MAXFLOAT;
  }
  else {
    ppl = 1.0f / lpp;
  }
  const float pixelsSqr = (s.w * (ppl * ppl)) / distSqr;

  int lod;
  for (lod = 0; lod < (sphereLods - 1); lod++) {
    if (lodPixelsSqr[lod] < pixelsSqr) {
      break;
    }
  }
  renderNode.setLod(lod);

  inside = (distSqr < s.w);

  renderer.addRenderNode(&renderNode, &gstate);

  return;
}


void SphereLodSceneNode::addShadowNodes(SceneRenderer&) {
  return;
}


//
// SphereLodSceneNode::SphereLodRenderNode
//

SphereLodSceneNode::SphereLodRenderNode::SphereLodRenderNode(
  const SphereLodSceneNode* _sceneNode)
  : sceneNode(_sceneNode) {
  return;
}


SphereLodSceneNode::SphereLodRenderNode::~SphereLodRenderNode() {
  return;
}


void SphereLodSceneNode::SphereLodRenderNode::setLod(int _lod) {
  lod = _lod;
  return;
}


static inline void drawFullScreenRect() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glRectf(-1.0f, -1.0f, +1.0f, +1.0f);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  return;
}


void SphereLodSceneNode::SphereLodRenderNode::render() {
  const float radius = sceneNode->radius;
  const fvec4& sphere = sceneNode->getSphere();

  static const GLdouble groundPlane[] = { 0.0, 0.0, 1.0, 0.0 };
  glClipPlane(GL_CLIP_PLANE0, groundPlane);
  glEnable(GL_CLIP_PLANE0);

  const GLenum normalizing = GLEW_VERSION_1_2 ? GL_RESCALE_NORMAL
                             : GL_NORMALIZE;
  glEnable(normalizing);

  const bool transparent = sceneNode->transparent;
  const bool stippled = transparent && !BZDBCache::blend;

  const GLuint list = SphereLodSceneNode::lodLists[lod];

  glPushMatrix();
  {
    glTranslatef(sphere.x, sphere.y, sphere.z);
    glScalef(radius, radius, radius);

    // invert the color within contained volume
    if (sceneNode->shockWave) {
      if (transparent) {
        if (BZDBCache::blend) {
          glDisable(GL_BLEND);
        }
        else {
          myStipple(1.0f);
        }
      }
      glDisable(GL_LIGHTING);

      glLogicOp(GL_INVERT);
      glEnable(GL_COLOR_LOGIC_OP);
      {
        glCullFace(GL_FRONT);
        glCallList(list);
        addTriangleCount(listTriangleCount[lod]);
        glCullFace(GL_BACK);
        if (!sceneNode->inside) {
          glCallList(list);
          addTriangleCount(listTriangleCount[lod]);
        }
        else {
          drawFullScreenRect();
          addTriangleCount(2);
        }
      }
      glDisable(GL_COLOR_LOGIC_OP);

      if (transparent) {
        if (BZDBCache::blend) {
          glEnable(GL_BLEND);
        }
        else {
          myStipple(0.5f);
        }
      }
      glEnable(GL_LIGHTING);
    }

    // draw the surface
    myColor4fv(sceneNode->color);
    if (stippled) {
      myStipple(sceneNode->color.a);
    }
    if (!stippled) {
      glCullFace(GL_FRONT);
      glCallList(list);
      addTriangleCount(listTriangleCount[lod]);
    }
    glCullFace(GL_BACK);
    if (!sceneNode->inside) {
      glCallList(list);
      addTriangleCount(listTriangleCount[lod]);
    }
    else {
      glDisable(GL_LIGHTING);
      drawFullScreenRect();
      glEnable(GL_LIGHTING);
      addTriangleCount(2);
    }
    if (stippled) {
      myStipple(0.5f);
    }
  }
  glPopMatrix();

  glDisable(normalizing);

  glDisable(GL_CLIP_PLANE0);

  return;
}


//============================================================================//

//
// SphereBspSceneNode
//

const int NumSlices = 2 * SphereRes;
const int NumParts  = SphereLowRes * SphereLowRes;

SphereBspSceneNode::SphereBspSceneNode(const fvec3& pos, float _radius)
  : SphereSceneNode(pos, _radius)
  , renderNode(this)
  , parts(NULL) {
}


SphereBspSceneNode::~SphereBspSceneNode() {
  if (parts) {
    for (int i = 0; i < NumParts; i++) {
      delete parts[i];
    }
    delete[] parts;
  }
}


SceneNode** SphereBspSceneNode::getParts(int& numParts) {
  if (!parts) {
    // make parts -- always use low detail sphere (if your zbuffer is
    // slow, then you probably don't want to render lots o' polygons)
    parts = new SphereFragmentSceneNode*[NumParts];
    for (int i = 0; i < SphereLowRes; i++) {
      for (int j = 0; j < SphereLowRes; j++) {
        parts[SphereLowRes * i + j] = new SphereFragmentSceneNode(j, i, this);
      }
    }
  }

  // choose number of parts to cut off bottom at around ground level
  int i;
  const fvec4& mySphere = getSphere();
  for (i = 0; i < SphereLowRes; i++) {
    const float geomZ = SphereBspRenderNode::lgeom[SphereLowRes * i].z;
    if (((radius * geomZ) + mySphere.z) < 0.01f) {
      break;
    }
  }
  numParts = SphereLowRes * i;

  return (SceneNode**)parts;
}


void SphereBspSceneNode::addRenderNodes(SceneRenderer& renderer) {
  const fvec4& mySphere = getSphere();
  const ViewFrustum& view = renderer.getViewFrustum();
  const float size = mySphere.w *
                     view.getAreaFactor() / getDistanceSq(view.getEye());
  const int lod = (size < 100.0f) ? 0 : 1;

  renderNode.setHighResolution(lod != 0);

  if (BZDBCache::blend) {
    const fvec3& eye = view.getEye();
    const float azimuth = atan2f(mySphere.y - eye.y, eye.x - mySphere.x);
    const int numSlices = (lod == 1) ? NumSlices : SphereLowRes;
    renderNode.setBaseIndex(int(float(numSlices) *
                                (1.0f + 0.5f * azimuth / M_PI)) % numSlices);
  }

  renderer.addRenderNode(&renderNode, &gstate);
}


void SphereBspSceneNode::addShadowNodes(SceneRenderer& /*renderer*/) {
  return;
  /*
    renderNode.setHighResolution(false);
    renderNode.setBaseIndex(0);
    renderer.addShadowNode(&renderNode);
  */
}


//
// SphereBspSceneNode::SphereBspRenderNode
//

fvec3 SphereBspSceneNode::SphereBspRenderNode::geom[NumSlices * (SphereRes + 1)];

fvec3 SphereBspSceneNode::SphereBspRenderNode::lgeom[SphereLowRes * (SphereLowRes + 1)];

SphereBspSceneNode::SphereBspRenderNode::SphereBspRenderNode(
  const SphereBspSceneNode* _sceneNode)
  : sceneNode(_sceneNode)
  , highResolution(false)
  , baseIndex(0) {
  // initialize geometry if first instance
  static bool init = false;
  if (!init) {
    init = true;

    // high resolution sphere
    int i, j;
    for (i = 0; i <= SphereRes; i++) {
      const float phi = (float)(M_PI * (0.5f - double(i) / SphereRes));
      for (j = 0; j < NumSlices; j++) {
        const float theta = (float)(2.0 * M_PI * double(j) / NumSlices);
        geom[NumSlices * i + j].x = cosf(theta) * cosf(phi);
        geom[NumSlices * i + j].y = sinf(theta) * cosf(phi);
        geom[NumSlices * i + j].z = sinf(phi);
      }
    }

    // low resolution sphere
    for (i = 0; i <= SphereLowRes; i++) {
      const float phi = (float)(M_PI * (0.5 - double(i) / SphereLowRes));
      for (j = 0; j < SphereLowRes; j++) {
        const float theta = (float)(2.0 * M_PI * double(j) / SphereLowRes);
        lgeom[SphereLowRes * i + j].x = cosf(theta) * cosf(phi);
        lgeom[SphereLowRes * i + j].y = sinf(theta) * cosf(phi);
        lgeom[SphereLowRes * i + j].z = sinf(phi);
      }
    }
  }
}


SphereBspSceneNode::SphereBspRenderNode::~SphereBspRenderNode() {
  // do nothing
}


void SphereBspSceneNode::SphereBspRenderNode::setHighResolution(bool value) {
  highResolution = value;
}


void SphereBspSceneNode::SphereBspRenderNode:: setBaseIndex(int value) {
  baseIndex = value;
}


void SphereBspSceneNode::SphereBspRenderNode::render() {
  static const GLdouble groundPlane[] = { 0.0, 0.0, 1.0, 0.0 };

  int i, j;
  const float radius = sceneNode->radius;
  const fvec4& sphere = sceneNode->getSphere();

  glClipPlane(GL_CLIP_PLANE0, groundPlane);
  glEnable(GL_CLIP_PLANE0);

  glPushMatrix();
  {
    glTranslatef(sphere.x, sphere.y, sphere.z);
    glScalef(radius, radius, radius);

    myColor4fv(sceneNode->color);
    if (!BZDBCache::blend && sceneNode->transparent) {
      myStipple(sceneNode->color.a);
    }

    if (BZDBCache::lighting) {
      glEnable(GL_RESCALE_NORMAL);
      // draw with normals (normal is same as vertex!
      // one of the handy properties of a sphere.)
      if (highResolution) {
        for (i = 0; i < SphereRes; i++) {
          glBegin(GL_QUAD_STRIP);
          for (j = baseIndex; j < NumSlices; j++) {
            glNormal3fv(geom[NumSlices * i + j]);
            glVertex3fv(geom[NumSlices * i + j]);
            glNormal3fv(geom[NumSlices * i + j + NumSlices]);
            glVertex3fv(geom[NumSlices * i + j + NumSlices]);
          }
          for (j = 0; j <= baseIndex; j++) {
            glNormal3fv(geom[NumSlices * i + j]);
            glVertex3fv(geom[NumSlices * i + j]);
            glNormal3fv(geom[NumSlices * i + j + NumSlices]);
            glVertex3fv(geom[NumSlices * i + j + NumSlices]);
          }
          glEnd();
        }
        addTriangleCount(SphereRes * NumSlices * 2);
      }
      else {
        for (i = 0; i < SphereLowRes; i++) {
          glBegin(GL_QUAD_STRIP);
          for (j = baseIndex; j < SphereLowRes; j++) {
            glNormal3fv(lgeom[SphereLowRes * i + j]);
            glVertex3fv(lgeom[SphereLowRes * i + j]);
            glNormal3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
            glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
          }
          for (j = 0; j <= baseIndex; j++) {
            glNormal3fv(lgeom[SphereLowRes * i + j]);
            glVertex3fv(lgeom[SphereLowRes * i + j]);
            glNormal3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
            glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
          }
          glEnd();
        }
        addTriangleCount(SphereLowRes * SphereLowRes * 2);
      }
      glDisable(GL_RESCALE_NORMAL);
    }
    else {
      // draw without normals
      if (highResolution) {
        for (i = 0; i < SphereRes; i++) {
          glBegin(GL_QUAD_STRIP);
          for (j = baseIndex; j < NumSlices; j++) {
            glVertex3fv(geom[NumSlices * i + j]);
            glVertex3fv(geom[NumSlices * i + j + NumSlices]);
          }
          for (j = 0; j <= baseIndex; j++) {
            glVertex3fv(geom[NumSlices * i + j]);
            glVertex3fv(geom[NumSlices * i + j + NumSlices]);
          }
          glEnd();
        }
        addTriangleCount(SphereRes * NumSlices * 2);
      }
      else {
        for (i = 0; i < SphereLowRes; i++) {
          glBegin(GL_QUAD_STRIP);
          for (j = baseIndex; j < SphereLowRes; j++) {
            glVertex3fv(lgeom[SphereLowRes * i + j]);
            glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
          }
          for (j = 0; j <= baseIndex; j++) {
            glVertex3fv(lgeom[SphereLowRes * i + j]);
            glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
          }
          glEnd();
        }
        addTriangleCount(SphereLowRes * SphereLowRes * 2);
      }
    }

    if (!BZDBCache::blend && sceneNode->transparent) {
      myStipple(0.5f);
    }
  }
  glPopMatrix();

  glDisable(GL_CLIP_PLANE0);
}


//
// SphereFragmentSceneNode
//

SphereFragmentSceneNode::SphereFragmentSceneNode(int _theta, int _phi,
                                                 SphereBspSceneNode* _parentSphere)
  : parentSphere(_parentSphere)
  , renderNode(_parentSphere, _theta, _phi) {
  // position sphere fragment
  move();
}


SphereFragmentSceneNode::~SphereFragmentSceneNode() {
  // do nothing
}


void SphereFragmentSceneNode::move() {
  const fvec4& pSphere = parentSphere->getSphere();
  const float pRadius = parentSphere->getRadius();
  const fvec3& vertex = renderNode.getVertex();
  const fvec3 center = pSphere.xyz() + (pRadius * vertex);
  setCenter(center.x, center.y, center.z);
  setRadius((float)(4.0 * M_PI * M_PI * pSphere.w) /
            (float)(SphereLowRes * SphereLowRes));
}


void SphereFragmentSceneNode::addRenderNodes(SceneRenderer& renderer) {
  renderer.addRenderNode(&renderNode, &parentSphere->gstate);
}


void SphereFragmentSceneNode::addShadowNodes(SceneRenderer& /*renderer*/) {
  return;
  /*
    renderer.addShadowNode(&renderNode);
  */
}

//
// SphereFragmentSceneNode::FragmentRenderNode
//

SphereFragmentSceneNode::FragmentRenderNode::FragmentRenderNode(
  const SphereBspSceneNode* _sceneNode,
  int _theta, int _phi)
  : sceneNode(_sceneNode)
  , theta(_theta)
  , phi(_phi) {
  // compute incremented theta and phi
  theta2 = (theta + 1) % SphereLowRes;
  phi2 = phi + 1;
}


SphereFragmentSceneNode::FragmentRenderNode::~FragmentRenderNode() {
  // do nothing
}


const fvec3& SphereFragmentSceneNode::FragmentRenderNode::getVertex() const {
  return SphereBspSceneNode::SphereBspRenderNode::lgeom[phi * SphereLowRes + theta];
}


const fvec3& SphereFragmentSceneNode::FragmentRenderNode::getPosition() const {
  return sceneNode->getCenter();
}


void SphereFragmentSceneNode::FragmentRenderNode::render() {
  const float pRadius = sceneNode->getRadius();
  const fvec4& pSphere = sceneNode->getSphere();

  glPushMatrix();
  {
    glTranslatef(pSphere.x, pSphere.y, pSphere.z);
    glScalef(pRadius, pRadius, pRadius);

    myColor4fv(sceneNode->color);
    if (!BZDBCache::blend && sceneNode->transparent) {
      myStipple(sceneNode->color.a);
    }

    glBegin(GL_QUADS);
    {
      const fvec3* lgeom = SphereBspSceneNode::SphereBspRenderNode::lgeom;
      if (BZDBCache::lighting) {
        glNormal3fv(lgeom[SphereLowRes * phi  + theta]);
        glVertex3fv(lgeom[SphereLowRes * phi  + theta]);
        glNormal3fv(lgeom[SphereLowRes * phi2 + theta]);
        glVertex3fv(lgeom[SphereLowRes * phi2 + theta]);
        glNormal3fv(lgeom[SphereLowRes * phi2 + theta2]);
        glVertex3fv(lgeom[SphereLowRes * phi2 + theta2]);
        glNormal3fv(lgeom[SphereLowRes * phi  + theta2]);
        glVertex3fv(lgeom[SphereLowRes * phi  + theta2]);
        addTriangleCount(2);
      }
      else {
        glVertex3fv(lgeom[SphereLowRes * phi  + theta]);
        glVertex3fv(lgeom[SphereLowRes * phi2 + theta]);
        glVertex3fv(lgeom[SphereLowRes * phi2 + theta2]);
        glVertex3fv(lgeom[SphereLowRes * phi  + theta2]);
        addTriangleCount(2);
      }
    }
    glEnd(); // 4 verts -> 2 tris
  }
  glPopMatrix();

  return;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
