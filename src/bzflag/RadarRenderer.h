/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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

class SceneRenderer;
class World;
class ShotPath;

class RadarRenderer {
  public:
			RadarRenderer(const SceneRenderer&,
						const World& world);
			~RadarRenderer();

    int			getX() const;
    int			getY() const;
    int			getWidth() const;
    int			getHeight() const;
    float		getRange() const;

    void		setShape(int x, int y, int w, int h);
    void		setRange(float range);
    void		setJammed(boolean = True);

    void		render(SceneRenderer&, boolean blank = False);
    void		freeList();
    void		makeList(boolean);

  private:
    // no copying
			RadarRenderer(const RadarRenderer&);
    RadarRenderer&	operator=(const RadarRenderer&);

    void		makeNoise();
    void		drawShot(const ShotPath*);
    void		drawTank(float x, float y, float z, float ps);

  private:
    const World&	world;
    int			x, y;
    int			w, h;
    float		range;
    GLfloat		background[4];
    boolean		blend;
    boolean		smooth;
    boolean		jammed;
    double		decay;
    GLuint		list;
    unsigned char*	noise;
    GLenum		noiseFormat;
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
