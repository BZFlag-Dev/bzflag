/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * RadarRenderer:
 *	Encapsulates drawing a radar
 */

#ifndef	BZF_RADAR_RENDERER_H
#define	BZF_RADAR_RENDERER_H

#include "common.h"
#include "bzfgl.h"
#include "Obstacle.h"


class SceneRenderer;
class World;
class ShotPath;

class RadarRenderer {
  public:
			RadarRenderer(const SceneRenderer&,
						const World& world);

    void		setControlColor(const GLfloat *color = NULL);

    int			getX() const;
    int			getY() const;
    int			getWidth() const;
    int			getHeight() const;

    void		setShape(int x, int y, int w, int h);
    void		setJammed(bool = true);

    void		render(SceneRenderer&, bool blank = false);
    
    void		renderObstacles(bool smoothing,
                                        bool fastRadar,
                                        float range);
    void		renderWalls();
    void		renderBoxPyrMesh(bool smoothing);
    void		renderBoxPyrMeshFast(bool smoothing, float range);
    void		renderBasesAndTeles();

  private:
    // no copying
    RadarRenderer(const RadarRenderer&);
    RadarRenderer&	operator=(const RadarRenderer&);

    void		drawShot(const ShotPath*);
    void		drawTank(float x, float y, float z);
    void		drawFlag(float x, float y, float z);
    void		drawFlagOnTank(float x, float y, float z);

    static float	colorScale(const float z, const float h);
    static float	transScale(const float z, const float h);

  private:
    const World&	world;
    int			x, y;
    int			w, h;
    float		ps;
    float		range;
    bool		smooth;
    bool		jammed;
    double		decay;
    GLfloat		teamColor[3];
    static const float	colorFactor;
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
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

