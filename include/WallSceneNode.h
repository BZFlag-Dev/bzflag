/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WallSceneNode:
 *	Encapsulates information for rendering an wall.
 *
 * WallGeometry:
 *	Encapsulates vertices and uv's for a wall
 *
 * Walls are flat and don't move.  A wall also picks a level of
 * detail based on its projected area and on the presence of
 * light sources nearby (to capture light highlights).
 */

#ifndef	BZF_WALL_SCENE_NODE_H
#define	BZF_WALL_SCENE_NODE_H

#include "SceneNode.h"

class WallSceneNode : public SceneNode {
  public:
			WallSceneNode();
			~WallSceneNode();

    const GLfloat*	getColor() const { return color; }
    const GLfloat*	getModulateColor() const { return modulateColor; }
    const GLfloat*	getLightedColor() const { return lightedColor; }
    const GLfloat*	getLightedModulateColor() const
				{ return lightedModulateColor; }
    const GLfloat*	getPlane() const;
    GLfloat		getDistance(const GLfloat*) const;

    void		setColor(GLfloat r, GLfloat g,
				GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		setModulateColor(GLfloat r, GLfloat g,
				GLfloat b, GLfloat a = 1.0f);
    void		setModulateColor(const GLfloat* rgba);
    void		setLightedColor(GLfloat r, GLfloat g,
				GLfloat b, GLfloat a = 1.0f);
    void		setLightedColor(const GLfloat* rgba);
    void		setLightedModulateColor(GLfloat r, GLfloat g,
				GLfloat b, GLfloat a = 1.0f);
    void		setLightedModulateColor(const GLfloat* rgba);
    void		setMaterial(const OpenGLMaterial&);
    void		setTexture(const OpenGLTexture&);

    void		setColor();

    bool		cull(const ViewFrustum&) const;
    void		notifyStyleChange(const SceneRenderer& renderer);

    void		copyStyle(WallSceneNode*);

  protected:
    int			getNumLODs() const;
    void		setNumLODs(int, float* elementAreas);
    void		setPlane(const GLfloat[4]);
    int			pickLevelOfDetail(const SceneRenderer&) const;

    int			getStyle() const { return style; }
    const OpenGLGState&	getGState() const { return gstate; }

    static int		splitWall(const GLfloat* plane,
				const GLfloat3Array& vertices,
				const GLfloat2Array& uvs,
				SceneNode*& front, SceneNode*& back); // const

  private:
    static void		splitEdge(const GLfloat* p1, const GLfloat* p2,
				const GLfloat* uv1, const GLfloat* uv2,
				const GLfloat* plane,
				GLfloat* p, GLfloat* uv); // const

  private:
    int			numLODs;
    float*		elementAreas;
    GLfloat		plane[4];	// unit normal, distance to origin
    GLfloat		color[4];
    GLfloat		modulateColor[4];
    GLfloat		lightedColor[4];
    GLfloat		lightedModulateColor[4];
    int			style;
    bool		transparent;
    bool		modulateTransparent;
    bool		lightedTransparent;
    bool		lightedModulateTransparent;
    OpenGLGState	gstate;
};

//
// WallSceneNode
//

inline int		WallSceneNode::getNumLODs() const
{
  return numLODs;
}

inline const GLfloat*	WallSceneNode::getPlane() const
{
  return plane;
}

#endif // BZF_WALL_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
