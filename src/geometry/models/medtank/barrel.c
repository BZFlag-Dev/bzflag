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

      glShadeModel(GL_SMOOTH);
      glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(1.570f, -0.18f, 1.530f);
	glVertex3f(4.940f, -0.126f, 1.530f);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.570f, 0.0f, 1.710f);
	glVertex3f(4.940f, 0.0f, 1.660f);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.570f, 0.18f, 1.530f);
	glVertex3f(4.940f, 0.126f, 1.530f);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(1.570f, 0.0f, 1.350f);
	glVertex3f(4.940f, 0.0f, 1.410f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(1.570f, -0.18f, 1.530f);
	glVertex3f(4.940f, -0.126f, 1.530f);
      glEnd();
      glShadeModel(GL_FLAT);
      glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1.000000f, 0.000000f, 0.000000f);
	glVertex3f(4.940f, 0.0f, 1.410f);
	glVertex3f(4.940f, 0.126f, 1.530f);
	glVertex3f(4.940f, 0.0f, 1.660f);
	glVertex3f(4.940f, -0.126f, 1.530f);
      glEnd();
// ex: shiftwidth=2 tabstop=8
