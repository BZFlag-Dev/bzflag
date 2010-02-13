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

/* TriWallSceneNode:
 *	Encapsulates information for rendering a triangular wall.
 */

#ifndef	BZF_TRI_WALL_SCENE_NODE_H
#define	BZF_TRI_WALL_SCENE_NODE_H

#include "common.h"
#include "WallSceneNode.h"

class TriWallSceneNode : public WallSceneNode {
  public:
    TriWallSceneNode(const fvec3& base,
                     const fvec3& sEdge, const fvec3& tEdge,
                     float uRepeats = 1.0, float vRepeats = 1.0,
                     bool makeLODs = true);
    ~TriWallSceneNode();

    int			split(const fvec4&, SceneNode*&, SceneNode*&) const;

    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);
    void		renderRadar();

    bool		inAxisBox (const Extents& exts) const;

    int			getVertexCount () const;
    const fvec3&	getVertex (int vertex) const;

    bool		cull(const ViewFrustum&) const;

    void		getRenderNodes(std::vector<RenderSet>& rnodes);

  protected:
    class Geometry : public RenderNode {
      public:
        Geometry(TriWallSceneNode*, int eCount,
                 const fvec3& base, const fvec3& uEdge, const fvec3& vEdge,
                 const float* normal, float uRepeats, float vRepeats);
        ~Geometry();
	void		setStyle(int _style) { style = _style; }
	void		render();
	void		renderShadow();
	const fvec3&	getVertex(int i) const;
	const fvec3&	getPosition() const { return wall->getCenter(); }
      private:
	void		drawV() const;
	void		drawVT() const;
      private:
	WallSceneNode*	wall;
	int		style;
	int		de;
	const float*	normal;
      public:
	fvec3Array	vertex;
	fvec2Array	uv;
	int	     triangles;
    };

  private:
    Geometry**		nodes;
    Geometry*		shadowNode;
};

#endif // BZF_TRI_WALL_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
