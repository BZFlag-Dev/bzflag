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

/* TriWallSceneNode:
 *	Encapsulates information for rendering a triangular wall.
 */

#ifndef	BZF_TRI_WALL_SCENE_NODE_H
#define	BZF_TRI_WALL_SCENE_NODE_H

#include "common.h"
#include "WallSceneNode.h"

class TriWallSceneNode : public WallSceneNode {
  public:
			TriWallSceneNode(const GLfloat base[3],
				const GLfloat sEdge[3],
				const GLfloat tEdge[3],
				float uRepeats = 1.0,
				float vRepeats = 1.0,
				bool makeLODs = true);
			~TriWallSceneNode();

    int			split(const float*, SceneNode*&, SceneNode*&) const;

    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);
    void		renderRadar();

    bool		inAxisBox (const Extents& exts) const;

    int			getVertexCount () const;
    const		GLfloat* getVertex (int vertex) const;

    bool		cull(const ViewFrustum&) const;

    void		getRenderNodes(std::vector<RenderSet>& rnodes);

  protected:
    class Geometry : public RenderNode {
      public:
			Geometry(TriWallSceneNode*,
				int eCount,
				const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				const GLfloat* normal,
				float uRepeats, float vRepeats);
			~Geometry();
	void		setStyle(int _style) { style = _style; }
	void		render();
	void		renderShadow();
	const GLfloat*  getVertex(int i) const;
	const GLfloat*	getPosition() const { return wall->getSphere(); }
      private:
	void		drawV() const;
	void		drawVT() const;
      private:
	WallSceneNode*	wall;
	int		style;
	int		de;
	const GLfloat*	normal;
      public:
	GLfloat3Array	vertex;
	GLfloat2Array	uv;
	int             triangles;
    };

  private:
    Geometry**		nodes;
    Geometry*		shadowNode;
};

#endif // BZF_TRI_WALL_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

