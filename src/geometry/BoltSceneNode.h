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

/* BoltSceneNode:
 *  Encapsulates information for rendering a ball lighting bolt.
 */

#ifndef BZF_BOLT_SCENE_NODE_H
#define BZF_BOLT_SCENE_NODE_H

#include "common.h"
#include "ShotSceneNode.h"
#include "ogl/OpenGLLight.h"

class BoltSceneNode : public ShotSceneNode {
  public:
    BoltSceneNode(const fvec3& pos, const fvec3& vel);
    ~BoltSceneNode();

    void    setFlares(bool);
    void    setSize(float radius);
    void    setColor(float r, float g, float b, float a = 1.0f);
    void    setTextureColor(float r, float g, float b, float a = 1.0f);
    void    setColor(const fvec4& rgb);
    void    setTexture(const int);
    void    setTextureAnimation(int cu, int cv);

    bool    getColorblind() const;
    void    setColorblind(bool);

    bool    getInvisible() const;
    void    setInvisible(bool);

    void    move(const fvec3& pos, const fvec3& forward);
    void    addLight(SceneRenderer&);

    void    notifyStyleChange();
    void    addRenderNodes(SceneRenderer&);

  public:
    bool    phasingShot;

  protected:
    class BoltRenderNode : public RenderNode {
      public:
        BoltRenderNode(const BoltSceneNode*);
        ~BoltRenderNode();
        void    setColor(const fvec4& rgba);
        void    setTextureColor(const fvec4& rgba);
        void    render();
        void    renderGeoBolt();
        void    renderGeoGMBolt();
        void    renderGeoPill(float radius, float length, int segments, float endRad = -1);

        const fvec3&  getPosition() const { return sceneNode->getCenter(); }
        void    setAnimation(int cu, int cv);
      private:
        const BoltSceneNode* sceneNode;
        int   u, v, cu, cv;
        float   du, dv;
        fvec4   mainColor;
        fvec4   innerColor;
        fvec4   outerColor;
        fvec4   coronaColor;
        fvec4   flareColor;
        fvec4   textureColor;
        int   numFlares;
        float   theta[6];
        float   phi[6];

        static float  core[9][2];
        static float  corona[8][2];
        static const float ring[8][2];
        static const float CoreFraction;
        static const float FlareSize;
        static const float FlareSpread;
    };
    friend class BoltRenderNode;

  private:
    bool    drawFlares;
    bool    invisible;
    bool    texturing;
    bool    colorblind;
    float   size;
    fvec3   velocity;
    fvec4   color;
    OpenGLLight   light;
    OpenGLGState  gstate;
    OpenGLGState  colorblindGState;
    BoltRenderNode  renderNode;

    float   azimuth, elevation, length;
};

#endif // BZF_BOLT_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
