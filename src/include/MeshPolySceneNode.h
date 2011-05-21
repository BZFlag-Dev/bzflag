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
    MeshPolySceneNode(const fvec4& plane,
		      bool noRadar, bool noShadow,
		      const fvec3Array& vertices,
		      const fvec3Array& normals,
		      const fvec2Array& texcoords);
    ~MeshPolySceneNode();

    bool cull(const ViewFrustum& frustum) const;
    bool inAxisBox (const Extents& exts) const;
    int getVertexCount () const;
    const fvec3& getVertex(int vertex) const;
    const fvec3* getVertices() const;

    int split(const fvec4& plane, SceneNode*&, SceneNode*&) const;

    void addRenderNodes(SceneRenderer&);
    void addShadowNodes(SceneRenderer&);
    void renderRadar();

    void getRenderNodes(std::vector<RenderSet>& rnodes);

    void setRadarSpecial(bool value) { radarSpecial = value; }

  protected:
    class Geometry : public RenderNode {
      public:
	Geometry(MeshPolySceneNode*,
		 const fvec3Array& vertices,
		 const fvec3Array& normals,
		 const fvec2Array& texcoords,
		 const fvec3& normal);
	~Geometry();
	void setStyle(int _style) { style = _style; }
	bool getNoRadar() const;
	void setNoRadar();
	void render();
	void renderRadar();
	void renderShadow();
	const fvec3& getVertex(int i) const;
	const fvec3* getVertices() const;
	int getVertexCount() const;
	const fvec3& getPosition() const { return sceneNode->getCenter(); }
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
	const fvec3& normal;
      public:
	fvec3Array vertices;
	fvec3Array normals;
	fvec2Array texcoords;
    };

  private:
    int splitWallVTN(const fvec4& plane,
		     const fvec3Array& vertices,
		     const fvec3Array& normals,
		     const fvec2Array& texcoords,
		     SceneNode*& front, SceneNode*& back) const;

    void splitEdgeVTN(float d1, float d2,
		      const fvec3& p1,  const fvec3& p2,
		      const fvec3& n1,  const fvec3& n2,
		      const fvec2& uv1, const fvec2& uv2,
		      fvec3& p, fvec3& n, fvec2& uv) const;

    int splitWallVT(const fvec4& plane,
		    const fvec3Array& vertices,
		    const fvec2Array& texcoords,
		    SceneNode*& front, SceneNode*& back) const;

    void splitEdgeVT(float d1, float d2,
		     const fvec3& p1,  const fvec3& p2,
		     const fvec2& uv1, const fvec2& uv2,
		     fvec3& p, fvec2& uv) const;

    Geometry node;
    bool noRadar;
    bool noShadow;
    bool radarSpecial;
};

inline int MeshPolySceneNode::Geometry::getVertexCount() const
{
  return vertices.getSize();
}

inline int MeshPolySceneNode::getVertexCount () const
{
  return node.getVertexCount();
}

inline const fvec3& MeshPolySceneNode::Geometry::getVertex(int i) const
{
  return vertices[i];
}

inline const fvec3* MeshPolySceneNode::Geometry::getVertices() const
{
  return vertices.getArray();
}

inline const fvec3& MeshPolySceneNode::getVertex(int i) const
{
  return node.getVertex(i);
}

inline const fvec3* MeshPolySceneNode::getVertices() const
{
  return node.getVertices();
}


#endif // BZF_MESH_POLY_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
