/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
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
* GrassRenderer:
*	3D Grass rendering
*
*/

#ifndef	BZF_GRASS_RENDERER_H
#define	BZF_GRASS_RENDERER_H

#include "common.h"

/* system headers */
#include <string>
#include <vector>
#include <map>

/* common interface headers */
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"

class GrassRenderer {
public:
	GrassRenderer();
	~GrassRenderer();

	// called once to setup grass state, load lists and materials and stuff
	void init(void);

	// called to draw the grass for the current frame
	void draw(const SceneRenderer& sr);

	// called when the GL lists need to be deleted
	void freeContext(void);

	// called when the GL lists need to be remade
	void rebuildContext(void);

protected:
	void buildGrassList(bool draw = false);

	OpenGLGState				grassGState;
	GLuint					grassList;
	GLfloat					grassHeight;
};

#endif // BZF_GRASS_RENDERER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
