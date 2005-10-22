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

#include "common.h"
#include "SceneNode.h"

class WallSceneNode : public SceneNode {
  public:
			WallSceneNode();
			~WallSceneNode();

    const GLfloat*	getColor() const;
    const GLfloat*	getDynamicColor() const;
    const GLfloat*	getModulateColor() const;
    const GLfloat*	getLightedColor() const;
    const GLfloat*	getLightedModulateColor() const;
    GLfloat		getDistance(const GLfloat*) const;
    virtual bool	inAxisBox (const Extents& exts) const;

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
    void		setTexture(const int);
    void		setTextureMatrix(const GLfloat* texmat);
    void		setDynamicColor(const float* color);
    void		setBlending(bool);
    void		setSphereMap(bool);
    void		setNoCulling(bool);
    void		setNoSorting(bool);
    void		setAlphaThreshold(float);
    void		setRadarColor(const float color[4]);

    void		setColor();

    bool		cull(const ViewFrustum&) const;
    void		notifyStyleChange();

    void		copyStyle(WallSceneNode*);

    void		setUseColorTexture(bool use){useColorTexture=use;}
  protected:
    int			getNumLODs() const;
    void		setNumLODs(int, float* elementAreas);
    void		setPlane(const GLfloat[4]);
    int			pickLevelOfDetail(const SceneRenderer&) const;

    int			getStyle() const;
    const OpenGLGState*	getGState() const { return &gstate; }
    const OpenGLGState*	getWallGState() const;

    static int		splitWall(const GLfloat* plane,
				const GLfloat3Array& vertices,
				const GLfloat2Array& uvs,
				SceneNode*& front, SceneNode*& back); // const

  private:
    static void splitEdge(float d1, float d2,
			  const GLfloat* p1, const GLfloat* p2,
			  const GLfloat* uv1, const GLfloat* uv2,
			  GLfloat* p, GLfloat* uv); //const

  private:
    int			numLODs;
    float*		elementAreas;
    const GLfloat*	dynamicColor;
    GLfloat		color[4];
    GLfloat		modulateColor[4];
    GLfloat		lightedColor[4];
    GLfloat		lightedModulateColor[4];
    float		alphaThreshold;
    int			style;
    bool		noCulling;
    bool		noSorting;
    bool		isBlended;
    bool		wantBlending;
    bool		wantSphereMap;
    OpenGLGState	gstate;
    bool		useColorTexture;
};

//
// WallSceneNode
//

inline int WallSceneNode::getNumLODs() const
{
  return numLODs;
}

inline const GLfloat* WallSceneNode::getColor() const
{
  return color;
}
inline const GLfloat* WallSceneNode::getDynamicColor() const
{
  return dynamicColor;
}
inline const GLfloat* WallSceneNode::getModulateColor() const
{
  return modulateColor;
}
inline const GLfloat* WallSceneNode::getLightedColor() const
{
  return lightedColor;
}
inline const GLfloat* WallSceneNode::getLightedModulateColor() const
{
  return lightedModulateColor;
}

inline int WallSceneNode::getStyle() const
{
  return style;
}
inline const OpenGLGState* WallSceneNode::getWallGState() const
{
  return &gstate;
}


#endif // BZF_WALL_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

