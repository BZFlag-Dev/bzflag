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

/* TankSceneNode:
 *  Encapsulates information for rendering a tank
 */

#ifndef BZF_TANK_SCENE_NODE_H
#define BZF_TANK_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"
#include "OpenGLLight.h"
#include "TankGeometryMgr.h"
#include "vectors.h"

class TankSceneNode;

class TankIDLSceneNode : public SceneNode {
  public:
    TankIDLSceneNode(const TankSceneNode*);
    virtual ~TankIDLSceneNode();

    virtual void move(const fvec4& plane);

    virtual void notifyStyleChange();
    virtual void addRenderNodes(SceneRenderer&);
    // Irix 7.2.1 and solaris compilers appear to have a bug.  if the
    // following declaration isn't public it generates an error when trying
    // to declare SphereFragmentSceneNode::FragmentRenderNode a friend in
    // SphereSceneNode::SphereRenderNode.  i think this is a bug in the
    // compiler because:
    //   no other compiler complains
    //   public/protected/private adjust access not visibility
    //     SphereSceneNode isn't requesting access, it's granting it
//  protected:
  public:
    class IDLRenderNode : public RenderNode {
      public:
        IDLRenderNode(const TankIDLSceneNode*);
        ~IDLRenderNode();
        void    render();
        const fvec3&  getPosition() const { return sceneNode->getCenter(); }
      private:
        const TankIDLSceneNode* sceneNode;
        static const int  idlFaces[][5];
        static const fvec3  idlVertex[];
    };
    friend class IDLRenderNode;

  protected:
    const TankSceneNode* tank;
    fvec4   plane;
    OpenGLGState  gstate;
    IDLRenderNode renderNode;
};


class TankSceneNode : public SceneNode {

    friend class TankIDLSceneNode;
    friend class TankIDLSceneNode::IDLRenderNode;

  public:
    TankSceneNode(const fvec3& pos, const fvec3& forward);
    virtual ~TankSceneNode();

    virtual void move(const fvec3& pos, const fvec3& forward);

    virtual void setColor(float r, float g, float b, float a = 1.0f);
    virtual void setColor(const fvec4& rgba);
    virtual void setMaterial(const OpenGLMaterial&);
    virtual void setTexture(const int);
    virtual void setJumpJetsTexture(const int);

    virtual void setNormal();
    virtual void setObese();
    virtual void setTiny();
    virtual void setNarrow();
    virtual void setThief();
    virtual void setDimensions(const fvec3& size);

    virtual void setClipPlane(const fvec4* plane);
    virtual void setExplodeFraction(float t);
    virtual void setJumpJets(float scale);

    virtual void setInTheCockpit(bool value);
    virtual void setOnlyShadows(bool value);

    virtual void rebuildExplosion();
    virtual void addTreadOffsets(float left, float right);

    virtual void notifyStyleChange();
    virtual void addRenderNodes(SceneRenderer&);
    virtual void addShadowNodes(SceneRenderer&);

    virtual bool cullShadow(int planeCount, const fvec4* planes) const;

    virtual void addLight(SceneRenderer&);

    virtual void renderRadar();

    static void setMaxLOD(int maxLevel);

  protected:

    class TankRenderNode : public RenderNode {
      public:
        TankRenderNode(const TankSceneNode*);
        ~TankRenderNode();

        void    setShadow();
        void    setRadar(bool);
        void    setTreads(bool);
        void    setTankLOD(TankGeometryEnums::TankLOD);
        void    setTankSize(TankGeometryEnums::TankSize);
        void    sortOrder(bool above, bool towards, bool left);
        void    setNarrowWithDepth(bool narrow);
        const fvec3&  getPosition() const { return sceneNode->getCenter(); }

        void    render();
        void    renderPart(TankGeometryEnums::TankPart part);
        void    renderParts();
        void    renderTopParts();
        void    renderLeftParts();
        void    renderRightParts();
        void    renderNarrowWithDepth();
        void    renderLights();
        void    renderJumpJets();
        void    setupPartColor(TankGeometryEnums::TankPart part);
        bool    setupTextureMatrix(TankGeometryEnums::TankPart part);

      protected:
        const TankSceneNode* sceneNode;
        TankGeometryEnums::TankLOD drawLOD;
        TankGeometryEnums::TankSize drawSize;
        const fvec4*  color;
        float   alpha;
        bool    isRadar;
        bool    isTreads;
        bool    isShadow;
        bool    left;
        bool    above;
        bool    towards;
        bool    isExploding;
        bool    narrowWithDepth;
        float   explodeFraction;
        static const fvec3 centerOfGravity[TankGeometryEnums::LastTankPart];
    };
    friend class TankRenderNode;
  protected:
    float   azimuth;
    float   elevation;
    float   baseRadius;
    fvec3   dimensions; // tank dimensions
    float   leftTreadOffset;
    float   rightTreadOffset;
    float   leftWheelOffset;
    float   rightWheelOffset;
    bool    useDimensions;
    bool    useOverride;
    bool    onlyShadows;
    bool    transparent, sort;
    float   spawnFraction;
    float   explodeFraction;
    bool    clip;
    bool    inTheCockpit;
    fvec4   colorOverride;
    fvec4   color;
    dvec4   clipPlane;
    OpenGLGState  gstate;
    OpenGLGState  treadState;
    OpenGLGState  lightsGState;
    TankRenderNode  tankRenderNode;
    TankRenderNode  treadsRenderNode;
    TankRenderNode  shadowRenderNode;
    TankGeometryEnums::TankSize tankSize;
    fvec3   vel[TankGeometryEnums::LastTankPart];
    fvec4   spin[TankGeometryEnums::LastTankPart];
    bool    jumpJetsOn;
    float   jumpJetsScale;
    float   jumpJetsLengths[4];
    fvec3   jumpJetsPositions[4];
    OpenGLLight   jumpJetsRealLight;
    OpenGLLight   jumpJetsGroundLights[4];
    OpenGLGState  jumpJetsGState;

    static int    maxLevel;
    static const int  numLOD;
    static fvec3  jumpJetsModel[4];
};


#endif // BZF_TANK_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
