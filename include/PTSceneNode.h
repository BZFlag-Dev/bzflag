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

/* PhotonTorpedoSceneNode:
 *	Encapsulates information for rendering a photon torpedo-ish
 *	object.
 */

#ifndef	BZF_PHOTON_TORPEDO_SCENE_NODE_H
#define	BZF_PHOTON_TORPEDO_SCENE_NODE_H

#include "common.h"
#include "ShotSceneNode.h"
#include "OpenGLLight.h"

class PhotonTorpedoSceneNode : public ShotSceneNode {
  public:
    PhotonTorpedoSceneNode(const fvec3& pos);
    ~PhotonTorpedoSceneNode();

    void move(const fvec3& pos, const fvec3& forward);
    void addLight(SceneRenderer&);

    void notifyStyleChange();
    void addRenderNodes(SceneRenderer&);

  protected:
    class PTRenderNode : public RenderNode {
      public:
        PTRenderNode(const PhotonTorpedoSceneNode*);
        ~PTRenderNode();
	void render();
	const fvec3&	getPosition() const { return sceneNode->getCenter(); }
      private:
	const PhotonTorpedoSceneNode* sceneNode;
	int   numFlares;
	float theta[6];
	float phi[6];

	static float core[9][2];
	static float corona[8][2];
	static const float ring[8][2];
    };
    friend class PTRenderNode;

  private:
    OpenGLLight  light;
    OpenGLGState gstate;
    PTRenderNode renderNode;
    static const float CoreSize;
    static const float CoronaSize;
    static const float FlareSize;
    static const float FlareSpread;
};

#endif // BZF_PHOTON_TORPEDO_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
