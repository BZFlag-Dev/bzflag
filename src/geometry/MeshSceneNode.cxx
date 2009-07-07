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
#include "MeshSceneNode.h"

// system headers
#include <math.h>
#include <string.h>
#include <stdlib.h>

// common headers
#include "vectors.h"
#include "Extents.h"
#include "Intersect.h"
#include "TimeKeeper.h"

#include "MeshFace.h"
#include "MeshObstacle.h"
#include "MeshDrawInfo.h"
#include "MeshRenderNode.h"
#include "SceneNode.h"

#include "BzMaterial.h"
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "ViewFrustum.h"
#include "SceneRenderer.h"
#include "TextureManager.h"

#include "StateDatabase.h"
#include "BZDBCache.h"

//
// NOTE
//
//   This type of SceneNode does not support
//   tesselation or splitting of translucent
//   nodes for improved back-to-front sorting.
//


float MeshSceneNode::LodScale = 1.0f;
float MeshSceneNode::RadarLodScale = 1.0f;

MeshSceneNode::MeshSceneNode(const MeshObstacle* _mesh)
{
  mesh = _mesh;

  lodCount = 0;
  lods = NULL;

  xformList = INVALID_GL_LIST_ID;

  if (mesh == NULL) {
    return;
  }

  drawInfo = mesh->getDrawInfo();
  if (drawInfo == NULL) {
    return;
  }

  drawMgr = drawInfo->getDrawMgr();
  if (drawMgr == NULL) {
    return;
  }

  // disable the plane
  noPlane = true;
  plane.x = plane.y = plane.z = plane.w = 0.0f;

  // setup the extents, sphere, and lengthPerPixel adjustment
  float lengthAdj = 1.0f;
  const Extents& diExts = drawInfo->getExtents();
  const MeshTransform::Tool* xformTool = drawInfo->getTransformTool();
  if (xformTool == NULL) {
    extents = drawInfo->getExtents();
    setSphere(drawInfo->getSphere());
  } else {
    // sloppy way to recalcuate the transformed extents
    fvec3 c[8];
    c[0].x = c[6].x = c[5].x = c[3].x = diExts.mins.x;
    c[7].x = c[1].x = c[2].x = c[4].x = diExts.maxs.x;
    c[0].y = c[1].y = c[5].y = c[4].y = diExts.mins.y;
    c[7].y = c[6].y = c[2].y = c[3].y = diExts.maxs.y;
    c[0].z = c[1].z = c[2].z = c[3].z = diExts.mins.z;
    c[7].z = c[6].z = c[5].z = c[4].z = diExts.maxs.z;
    extents.reset();
    for (int v = 0; v < 8; v++) {
      xformTool->modifyVertex(c[v]);
      extents.expandToPoint(c[v]);
    }
    // lengthPerPixel adjustment
    lengthAdj = +MAXFLOAT;
    for (int a = 0; a < 3; a++) {
      const float oldWidth = diExts.maxs[a] - diExts.mins[a];
      const fvec3& s = c[0]; // all mins
      // mins, except: c[1] -> max[0], c[3] -> max[1], c[5] -> max[2]
      const fvec3& e = c[(a * 2) + 1];
      const float newWidth = (s - e).length();
      if (oldWidth > 0.0f) {
	const float scale = (newWidth / oldWidth);
	if (scale < lengthAdj) {
	  lengthAdj = scale;
	}
      }
    }
    // adjust the sphere
    fvec4 mySphere(drawInfo->getSphere());
    xformTool->modifyVertex(mySphere.xyz());
    mySphere.w *= (lengthAdj * lengthAdj);
    setSphere(mySphere);
  }

  // setup lod/set nodes
  lods = NULL;
  lodCount = drawInfo->getLodCount();
  lods = new LodNode[lodCount];
  lodLengths = new float[lodCount];
  const DrawLod* drawLods = drawInfo->getDrawLods();
  for (int lod = 0; lod < lodCount; lod++) {
    LodNode& lodNode = lods[lod];
    const DrawLod& drawLod = drawLods[lod];
    lodLengths[lod] = drawLod.lengthPerPixel * lengthAdj; // lengthPerPixel
    lodNode.count = drawLods[lod].count;
    lodNode.sets = new SetNode[lodNode.count];
    for (int set = 0; set < lodNode.count; set++) {
      const DrawSet& drawSet = drawLod.sets[set];
      SetNode& setNode = lodNode.sets[set];
      setNode.node = NULL;
      setNode.radarNode = NULL;
      setNode.meshMat.bzmat = convertMaterial(drawSet.material);
    }
  }

  // FIXME - use alternate radar LODs
  radarCount = lodCount;
  radarLengths = new float[lodCount];
  memcpy(radarLengths, lodLengths, radarCount * sizeof(float));
  radarLods = lods;

  // build the transform display list
  makeXFormList();
  OpenGLGState::registerContextInitializer(freeContext, initContext, this);

  // build gstates and render nodes
  notifyStyleChange();

  return;
}


MeshSceneNode::~MeshSceneNode()
{
  //FIXME - delete radar lods  (once they're implemented)
  for (int i = 0; i < lodCount; i++) {
    LodNode& lod = lods[i];
    for (int j = 0; j < lod.count; j++) {
      SetNode& set = lod.sets[j];
      delete set.node;
      delete set.radarNode;
    }
    delete[] lod.sets;
  }
  delete[] lods;

  delete[] lodLengths;
  delete[] radarLengths;

  OpenGLGState::unregisterContextInitializer(freeContext, initContext, this);
  freeXFormList();

  return;
}


inline int MeshSceneNode::calcNormalLod(const ViewFrustum& vf)
{
  const fvec3& eye = vf.getEye();
  const fvec3& pos = getSphere().xyz();
  const fvec3& dir = vf.getDirection();
  const float dist = fvec3::dot(dir, (pos - eye));
  const float lengthPerPixel = dist * LodScale;
  for (int i = (lodCount - 1); i > 0; i--) {
    if (lengthPerPixel > lodLengths[i]) {
      return i;
    }
  }
  return 0;
}


inline int MeshSceneNode::calcShadowLod(const ViewFrustum& vf)
{
  // FIXME: adjust for ray direction
  const fvec3& eye = vf.getEye();
  const fvec3& pos = getSphere().xyz();
  const fvec3& dir = vf.getDirection();
  const float dist = fvec3::dot(dir, (pos - eye));
  const float lengthPerPixel = dist * LodScale;
  for (int i = (lodCount - 1); i > 0; i--) {
    if (lengthPerPixel > lodLengths[i]) {
      return i;
    }
  }
  return 0;
}


inline int MeshSceneNode::calcRadarLod()
{
  for (int i = (radarCount - 1); i > 0; i--) {
    if (radarLengths[i] < RadarLodScale) {
      return i;
    }
  }
  return 0;
}


void MeshSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  const ViewFrustum& vf = renderer.getViewFrustum();
  const int level = (lodCount == 1) ? 0 : calcNormalLod(vf);
  LodNode& lod = lods[level];

  if (animRepos) {
    const MeshTransform::Tool* xformTool = drawInfo->getTransformTool();
    const DrawLod* drawLods = drawInfo->getDrawLods();
    const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
    const float& cos_val = animInfo->cos_val;
    const float& sin_val = animInfo->sin_val;
    for (int i = 0; i < lod.count; i++) {
      SetNode& set = lod.sets[i];
      if (set.meshMat.animRepos) {
	const fvec4& s = drawLods[level].sets[i].sphere;
	fvec3 pos;
	pos.x = (cos_val * s.x) - (sin_val * s.y);
	pos.y = (sin_val * s.x) + (cos_val * s.y);
	pos.z = s.z;
	if (xformTool != NULL) {
	  xformTool->modifyVertex(pos);
	}
	((AlphaGroupRenderNode*)set.node)->setPosition(pos);
      }
    }
  }

  for (int i = 0; i < lod.count; i++) {
    SetNode& set = lod.sets[i];
    if (set.meshMat.colorPtr->a > 0.0f) {
      renderer.addRenderNode(set.node, &set.meshMat.gstate);
    }
  }

  return;
}


void MeshSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  const ViewFrustum& vf = renderer.getViewFrustum();
  const int level = (lodCount == 1) ? 0 : calcShadowLod(vf);
  LodNode& lod = lods[level];
  for (int i = 0; i < lod.count; i++) {
    SetNode& set = lod.sets[i];
    const MeshMaterial& mat = set.meshMat;
    if (mat.drawShadow && (mat.colorPtr->a != 0.0f)) {
      renderer.addShadowNode(set.node);
    }
  }
  return;
}


void MeshSceneNode::renderRadar()
{
  const int level = (lodCount == 1) ? 0 : calcRadarLod();
  LodNode& lod = radarLods[level];
  for (int i = 0; i < lod.count; i++) {
    SetNode& set = lod.sets[i];
    if (set.meshMat.drawRadar) {
      set.radarNode->renderRadar();
    }
  }
  return;
}


bool MeshSceneNode::cull(const ViewFrustum& frustum) const
{
  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  const Frustum* f = (const Frustum *) &frustum;
  if (Intersect::testAxisBoxInFrustum(extents, f) == Intersect::Outside) {
    return true;
  }

  // probably visible
  return false;
}


bool MeshSceneNode::inAxisBox(const Extents& exts) const
{
  // quick and dirty
  return extents.touches(exts);
}


void MeshSceneNode::notifyStyleChange()
{
  const DrawLod* drawLods = drawInfo->getDrawLods();

  animRepos = false;

  // FIXME -- render nodes should be created in the constructor
  //          (for EightDim node viewing)

  for (int lod = 0; lod < lodCount; lod++) {
    LodNode& lodNode = lods[lod];
    for (int set = 0; set < lodNode.count; set++) {
      SetNode& setNode = lodNode.sets[set];
      MeshMaterial& mat = setNode.meshMat;
      const DrawSet& drawSet = drawLods[lod].sets[set];

      delete setNode.node;
      delete setNode.radarNode;

      updateMaterial(&mat);

      // how shall we normalize?
      bool normalize = false;
      const MeshTransform::Tool* xformTool = drawInfo->getTransformTool();
      if (xformTool != NULL) {
	normalize = xformTool->isSkewed();
      }

      // enough elements to warrant disabling lights?
      const Extents* extPtr = &extents;
      if ((drawSet.triangleCount < 100) ||
	  !BZDBCache::lighting || mat.bzmat->getNoLighting()) {
	extPtr = NULL;
      }

      if (!mat.needsSorting) {
	setNode.node =
	  new OpaqueRenderNode(drawMgr, &xformList, normalize,
			       mat.colorPtr, lod, set, extPtr,
			       drawSet.triangleCount);
	mat.animRepos = false;
      } else {
	fvec3 setPos;
	memcpy(setPos, drawSet.sphere, sizeof(fvec3));
	if (xformTool != NULL) {
	  xformTool->modifyVertex(setPos);
	}
	setNode.node =
	  new AlphaGroupRenderNode(drawMgr, &xformList, normalize,
				   mat.colorPtr, lod, set, extPtr, setPos,
				   drawSet.triangleCount);
	if ((fabsf(drawSet.sphere.x) > 0.001f) &&
	    (fabsf(drawSet.sphere.y) > 0.001f) &&
	    (mat.color.a != 0.0f) &&
	    (drawInfo->getAnimationInfo() != NULL)) {
	  animRepos = true;
	  mat.animRepos = true;
	} else {
	  mat.animRepos = false;
	}
      }

      setNode.radarNode =
	new OpaqueRenderNode(drawMgr, &xformList, normalize,
			     mat.colorPtr, lod, set, extPtr,
			     drawSet.triangleCount);
    }
  }
  return;
}


const BzMaterial* MeshSceneNode::convertMaterial(const BzMaterial* bzmat)
{
  const MaterialMap* matMap = drawInfo->getMaterialMap();
  if (matMap == NULL) {
    return bzmat;
  }

  MaterialMap::const_iterator it = matMap->find(bzmat);
  if (it == matMap->end()) {
    return bzmat;
  } else {
    return it->second;
  }
}


void MeshSceneNode::updateMaterial(MeshSceneNode::MeshMaterial* mat)
{
// FIXME - deal with invisibility

  // get the references
  const BzMaterial* bzmat  = mat->bzmat;
  OpenGLGState&     gstate = mat->gstate;
  fvec4&            color  = mat->color;

  OpenGLGStateBuilder builder;
  TextureManager &tm = TextureManager::instance();

  // cheat a little
  ((BzMaterial*)bzmat)->setReference();

  builder.setOrder(bzmat->getOrder());

  // ways of requiring blending
  bool colorAlpha = false;
  bool textureAlpha = false;

  bool useDiffuseColor = true;

  // texturing
  if (BZDBCache::texture) {
    int faceTexture = -1;
    bool userTexture = (bzmat->getTextureCount() > 0);
    if (userTexture) {
      const std::string& texname = bzmat->getTextureLocal(0);
      if (texname.size() > 0) {
	faceTexture = tm.getTextureID(texname);
      }
      if (faceTexture >= 0) {
	useDiffuseColor = bzmat->getUseColorOnTexture(0);
	if (bzmat->getUseTextureAlpha(0)) {
	  const ImageInfo& imageInfo = tm.getInfo(faceTexture);
	  textureAlpha = imageInfo.alpha;
	}
      } else {
	faceTexture = tm.getTextureID("mesh", false /* no failure reports */);
      }
      if (faceTexture >= 0) {
	// texture matrix
	const int texMatId = bzmat->getTextureMatrix(0);
	const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texMatId);
	if (texmat != NULL) {
	  const float* matrix = texmat->getMatrix();
	  if (matrix != NULL) {
	    builder.setTextureMatrix(matrix);
	    builder.enableTextureMatrix(true);
	  }
	}
	// sphere mapping
	if (bzmat->getUseSphereMap(0)) {
	  builder.enableSphereMap(true);
	}
      }
      builder.setTexture(faceTexture);
      builder.enableTexture(true);
    }
  }

  // lighting
  if (BZDBCache::lighting && !bzmat->getNoLighting()) {
    OpenGLMaterial oglMaterial(bzmat->getSpecular(),
			       bzmat->getEmission(),
			       bzmat->getShininess());
    builder.setMaterial(oglMaterial, RENDERER.useQuality() > _LOW_QUALITY);
    builder.setShading(GL_SMOOTH);
  } else {
    builder.setShading(GL_FLAT);
  }

  // color
  if (useDiffuseColor) {
    color = bzmat->getDiffuse();
    colorAlpha = (color.a != 1.0f);
  } else {
    // set it to white, this should only happen when
    // we've gotten a user texture, and there's a
    // request to not use the material's diffuse color.
    color.r = color.g = color.b = color.a = 1.0f;
  }

  // dynamic color
  const DynamicColor* dyncol = DYNCOLORMGR.getColor(bzmat->getDynamicColor());
  if (dyncol != NULL) {
    mat->colorPtr = &dyncol->getColor();
    colorAlpha = dyncol->canHaveAlpha(); // override
  } else {
    mat->colorPtr = &color;
  }

  // blending
  const bool isAlpha = (colorAlpha || textureAlpha);
  if (isAlpha) {
    if (BZDBCache::blend) {
      builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      builder.setStipple(1.0f);
    } else {
      builder.resetBlending();
      if (dyncol != NULL) {
	builder.setStipple(0.5f);
      } else {
	builder.setStipple(color.a);
      }
    }
  }
  if (bzmat->getNoBlending()) {
    builder.resetBlending();
  }

  // sorting  (do this after using setBlending())
  //
  // NOTE:  getGroupAlpha() isn't used because all MeshSceneNode
  //	elements are sorted as groups rather then individually
  //
  mat->needsSorting = (isAlpha && !bzmat->getNoSorting());
  builder.setNeedsSorting(mat->needsSorting);

  // alpha thresholding
  float alphaThreshold = bzmat->getAlphaThreshold();
  if (alphaThreshold != 0.0f) {
    builder.setAlphaFunc(GL_GEQUAL, alphaThreshold);
  }

  // radar and shadows
  mat->drawRadar = !bzmat->getNoRadar();
  mat->drawShadow = !bzmat->getNoShadow();

  // culling
  if (bzmat->getNoCulling()) {
    builder.setCulling(GL_NONE);
  }

  // generate the gstate
  gstate = builder.getState();

  return;
}


void MeshSceneNode::makeXFormList()
{
  GLenum error;
  const MeshTransform::Tool* xformTool = drawInfo->getTransformTool();
  if (xformTool != NULL) {
    int errCount = 0;
    // reset the error state
    while (true) {
      error = glGetError();
      if (error == GL_NO_ERROR) {
	break;
      }
      errCount++; // avoid a possible spin-lock?
      if (errCount > 666) {
	logDebugMessage(0,"ERROR: MeshSceneNode::makeXFormList() glError: %i\n", error);
	return; // don't make the list, something is borked
      }
    };

    xformList = glGenLists(1);
    glNewList(xformList, GL_COMPILE);
    {
      const float* m = xformTool->getMatrix();
      if (glMultTransposeMatrixf) {
        glMultTransposeMatrixf(m);
      }
      else {
        float matrix[16];
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            matrix[(i * 4) + j] = m[(j * 4) + i];
          }
        }
        glMultMatrixf(matrix);
      }
    }
    glEndList();

    error = glGetError();
    if (error != GL_NO_ERROR) {
      logDebugMessage(0,"ERROR: MeshSceneNode::makeXFormList() failed: %i\n", error);
      xformList = INVALID_GL_LIST_ID;
    }
  }
  return;
}


void MeshSceneNode::freeXFormList()
{
  if (xformList != INVALID_GL_LIST_ID) {
    glDeleteLists(xformList, 1);
    xformList = INVALID_GL_LIST_ID;
  }
  return;
}


void MeshSceneNode::initContext(void* data)
{
  ((MeshSceneNode*)data)->makeXFormList();
  return;
}


void MeshSceneNode::freeContext(void* data)
{
  ((MeshSceneNode*)data)->freeXFormList();
  return;
}


void MeshSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
  // NOTES:
  // - used for ShellNodes
  // - send out the highest LOD, because only used up close
  // - FIXME  only use opaque nodes?
  if (lodCount > 0) {
    LodNode& lodNode = lods[0];
    for (int set = 0; set < lodNode.count; set++) {
      SetNode& setNode = lodNode.sets[set];
      RenderSet rs = { setNode.node, &setNode.meshMat.gstate };
      rnodes.push_back(rs);
    }
  }
  return;
}


//////////////////////////
// Static Class Members //
//////////////////////////

void MeshSceneNode::setLodScale(int pixelsX, float fovx,
				int pixelsY, float fovy)
{
  const float lppx = 2.0f * sinf(fovx * 0.5f) / (float)pixelsX;
  const float lppy = 2.0f * sinf(fovy * 0.5f) / (float)pixelsY;
  const float lpp = (lppx < lppy) ? lppx : lppy;
  static BZDB_float lodScale("lodScale");
  LodScale = lpp * lodScale;
  return;
}


void MeshSceneNode::setRadarLodScale(float lengthPerPixel)
{
  RadarLodScale = lengthPerPixel * BZDB.eval("radarLodScale");
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
