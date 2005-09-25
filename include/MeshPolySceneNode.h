/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshPolySceneNode:
 *	Encapsulates information for rendering a planar
 *	polygonal wall.  Does not support level of detail.
 */

#ifndef	BZF_MESH_POLY_SCENE_NODE_H
#define	BZF_MESH_POLY_SCENE_NODE_H

#include "common.h"
#include "WallSceneNode.h"

class MeshPolySceneNode : public WallSceneNode {
  public:
    MeshPolySceneNode(const float plane[4],
                      bool noRadar, bool noShadow,
		      const GLfloat3Array& vertices,
		      const GLfloat3Array& normals,
		      const GLfloat2Array& texcoords);
    ~MeshPolySceneNode();

    bool cull(const ViewFrustum& frustum) const;
    bool inAxisBox (const Extents& exts) const;
    int getVertexCount () const;
    const GLfloat* getVertex (int vertex) const;
    const GLfloat (*getVertices() const)[3];

    int split(const float* plane, SceneNode*&, SceneNode*&) const;

    void addRenderNodes(SceneRenderer&);
    void addShadowNodes(SceneRenderer&);
    void renderRadar();

    void getRenderNodes(std::vector<RenderSet>& rnodes);


  protected:
    class Geometry : public RenderNode {
      public:
	Geometry(MeshPolySceneNode*,
		 const GLfloat3Array& vertices,
		 const GLfloat3Array& normals,
		 const GLfloat2Array& texcoords,
		 const GLfloat* normal);
	~Geometry();
	void setStyle(int _style) { style = _style; }
	bool getNoRadar() const;
	void setNoRadar();
	void render();
	void renderRadar();
	void renderShadow();
	const GLfloat* getVertex(int i) const;
	const GLfloat (*getVertices() const)[3];
	const int getVertexCount() const;
	const GLfloat* getPosition() const { return sceneNode->getSphere(); }
      private:
	void drawV() const; // draw with just vertices
	void drawVT() const; // draw with texcoords
	void drawVN() const; // draw with normals
	void drawVTN() const; // draw with texcoords and normals
      private:
	MeshPolySceneNode* sceneNode;
	int style;
	bool drawRadar;
	bool draw;
	const GLfloat* normal;
      public:
	GLfloat3Array vertices;
	GLfloat3Array normals;
	GLfloat2Array texcoords;
    };

  private:
    int splitWallVTN(const GLfloat* plane,
		     const GLfloat3Array& vertices,
		     const GLfloat3Array& normals,
		     const GLfloat2Array& texcoords,
		     SceneNode*& front, SceneNode*& back) const;

    void splitEdgeVTN(float d1, float d2,
		      const GLfloat* p1, const GLfloat* p2,
		      const GLfloat* n1, const GLfloat* n2,
		      const GLfloat* uv1, const GLfloat* uv2,
		      GLfloat* p, GLfloat* n, GLfloat* uv) const;

    int splitWallVT(const GLfloat* plane,
		    const GLfloat3Array& vertices,
		    const GLfloat2Array& texcoords,
		    SceneNode*& front, SceneNode*& back) const;

    void splitEdgeVT(float d1, float d2,
		     const GLfloat* p1, const GLfloat* p2,
		     const GLfloat* uv1, const GLfloat* uv2,
		     GLfloat* p, GLfloat* uv) const;

    Geometry node;
    bool noRadar;
    bool noShadow;
};

inline const int MeshPolySceneNode::Geometry::getVertexCount() const
{
  return vertices.getSize();
}

inline int MeshPolySceneNode::getVertexCount () const
{
  return node.getVertexCount();
}

inline const GLfloat* MeshPolySceneNode::Geometry::getVertex(int i) const
{
  return vertices[i];
}

inline const GLfloat (*MeshPolySceneNode::Geometry::getVertices() const)[3]
{
  return vertices.getArray();
}

inline const GLfloat* MeshPolySceneNode::getVertex(int i) const
{
  return node.getVertex(i);
}

inline const GLfloat (*MeshPolySceneNode::getVertices() const)[3]
{
  return node.getVertices();
}


#endif // BZF_MESH_POLY_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

