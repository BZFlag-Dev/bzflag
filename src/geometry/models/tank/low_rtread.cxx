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

#include "TankSceneNode.h"

#define	doVertex3f	doVertex3f
#define	doNormal3f	doNormal3f

void buildLowRTread ( void )
{
    glShadeModel(GL_FLAT);
      glBegin(GL_TRIANGLE_STRIP);
	doNormal3f(0.020362f, 0.000000f, 0.999793f);
	glTexCoord2f(-0.072f, 0.099f);
	doVertex3f(2.730f, -1.400f, 1.294f);
	glTexCoord2f(0.032f, 0.022f);
	doVertex3f(2.730f, -0.875f, 1.294f);
	glTexCoord2f(0.419f, 0.757f);
	doVertex3f(-2.970f, -1.400f, 1.410f);
	glTexCoord2f(0.511f, 0.690f);
	doVertex3f(-2.970f, -0.875f, 1.410f);
	doNormal3f(-0.885132f, 0.000000f, -0.465341f);
	glTexCoord2f(-0.026f, 0.803f);
	doVertex3f(-2.229f, -1.400f, 0.000f);
	glTexCoord2f(0.690f, 0.279f);
	doVertex3f(-2.229f, -0.875f, 0.000f);
	doNormal3f(0.000000f, 0.000000f, -1.000000f);
	glTexCoord2f(-0.383f, 0.314f);
	doVertex3f(2.597f, -1.400f, 0.000f);
	glTexCoord2f(0.332f, -0.209f);
	doVertex3f(2.597f, -0.875f, 0.000f);
	doNormal3f(0.994712f, 0.000000f, -0.102699f);
	glTexCoord2f(-0.072f, 0.099f);
	doVertex3f(2.730f, -1.400f, 1.294f);
	glTexCoord2f(0.032f, 0.022f);
	doVertex3f(2.730f, -0.875f, 1.294f);
      glEnd();
      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	glTexCoord2f(-0.026f, 0.803f);
	doVertex3f(-2.229f, -1.400f, 0.000f);
	glTexCoord2f(-0.383f, 0.314f);
	doVertex3f(2.597f, -1.400f, 0.000f);
	glTexCoord2f(-0.072f, 0.099f);
	doVertex3f(2.730f, -1.400f, 1.294f);
	glTexCoord2f(0.419f, 0.757f);
	doVertex3f(-2.970f, -1.400f, 1.410f);
      glEnd();
      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	glTexCoord2f(0.690f, 0.279f);
	doVertex3f(-2.229f, -0.875f, 0.000f);
	glTexCoord2f(0.511f, 0.690f);
	doVertex3f(-2.970f, -0.875f, 1.410f);
	glTexCoord2f(0.032f, 0.022f);
	doVertex3f(2.730f, -0.875f, 1.294f);
	glTexCoord2f(0.332f, -0.209f);
	doVertex3f(2.597f, -0.875f, 0.000f);
      glEnd();
}
/*
 * Local Variables: ***
 * mode:C ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
