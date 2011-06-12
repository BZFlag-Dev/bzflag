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

/* FlagWarpSceneNode:
 *  Encapsulates information for rendering the little cloud
 *  that appears when a flag is coming or going.
 */

#ifndef BZF_FLAG_WARP_SCENE_NODE_H
#define BZF_FLAG_WARP_SCENE_NODE_H

#include "common.h"
#include "geometry/SceneNode.h"

class FlagWarpSceneNode : public SceneNode {
  public:
    FlagWarpSceneNode(const fvec3& pos);
    ~FlagWarpSceneNode();

    void    setSizeFraction(float);

    float   getDistanceSq(const fvec3& eye) const;
    void    move(const fvec3& pos);

    void    notifyStyleChange();
    void    addRenderNodes(SceneRenderer&);

  protected:
    class FlagWarpRenderNode : public RenderNode {
      public:
        FlagWarpRenderNode(const FlagWarpSceneNode*);
        ~FlagWarpRenderNode();
        void    render();
        const fvec3&  getPosition() const { return sceneNode->getCenter(); }
      private:
        const FlagWarpSceneNode* sceneNode;
    };
    friend class FlagWarpRenderNode;

  private:
    float   size;
    OpenGLGState  gstate;
    FlagWarpRenderNode  renderNode;
    static const fvec4  color[7];
};

#endif // BZF_FLAG_WARP_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
