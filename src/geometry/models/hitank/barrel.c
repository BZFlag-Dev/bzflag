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

      glShadeModel(GL_FLAT);
      glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1.000000f, 0.000000f, 0.000000f);
	glVertex3f(4.940f, 0.0f, 1.410f);
	glVertex3f(4.940f, 0.089f, 1.440f);
	glVertex3f(4.940f, 0.126f, 1.530f);
	glVertex3f(4.940f, 0.089f, 1.620f);
	glVertex3f(4.940f, 0.0f, 1.660f);
	glVertex3f(4.940f, -0.09f, 1.620f);
	glVertex3f(4.940f, -0.126f, 1.530f);
	glVertex3f(4.940f, -0.09f, 1.440f);
      glEnd();
      glShadeModel(GL_SMOOTH);
      glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0.015873f, -0.999874f, 0.000000f);
	glVertex3f(1.570f, -0.18f, 1.530f);
	glVertex3f(4.940f, -0.126f, 1.530f);
	glNormal3f(0.016407f, -0.704331f, 0.709682f);
	glVertex3f(1.570f, -0.128f, 1.660f);
	glNormal3f(0.016398f, -0.718536f, 0.695296f);
	glVertex3f(4.940f, -0.09f, 1.620f);
	glNormal3f(0.014835f, -0.000267f, 0.999890f);
	glVertex3f(1.570f, 0.0f, 1.710f);
	glVertex3f(4.940f, 0.0f, 1.660f);
	glNormal3f(0.016367f, 0.702262f, 0.711730f);
	glVertex3f(1.570f, 0.126f, 1.660f);
	glNormal3f(0.016356f, 0.717625f, 0.696238f);
	glVertex3f(4.940f, 0.089f, 1.620f);
	glNormal3f(0.016022f, 0.999872f, 0.000000f);
	glVertex3f(1.570f, 0.18f, 1.530f);
	glVertex3f(4.940f, 0.126f, 1.530f);
	glNormal3f(0.016367f, 0.702262f, -0.711730f);
	glVertex3f(1.570f, 0.126f, 1.400f);
	glNormal3f(0.016369f, 0.683073f, -0.730167f);
	glVertex3f(4.940f, 0.089f, 1.440f);
	glNormal3f(0.017801f, -0.000267f, -0.999841f);
	glVertex3f(1.570f, 0.0f, 1.350f);
	glVertex3f(4.940f, 0.0f, 1.410f);
	glNormal3f(0.016407f, -0.704331f, -0.709683f);
	glVertex3f(1.570f, -0.128f, 1.400f);
	glNormal3f(0.016410f, -0.683912f, -0.729379f);
	glVertex3f(4.940f, -0.09f, 1.440f);
	glNormal3f(0.015873f, -0.999874f, 0.000000f);
	glVertex3f(1.570f, -0.18f, 1.530f);
	glVertex3f(4.940f, -0.126f, 1.530f);
      glEnd();
// ex: shiftwidth=2 tabstop=8
