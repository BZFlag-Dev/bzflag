/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_VIEWITEMRADAR_H
#define BZF_VIEWITEMRADAR_H

#include "View.h"

class Obstacle;
class OpenGLTexture;

class ViewItemRadar : public View {
public:
	ViewItemRadar();

protected:
	virtual ~ViewItemRadar();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	bool				makeNoise();
	void				makeNoiseTexture();
	void				drawObstacles(bool enhanced);
	void				drawSmoothObstacles(bool enhanced);
	void				drawObstacle(const Obstacle&,
								const float* color, bool enhanced);
	void				drawTank(float x, float y, float z, float minsize);
	void				drawFlag(float x, float y, float z, float minsize);

	static float		colorScale(const Obstacle& o, bool enhanced);
	static float		transScale(const Obstacle& o);

	void				initContext(bool);
	static void			initContextCB(bool, void*);

private:
	bool				smooth;
	double				decay;
	unsigned char*		noise;
	OpenGLTexture*		noiseTexture;
	static const float	colorFactor;
	static const float	transFactor;
};

class ViewItemRadarReader : public ViewTagReader {
public:
	ViewItemRadarReader();
	virtual ~ViewItemRadarReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(XMLTree::iterator);

private:
	ViewItemRadar*		item;
};

#endif
// ex: shiftwidth=4 tabstop=4
