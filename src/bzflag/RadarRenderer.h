/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 *	Encapsulates drawing a radar
 */

#ifndef	BZF_RADAR_RENDERER_H
#define	BZF_RADAR_RENDERER_H

#include "common.h"

/* common interface headers */
#include "bzfgl.h"
#include "Obstacle.h"
#include "OpenGLUtils.h"

/* local implementation headers */
#include "MeshObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"


class SceneRenderer;
class World;
class ShotPath;
class Player;


class RadarRenderer : public GLDisplayListCreator
{
public:
  virtual	~RadarRenderer();
  RadarRenderer(const SceneRenderer&, World* _world);
  void		setWorld(World* _world);

  void		setControlColor(const GLfloat *color = NULL);

  int		getX() const;
  int		getY() const;
  int		getWidth() const;
  int		getHeight() const;

  void		setShape(int x, int y, int w, int h);
  void		setJammed(bool = true);

  void		setDimming(float newDimming);

  void		render(SceneRenderer&, bool blank, bool observer);

  void		renderFrame(SceneRenderer&);

  void		renderObstacles(bool fastRadar, float range);
  void		renderWalls();
  void		renderBoxPyrMesh();
  void		renderBoxPyrMeshFast(float range);
  void		renderBasesAndTeles();

  int		getFrameTriangleCount() const;

  virtual void	buildGeometry ( GLDisplayList displayList );
  void		clearRadarObjects ( void );

private:
  // no copying
  RadarRenderer(const RadarRenderer&);
  RadarRenderer& operator=(const RadarRenderer&);

  void		drawShot(const ShotPath*);
  void		drawTank(const Player* player, bool allowFancy);
  void		drawFancyTank(const Player* player);
  void		drawHuntLevel(const Player* player,
			      float tankSize, float heightBoxSize);
  void		drawFlag(const float pos[3]);
  void		drawFlagOnTank(const float pos[3]);

  static float	colorScale(const float z, const float h);
  static float	transScale(const float z, const float h);

private:
  World*	world;
  int		x, y;
  int		w, h;
  float		dimming;
  float		ps;
  float		range;
  double	decay;
  GLfloat	teamColor[3];
  bool		smooth;
  bool		jammed;
  bool		colorblind;
  bool		multiSampled;
  bool		useTankModels;
  bool		useTankDimensions;
  int		triangleCount;
  static const float	colorFactor;

  bool		lastFast;

  typedef enum {
    eNone,
    eBoxPyr,
    eMesh,
    eMeshDeathFaces,
    eBoxPyrOutline
  } RadarObjectType;

  typedef std::pair<RadarObjectType,Obstacle*> RadarObject;
  typedef std::map<GLDisplayList,RadarObject> RadarObjectMap;
  RadarObjectMap radarObjectLists;

  void buildBoxPyr ( Obstacle* object );
  // void buildBoxGeo ( BoxBuilding* box );
  // void buildPryGeo ( PyramidBuilding* pyr );
  void buildMeshGeo ( MeshObstacle* mesh, bool deathFaces );
  void buildOutline ( Obstacle* object );
  //   void buildBoxOutline ( const BoxBuilding& box );
  //   void buildPyrOutline ( const PyramidBuilding& pyr );
};

//
// RadarRenderer
//

inline int		RadarRenderer::getX() const
{
  return x;
}

inline int		RadarRenderer::getY() const
{
  return y;
}

inline int		RadarRenderer::getWidth() const
{
  return w;
}

inline int		RadarRenderer::getHeight() const
{
  return h;
}

#endif // BZF_RADAR_RENDERER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
