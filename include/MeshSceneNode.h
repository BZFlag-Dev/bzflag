/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshSceneNode:
 *	Encapsulates information for rendering a mesh fragment
 *      (a collection of faces with the same material properties).
 *	Does not support level of detail.
 */

#ifndef	BZF_MESH_SCENE_NODE_H
#define	BZF_MESH_SCENE_NODE_H

#include "common.h"
#include "bzfgl.h"
#include "SceneNode.h"

//
// NOTES:
//
// - Make sure that "noPlane" is set to true, for Mesh can not be
//   used as occluders, and can not be culled as a simple plane
//
// - All accesses are done through indices
//

class MeshObstacle;
class MeshDrawInfo;
class MeshDrawMgr;
class BzMaterial;
class OpenGLGState;
class SceneRenderer;
class ViewFrustum;
class RenderNode;


class MeshSceneNode : public SceneNode {
  public:
    MeshSceneNode(const MeshObstacle* mesh);
    ~MeshSceneNode();

    // virtual functions from SceneNode

    void notifyStyleChange();

    bool cull(const ViewFrustum&) const;
    bool inAxisBox(const Extents& exts) const;

    void addShadowNodes(SceneRenderer&);
    void addRenderNodes(SceneRenderer&);
    void renderRadar();

    void getRenderNodes(std::vector<RenderSet>& rnodes);

    void makeXFormList();
    void freeXFormList();
    static void initContext(void* data);
    static void freeContext(void* data);

    static void setLodScale(int pixelsX, float fovx,
			    int pixelsY, float fovy);
    static void setRadarLodScale(float lengthPerPixel);

  private:
    const MeshObstacle* mesh;

    MeshDrawMgr* drawMgr;
    const MeshDrawInfo* drawInfo;
    
    bool animRepos;

    // transform display list
    GLuint xformList;

    struct MeshMaterial {
      const BzMaterial* bzmat;
      OpenGLGState gstate;
      GLfloat color[4];
      const GLfloat* colorPtr;
      bool drawRadar;
      bool drawShadow;
      bool needsSorting;
      bool animRepos;
    };

    struct SetNode {
      int set;
      MeshMaterial meshMat;
      // basic render nodes
      RenderNode* node;
      RenderNode* radarNode;
    };

    struct LodNode {
      int count;
      SetNode* sets;
    };

    // Level Of Detail (LOD) information
    int lodCount;
    LodNode* lods;
    float* lodLengths;

    // Radar LODs
    int radarCount;
    LodNode* radarLods;
    float* radarLengths;

    static float LodScale;
    static float RadarLodScale;

  private:
    void updateMaterial(MeshMaterial* mat);
    const BzMaterial* convertMaterial(const BzMaterial* bzmat);
    int calcNormalLod(const ViewFrustum&);
    int calcShadowLod(const ViewFrustum&);
    int calcRadarLod();

  friend class MeshSceneNodeMgr;
};


#endif // BZF_MESH_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

