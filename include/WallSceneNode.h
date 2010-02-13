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
#include "vectors.h"

class WallSceneNode : public SceneNode {
  public:
			WallSceneNode();
			~WallSceneNode();

    const fvec4&	getColor() const;
    const fvec4*	getDynamicColor() const;
    const fvec4&	getModulateColor() const;
    const fvec4&	getLightedColor() const;
    const fvec4&	getLightedModulateColor() const;
    float		getDistanceSq(const fvec3& eye) const;
    virtual bool	inAxisBox(const Extents& exts) const;

    void		setColor(float r, float g,
				float b, float a = 1.0f);
    void		setColor(const fvec4& rgba);
    void		setModulateColor(float r, float g,
				float b, float a = 1.0f);
    void		setModulateColor(const fvec4& rgba);
    void		setLightedColor(float r, float g,
				float b, float a = 1.0f);
    void		setLightedColor(const fvec4& rgba);
    void		setLightedModulateColor(float r, float g,
				float b, float a = 1.0f);
    void		setLightedModulateColor(const fvec4& rgba);

    void		setMaterial(const OpenGLMaterial&);
    void		setTexture(const int);
    void		setTextureMatrix(const float* texmat);
    void		setOrder(int);
    void		setDynamicColor(const fvec4* color);
    void		setBlending(bool);
    void		setSphereMap(bool);
    void		setNoCulling(bool);
    void		setNoSorting(bool);
    void		setNoBlending(bool);
    void		setAlphaThreshold(float);
    void		setPolygonOffset(float factor, float units);

    void		setColor();

    bool		cull(const ViewFrustum&) const;
    void		notifyStyleChange();

    void		copyStyle(WallSceneNode*);

    void		setUseColorTexture(bool use){useColorTexture=use;}

    virtual void	setRadarSpecial(bool) { return; }

  protected:
    int			getNumLODs() const;
    void		setNumLODs(int, float* elementAreas);
    void		setPlane(const fvec4&);
    int			pickLevelOfDetail(const SceneRenderer&) const;

    int			getStyle() const;
    const OpenGLGState*	getGState() const { return &gstate; }
    const OpenGLGState*	getWallGState() const;

    static int		splitWall(const fvec4& plane,
			          const fvec3Array& vertices,
			          const fvec2Array& uvs,
			          SceneNode*& front, SceneNode*& back); // const

  private:
    static void splitEdge(float d1, float d2,
			  const fvec3& p1, const fvec3& p2,
			  const fvec2& uv1, const fvec2& uv2,
			  fvec3& p, fvec2& uv); //const

  private:
    int			numLODs;
    float*		elementAreas;
    int			order;
    const fvec4*	dynamicColor;
    fvec4		color;
    fvec4		modulateColor;
    fvec4		lightedColor;
    fvec4		lightedModulateColor;
    float		alphaThreshold;
    float		poFactor;
    float		poUnits;
    int			style;
    bool		noCulling;
    bool		noSorting;
    bool		noBlending;
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

inline const fvec4& WallSceneNode::getColor() const
{
  return color;
}
inline const fvec4* WallSceneNode::getDynamicColor() const
{
  return dynamicColor;
}
inline const fvec4& WallSceneNode::getModulateColor() const
{
  return modulateColor;
}
inline const fvec4& WallSceneNode::getLightedColor() const
{
  return lightedColor;
}
inline const fvec4& WallSceneNode::getLightedModulateColor() const
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
