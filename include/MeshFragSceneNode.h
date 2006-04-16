/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshFragSceneNode:
 *	Encapsulates information for rendering a mesh fragment
 *      (a collection of faces with the same material properties).
 *	Does not support level of detail.
 */

#ifndef	BZF_MESH_FRAG_SCENE_NODE_H
#define	BZF_MESH_FRAG_SCENE_NODE_H

#include "common.h"
#include "WallSceneNode.h"
#include "BzMaterial.h"

//
// NOTES:
//
// - Make sure that "noPlane" is set to true, for Mesh Fragments can not be
//   used as occluders, and can not be culled as a simple plane
//
// - The lists are all GL_TRIANGLE lists
//

class MeshFace;

class MeshFragSceneNode : public WallSceneNode {

  public:
    MeshFragSceneNode(int faceCount, const MeshFace** faces);
    ~MeshFragSceneNode();

    // virtual functions from SceneNode
    bool cull(const ViewFrustum&) const;
    void addShadowNodes(SceneRenderer&);
    void addRenderNodes(SceneRenderer&);
    void renderRadar();

    // virtual functions from WallSceneNode
    bool inAxisBox(const Extents& exts) const;

    void getRenderNodes(std::vector<RenderSet>& rnodes);

  protected:
    class Geometry : public RenderNode {
      public:
	Geometry(MeshFragSceneNode* node);
	~Geometry();

	void init();
	void setStyle(int _style) { style = _style; }
	void render();
	void renderRadar();
	void renderShadow();
	const GLfloat* getPosition() const { return sceneNode->getSphere(); }

      private:
	inline void drawV() const; // draw with just vertices
	inline void drawVT() const; // draw with texcoords
	inline void drawVN() const; // draw with normals
	inline void drawVTN() const; // draw with texcoords and normals

	void initDisplayList();
	void freeDisplayList();
	static void initContext(void *data);
	static void freeContext(void *data);

      private:
	int style;
	GLuint list;
	MeshFragSceneNode* sceneNode;
    };

  private:
    Geometry renderNode;

    GLint faceCount;
    const MeshFace** faces;

    bool noRadar;
    bool noShadow;
    GLint arrayCount;
    GLfloat* vertices;
    GLfloat* normals;
    GLfloat* texcoords;

  friend class MeshFragSceneNode::Geometry;
};


#endif // BZF_MESH_FRAG_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

