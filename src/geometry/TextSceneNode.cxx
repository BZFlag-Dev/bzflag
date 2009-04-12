/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "TextSceneNode.h"

// system headers
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common implementation headers
#include "BZDBCache.h"
#include "BzMaterial.h"
#include "CacheManager.h"
#include "DynamicColor.h"
#include "FontManager.h"
#include "Intersect.h"
#include "MeshTransform.h"
#include "OpenGLMaterial.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cxx is in src/bzflag)
#include "StateDatabase.h"
#include "TextureManager.h"
#include "TextUtils.h"
#include "WorldText.h"
#include "bzfio.h" // for debugLevel

// local implementation headers
#include "ViewFrustum.h"


static const GLbitfield FTGL_MISSING_ATTRIBS = GL_TEXTURE_BIT;


//============================================================================//
//============================================================================//
//
//  static callbacks
//

void TextSceneNode::TextRenderNode::bzdbCallback(const string& name, void* data)
{
  const string value = BZDB.get(name);
  ((TextRenderNode*)data)->setRawText(value);
}


void TextSceneNode::TextRenderNode::initContext(void* data)
{
  ((TextRenderNode*)data)->initXFormList();
}


void TextSceneNode::TextRenderNode::freeContext(void* data)
{
  ((TextRenderNode*)data)->freeXFormList();
}


//============================================================================//
//============================================================================//
//
//  TextSceneNode
//

TextSceneNode::TextSceneNode(const WorldText* _text)
: renderNode(this, _text)
{
  calcPlane();

  // NOTE: TextRenderNode() constructor calls calcSphere() and calcExtents()

  notifyStyleChange();
}


TextSceneNode::~TextSceneNode()
{
}


bool TextSceneNode::inAxisBox(const Extents& exts) const
{
  fvec3 points[5];
  getPoints(points);
  return Intersect::testPolygonInAxisBox(4, points, plane, exts)
         != Intersect::Outside;
}


//============================================================================//

void TextSceneNode::calcPlane()
{
  const WorldText& text = renderNode.text;

  if (!text.bzMaterial->getNoCulling()) { // FIXME ? || text.billboard) {
    noPlane = false;
  }

  MeshTransform::Tool xformTool(text.xform);
  fvec3 origin(0.0f, 0.0f, 0.0f);
  fvec3 normal(0.0f, 0.0f, 1.0f);
  xformTool.modifyVertex(origin);
  xformTool.modifyNormal(normal);

  plane[0] = normal[0];
  plane[1] = normal[1];
  plane[2] = normal[2];
  plane[3] = -((normal[0] * origin[0]) +
               (normal[1] * origin[1]) +
               (normal[2] * origin[2]));
}


void TextSceneNode::calcSphere(const fvec3 points[5])
{
  const fvec3& orig  = points[4];
  const float radius = getMaxDist(points);
  fvec4 tmpSphere(orig, (radius * radius));

  setSphere(tmpSphere);
}



void TextSceneNode::calcExtents(const fvec3 points[5])
{
  const WorldText& text = renderNode.text;

  extents.reset();

  if (!text.billboard) {
    for (int i = 0; i < 4; i++) {
      extents.expandToPoint(points[i]);
    }
  }
  else {
    const float dist = getMaxDist(points);
    const fvec3& orig = points[4];
    extents.set(orig, orig);
    extents.addMargin(dist);
  }
}


void TextSceneNode::getPoints(fvec3 points[5]) const
{
  const WorldText& text = renderNode.text;

  float yp = 0.0f;
  float yn = 0.0f;
  if (!renderNode.lines.empty()) {
    const float ySpan = (float)(renderNode.lines.size() - 1) *
                        (renderNode.lineStep / renderNode.fontSize);
    if (ySpan < 0.0f) {
      yp = +1.5f;
      yn = ySpan - 0.5f;
    } else {
      yp = +1.5f + ySpan;
      yn = -0.5f;
    }
  }

  float xn = +1.0e20f;
  float xp = -1.0e20f;
  const vector<float>& widths = renderNode.widths;
  if (renderNode.fixedWidth > 0.0f) {
    xn = renderNode.fixedWidth * -text.justify;
    xp = xn + renderNode.fixedWidth;
  }
  else {
    for (size_t i = 0; i < widths.size(); i++) {
      float n = widths[i] * -text.justify;
      float p = n + widths[i];
      if (xn > n) { xn = n; }
      if (xp < p) { xp = p; }
    }
  }

  // adjust for fontSize, and add some margin
  xp = (xp / renderNode.fontSize) + 0.5f;
  xn = (xn / renderNode.fontSize) - 0.5f;

  points[0][0] = xn;   points[0][1] = yn;   points[0][2] = 0.0f;
  points[1][0] = xp;   points[1][1] = yn;   points[1][2] = 0.0f;
  points[2][0] = xp;   points[2][1] = yp;   points[2][2] = 0.0f;
  points[3][0] = xn;   points[3][1] = yp;   points[3][2] = 0.0f;
  points[4][0] = 0.0f; points[4][1] = 0.0f; points[4][2] = 0.0f; // origin

  MeshTransform xform = text.xform;
  MeshTransform::Tool xformTool(xform);
  for (int i = 0; i < 5; i++) {
    xformTool.modifyVertex(points[i]);
  }
}


float TextSceneNode::getMaxDist(const fvec3 points[5]) const
{
  float maxDistSqr = 0.0f;
  for (int i = 0; i < 4; i++) {
    const float dx = (points[4][0] - points[i][0]);
    const float dy = (points[4][1] - points[i][1]);
    const float dz = (points[4][2] - points[i][2]);
    const float distSqr = (dx * dx) + (dy * dy) + (dz * dz);
    if (maxDistSqr < distSqr) {
      maxDistSqr = distSqr;
    }
  }
  return sqrtf(maxDistSqr);
}


//============================================================================//

void TextSceneNode::notifyStyleChange()
{
  const WorldText& text = renderNode.text;

  OpenGLGStateBuilder builder;

  const BzMaterial* bzmat = text.bzMaterial;

  builder.setOrder(bzmat->getOrder());

  // blending
  builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // alpha thresholding
  if (bzmat->getAlphaThreshold() != 0.0f) {
    builder.setAlphaFunc(GL_GEQUAL, bzmat->getAlphaThreshold());
  }

  // culling
  if (bzmat->getNoCulling()) {
    builder.setCulling(GL_NONE);
  }

  // sorting
  if (bzmat->getNoSorting()) {
    builder.setNeedsSorting(false);
  }

  // lighting
  if (BZDBCache::lighting && !bzmat->getNoLighting()) {
    OpenGLMaterial oglMaterial(bzmat->getSpecular(),
                               bzmat->getEmission(),
                               bzmat->getShininess());
    builder.setMaterial(oglMaterial, RENDERER.useQuality() > _LOW_QUALITY);
  } else {
    builder.enableMaterial(false);
  }
  builder.setShading(GL_FLAT);

  // smoothing
  builder.setSmoothing(BZDBCache::smooth);

  gstate = builder.getState();
}


//============================================================================//

bool TextSceneNode::cull(const ViewFrustum& frustum) const
{
  const WorldText& text = renderNode.text;
  // see if our eye is behind the plane
  if (!text.billboard && !text.bzMaterial->getNoCulling()) {
    const fvec3& eye = frustum.getEye();
    if ((fvec3::dot(eye, plane.xyz()) + plane.w) <= 0.0f) {
      return false;
    }
  }

  // now do an extents check
  const Frustum* frustumPtr = (const Frustum *) &frustum;
  return (Intersect::testAxisBoxInFrustum(extents, frustumPtr) == Intersect::Outside);
}


bool TextSceneNode::cullShadow(int planeCount, const fvec4* planes) const
{
  // FIXME -- use extents?
  const fvec4& s = getSphere();
  for (int i = 0; i < planeCount; i++) {
    const fvec4& p = planes[i];
    const float d = fvec3::dot(p.xyz(), s.xyz()) + p.w;
    if ((d < 0.0f) && ((d * d) > s.w)) {
      return true;
    }
  }
  return false;
}


//============================================================================//

void TextSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}


void TextSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  renderer.addShadowNode(&renderNode);
}


void TextSceneNode::renderRadar()
{
  renderNode.renderRadar();
}


//============================================================================//
//============================================================================//
//
//  static text routines
//

static void breakLines(const string& source, vector<string>& lines)
{
  lines.clear();
  const char* c = source.c_str();
  const char* s = c;
  while (true) {
    if ((*c == 0) || (*c == '\n')) {
      lines.push_back(string(s, c - s));
      if (*c == 0) {
        break;
      }
      c++;
      s = c;
    } else {
      c++;
    }
  }
}


//============================================================================//
//============================================================================//
//
//  TextRenderNode
//

TextSceneNode::TextRenderNode::TextRenderNode(TextSceneNode* _sceneNode,
                                              const WorldText* _text)
: sceneNode(_sceneNode)
, text(*_text)
, xformList(INVALID_GL_LIST_ID)
, fontID(-1)
, fontSize(text.fontSize)
, fixedWidth(0.0f)
, noRadar(text.bzMaterial->getNoRadar())
, noShadow(text.bzMaterial->getNoShadow())
, triangles(0)
{
  FontManager &fm = FontManager::instance();

  linesPtr = &lines;

  const float maxFontSize = BZDB.eval("maxFontSize");
  if (fontSize > maxFontSize) {
    fontSize = maxFontSize;
  }

  const BzMaterial* bzmat = text.bzMaterial;
  const DynamicColor* dyncol = DYNCOLORMGR.getColor(bzmat->getDynamicColor());
  if (dyncol != NULL) {
    colorPtr = dyncol->getColor();
  } else {
    colorPtr = bzmat->getDiffuse();
  }

  fontID = getFontID();

  lineStep = -text.lineSpace * fm.getStringHeight(fontID, fontSize);

  usePolygonOffset  = (text.poFactor != 0.0f) || (text.poUnits != 0.0f);

  useLengthPerPixel = (text.lengthPerPixel > 0.0f);

  setRawText(text.useBZDB ? BZDB.get(text.data) : text.data);

  if (text.useBZDB) {
    BZDB.addCallback(text.data, bzdbCallback, this);
  }

  OpenGLGState::registerContextInitializer(freeContext, initContext, this);
}


TextSceneNode::TextRenderNode::~TextRenderNode()
{
  if (CacheManager::isCacheFileType(text.font)) {
    FontManager &fm = FontManager::instance();
    const string localName = CACHEMGR.getLocalName(text.font);
    fm.freeFontFile(localName);
  }

  if (text.useBZDB) {
    BZDB.removeCallback(text.data, bzdbCallback, this);
  }
  freeXFormList();
  OpenGLGState::unregisterContextInitializer(freeContext, initContext, this);
}


//============================================================================//

int TextSceneNode::TextRenderNode::getFontID() const
{
  FontManager &fm = FontManager::instance();

  int id = -1;
  if (!CacheManager::isCacheFileType(text.font)) {
    id = fm.getFaceID(text.font);
  }
  else {
    // try the URL cache
    const string localName = CACHEMGR.getLocalName(text.font);
    id = fm.lookupFileID(localName);
    if (id < 0) {
      id = fm.load(localName.c_str());
    }
  }
  // backup plan
  if (id < 0) {
    id = fm.getFaceID("junkName"); // get the default face
  }

  return id;
}


//============================================================================//

void TextSceneNode::TextRenderNode::singleLineXForm() const
{
  if (text.fixedWidth > 0.0f) {
    const float newWidth = (text.fixedWidth * fontSize);
    glScalef(newWidth / widths[0], 1.0f, 1.0f);
  }
  if (text.justify != 0.0f) {
    glTranslatef(-text.justify * widths[0], 0.0f, 0.0f);
  }
}


void TextSceneNode::TextRenderNode::initXFormList()
{
  MeshTransform::Tool xformTool(text.xform);
  GLenum error;

  int errCount = 0;
  // reset the error state
  while (true) {
    error = glGetError();
    if (error == GL_NO_ERROR) {
      break;
    }
    errCount++; // avoid a possible spin-lock?
    if (errCount > 666) {
      logDebugMessage(0,
        "ERROR: TextSceneNode::initXFormList() glError: %i\n", error);
      return; // don't make the list, something is borked
    }
  };

  // oops, transpose
  GLfloat matrix[16];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      matrix[(i*4)+j] = xformTool.getMatrix()[(j*4)+i];
    }
  }

  xformList = glGenLists(1);
  glNewList(xformList, GL_COMPILE);
  {
    const float invSize = (1.0f / fontSize);
    glMultMatrixf(matrix);
    glScalef(invSize, invSize, invSize);
    if ((lines.size() == 1) && !text.billboard) {
      singleLineXForm();
    }
  }
  glEndList();

  error = glGetError();
  if (error != GL_NO_ERROR) {
    logDebugMessage(0,
      "ERROR: TextSceneNode::initXFormList() failed: %i\n", error);
    xformList = INVALID_GL_LIST_ID;
  }
}


void TextSceneNode::TextRenderNode::freeXFormList()
{
  glDeleteLists(xformList, 1);
  xformList = INVALID_GL_LIST_ID;
}


//============================================================================//

void TextSceneNode::TextRenderNode::setRawText(const string& rawText)
{
  breakLines(TextUtils::unescape_colors(rawText), lines);

  stripped.clear();
  for (size_t i = 0; i < lines.size(); i++) {
    stripped.push_back(stripAnsiCodes(lines[i].c_str()));
  }

  widths.clear();
  FontManager &fm = FontManager::instance();
  for (size_t i = 0; i < lines.size(); i++) {
    widths.push_back(fm.getStringWidth(fontID, fontSize, lines[i].c_str()));
  }

  fixedWidth = 0.0f;
  if (text.fixedWidth != 0.0f) {
    if (text.fixedWidth > 0.0f) {
      fixedWidth = text.fixedWidth * fontSize;
    }
    else {
      for (size_t i = 0; i < widths.size(); i++) {
        if (fixedWidth < widths[i]) {
          fixedWidth = widths[i];
        }
      }
    }
  }

  fvec3 points[5];
  sceneNode->getPoints(points);
  sceneNode->calcSphere(points);
  sceneNode->calcExtents(points);

  countTriangles();

  freeXFormList();
  initXFormList();
}


void TextSceneNode::TextRenderNode::countTriangles()
{
  triangles = 0;
  for (size_t i = 0; i < stripped.size(); i++) {
    const string& line = stripped[i];
    for (size_t c = 0; c < line.size(); c++) {
      if (!isspace(line[c])) {
        triangles += 2;
      }
    }
  }
}


//============================================================================//

inline bool TextSceneNode::TextRenderNode::checkDist() const
{
  if (!useLengthPerPixel) {
    return true;
  }
  const SceneRenderer& renderer = RENDERER;
  const ViewFrustum&   frustum  = renderer.getViewFrustum();
  const fvec3& pos = sceneNode->getSphere().xyz();
  const fvec3& eye = frustum.getEye();
  const fvec3& dir = frustum.getDirection();
  const float dist = fvec3::dot(dir, pos - eye);
  const float lpp = dist * renderer.getLengthPerPixel();

  return (lpp < text.lengthPerPixel);
}


//============================================================================//


static bool wantCheckDist = true;


void TextSceneNode::TextRenderNode::render()
{
  if (wantCheckDist && !checkDist()) {
    return;
  }

#ifdef DEBUG_RENDERING
  if (debugLevel > 0) {
    drawDebug();
  }
#endif

  FontManager& fm = FontManager::instance();

  glPushAttrib(FTGL_MISSING_ATTRIBS);
  glPushMatrix();

  glNormal3f(0.0f, 0.0f, 1.0f);

  myColor4fv(colorPtr);
  const float oldOpacity = fm.getOpacity(); // FIXME -- just do it once at the start
  if (colorPtr[3] != oldOpacity) {
    fm.setOpacity(colorPtr[3]);
  }

  glCallList(xformList);

  if (text.billboard) {
    RENDERER.getViewFrustum().executeBillboard();
  }

  if (usePolygonOffset) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(text.poFactor, text.poUnits);
  }

  const vector<string>& currLines = *linesPtr;

  if (currLines.size() == 1) {
    // all transformations are compiled into xformList
    if (text.billboard) {
      singleLineXForm();
    }
    fm.drawString(0.0f, 0.0f, 0.0f, fontID, fontSize,
                  currLines[0].c_str(), colorPtr, AlignLeft);
  }
  else {
    for (size_t i = 0; i < currLines.size(); i++) {
      const float width = widths[i];
      if (fixedWidth > 0.0f) {
        if (width == 0.0f) {
          continue;
        }
        float offx = 0.0f;
        if (text.justify != 0.0f) {
          offx = -text.justify * width;
        }
        glPushMatrix();
        glScalef(fixedWidth / width, 1.0f, 1.0f);
        fm.drawString(offx, 0.0f, 0.0f, fontID, fontSize,
                      currLines[i].c_str(), colorPtr, AlignLeft);
        glPopMatrix();
      }
      else {
        float offx = 0.0f;
        if (text.justify != 0.0f) {
          offx = -text.justify * width;
        }
        fm.drawString(offx, 0.0f, 0.0f, fontID, fontSize,
                      currLines[i].c_str(), colorPtr, AlignLeft);
      }
      glTranslatef(0.0f, lineStep, 0.0f);
    }
  }

  if (usePolygonOffset) {
    glDisable(GL_POLYGON_OFFSET_FILL);
  }

  fm.setOpacity(oldOpacity);

  glPopMatrix();
  glPopAttrib();

  addTriangleCount(triangles);
}


void TextSceneNode::TextRenderNode::renderRadar()
{
  return; // FIXME -- text does not render to radar, FIXME?  ;-)
          //       -- fast mode requires 2 textures?
          //       -- non-textured radar modes would need new code
  if (noRadar) {
    return;
  }
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_TEXTURE_GEN_S);
  glPushMatrix();
  glScalef(1.0f, 1.0f, 0.0f);
  render();
  glPopMatrix();
  glPopAttrib();
}


void TextSceneNode::TextRenderNode::renderShadow()
{
  if (noShadow) {
    return;
  }
  FontManager &fm = FontManager::instance();

  wantCheckDist = false;

  fm.setRawBlending(true);

  static fvec4 shadowColor(0.0f, 0.0f, 0.0f, 1.0f);
  const float* oldColor = colorPtr;
  colorPtr = shadowColor;

  linesPtr = &stripped;

  if (!BZDBCache::stencilShadows) {
    shadowColor[3] = 1.0f;
    render();
  }
  else {
    shadowColor[3] = BZDBCache::shadowAlpha;
    glAlphaFunc(GL_GEQUAL, 0.1f);
    glEnable(GL_ALPHA_TEST);
    render();
    glDisable(GL_ALPHA_TEST);
  }

  linesPtr = &lines;

  colorPtr = oldColor;

  fm.setRawBlending(false);

  wantCheckDist = true;
}


void TextSceneNode::TextRenderNode::drawDebug()
{
  myColor4fv(colorPtr);

  fvec3 points[5];
  sceneNode->getPoints(points);

  glPointSize(3.0f);
  glLineWidth(3.0f);

  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < 4; i++) {
    glVertex3fv(points[i]);
  }
  glEnd();

  glBegin(GL_POINTS);
    glVertex3fv(points[4]);
  glEnd();

  glLineWidth(1.0f);
  glPointSize(1.0f);
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
