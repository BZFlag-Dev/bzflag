/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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
#include "Obstacle.h"


class SceneRenderer;
class World;
class ShotPath;

class RadarRenderer {
  public:
			RadarRenderer(const SceneRenderer&,
						const World& world);
			~RadarRenderer();

    void		setControlColor(const GLfloat *color = NULL);

    int			getX() const;
    int			getY() const;
    int			getWidth() const;
    int			getHeight() const;
    float		getRange() const;

    void		setShape(int x, int y, int w, int h);
    void		setRange(float range);
    void		setJammed(bool = true);
    void		toggleFlags();

    void		render(SceneRenderer&, bool blank = false);
    void		freeList();
    void		makeList(bool, SceneRenderer&);

  private:
    // no copying
    RadarRenderer(const RadarRenderer&);
    RadarRenderer&	operator=(const RadarRenderer&);

    bool		makeNoise();
    void		makeNoiseTexture();
    void		drawShot(const ShotPath*);
    void		drawTank(float x, float y, float z);
    void		drawFlag(float x, float y, float z, bool drawAlways = false);
    void		drawFlagOnTank(float x, float y, float z);

    static float	colorScale(const float z, const float h, bool enhanced);
    static float	transScale(const Obstacle& o);

    void		doInitContext();
    static void		initContext(void*);

  private:
    const World&	world;
    int			x, y;
    int			w, h;
    float		ps;
    float		range;
    bool		blend;
    bool		smooth;
    bool		jammed;
    bool		showFlags;
    double		decay;
    GLuint		list;
    GLfloat		teamColor[3];
    unsigned char	*noise;
    OpenGLTexture	*noiseTexture;
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

inline float		RadarRenderer::getRange() const
{
  return range;
}

#endif // BZF_RADAR_RENDERER_H
// ex: shiftwidth=2 tabstop=8
