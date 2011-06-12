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

/*
 * RadarRenderer:
 *  Encapsulates drawing a radar
 */

#ifndef BZF_RADAR_RENDERER_H
#define BZF_RADAR_RENDERER_H

#include "common.h"

/* common interface headers */
#include "bzfgl.h"
#include "vectors.h"
#include "obstacle/Obstacle.h"
#include "ogl/OpenGLUtils.h"

/* local implementation headers */
#include "obstacle/MeshObstacle.h"
#include "obstacle/BoxBuilding.h"
#include "obstacle/PyramidBuilding.h"


class SceneRenderer;
class World;
class ShotPath;
class Player;


class RadarRenderer : public GLDisplayListCreator {
  public:
    virtual ~RadarRenderer();
    RadarRenderer(const SceneRenderer&, World* _world);
    void    setWorld(World* _world);

    void    setControlColor(const fvec4* color = NULL);

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    float getRange() const;

    void setShape(int x, int y, int w, int h);
    void setJammed(bool = true);

    void setDimming(float newDimming);

    void render(SceneRenderer&, bool blank, bool observer);

    void renderFrame(SceneRenderer&);

    void renderObstacles(bool fastRadar, float range);
    void renderWalls();
    void renderBoxPyrMesh();
    void renderBoxPyrMeshFast(float range);
    void renderBasesAndTeles();

    int getFrameTriangleCount() const;

    virtual void  buildGeometry(GLDisplayList displayList);
    void clearRadarObjects(void);

    bool executeScissor();
    bool executeTransform(bool localView);

  private:
    // no copying
    RadarRenderer(const RadarRenderer&);
    RadarRenderer& operator=(const RadarRenderer&);

    void drawNoise(SceneRenderer& renderer, float radarRange);
    void drawShot(const ShotPath*);
    void drawTank(const Player* player, bool allowFancy);
    void drawFancyTank(const Player* player);
    void drawHuntLevel(const Player* player,
                       float tankSize, float heightBoxSize);
    void drawFlag(const fvec3& pos);
    void drawFlagOnTank(const fvec3& pos);

    static float colorScale(const float z, const float h);
    static float transScale(const float z, const float h);

  private:
    World*  world;
    int   x, y;
    int   w, h;
    float   dimming;
    float   ps;
    float   range;
    double  decay;
    fvec4   teamColor;
    bool    smooth;
    bool    jammed;
    bool    colorblind;
    bool    multiSampled;
    bool    useTankModels;
    bool    useTankDimensions;
    int   triangleCount;
    static const float  colorFactor;

    bool    lastFast;

    enum RadarObjectType {
      eNone,
      eBoxPyr,
      eMesh,
      eMeshDeathFaces,
      eBoxPyrOutline
    };

    typedef std::pair<RadarObjectType, const Obstacle*> RadarObject;
    typedef std::map<GLDisplayList, RadarObject> RadarObjectMap;
    RadarObjectMap radarObjectLists;

    void buildBoxPyr(const Obstacle* object);
    // void buildBoxGeo(BoxBuilding* box);
    // void buildPryGeo(PyramidBuilding* pyr);
    void buildMeshGeo(const MeshObstacle* mesh, RadarObjectType type);
    void buildOutline(const Obstacle* object);
    //   void buildBoxOutline(const BoxBuilding& box);
    //   void buildPyrOutline(const PyramidBuilding& pyr);
};

//
// RadarRenderer
//

inline int RadarRenderer::getX() const {
  return x;
}

inline int RadarRenderer::getY() const {
  return y;
}

inline int RadarRenderer::getWidth() const {
  return w;
}

inline int RadarRenderer::getHeight() const {
  return h;
}

inline float RadarRenderer::getRange() const {
  return range;
}

#endif // BZF_RADAR_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
