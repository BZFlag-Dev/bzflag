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

float sideVerts[][3] = {  {2.800000f, 0.878000f, 0.829000f},
                        {2.820000f, 0.878000f, 0.716000f},
                        {2.680000f, 0.877000f, 0.684000f},
                        {-1.460000f, 0.877000f, 1.130000f},
                        {-2.740000f, 0.877000f, 0.528000f},
                        {-2.900000f, 0.877000f, 1.240000f},
                        {2.570000f, 0.877000f, 0.990000f},
                        {2.170000f, 0.877000f, 1.010000f},
                        {2.610000f, 0.877000f, 0.408000f},
                        {1.810000f, 0.877000f, 0.250000f},
                        {-1.620000f, 0.877000f, 0.25000f},
                        {1.760000f, 0.877000f, 1.130000f},
                        {1.760000f, 0.877000f, 1.130000f},
                        {1.760000f, 0.877000f, 1.130000f},
                        {2.800000f, -0.876000f, 0.829000f},
                        {2.680000f, -0.877000f, 0.684000f},
                        {2.820000f, -0.877000f, 0.716000f},
                        {-2.740000f, -0.877000f, 0.528000f},
                        {-1.460000f, -0.877000f, 1.130000f},
                        {-2.900000f, -0.877000f, 1.240000f},
                        {2.570000f, -0.877000f, 0.990000f},
                        {2.170000f, -0.877000f, 1.010000f},
                        {2.610000f, -0.877000f, 0.408000f},
                        {1.810000f, -0.877000f, 0.250000f},
                        {-1.620000f, -0.877000f, 0.250000f},
                        {1.760000f, -0.877000f, 1.130000f}};
  float sideUVs[][2] =  {{0.996503f, 0.331452f},
                        {1.000000f, 0.422581f},
                        {0.975524f, 0.448387f},
                        {0.251748f, 0.088710f},
                        {0.027972f, 0.574194f},
                        {0.000000f, 0.000000f},
                        {0.956294f, 0.201613f},
                        {0.886364f, 0.185484f},
                        {0.963287f, 0.670968f},
                        {0.823427f, 1.000000f},
                        {0.223776f, 1.000000f},
                        {0.814685f, 0.088710f}};

float sideNormals[][3] ={{-0.006338f, 0.999979f, -0.001652f},
                        {-0.003169f, 0.999995f, -0.000826f},
                        {0.000000f, 1.000000f, 0.000000f},
                        {0.000000f, 0.000000f, 0.000000f},
                        {0.001933f, -0.999984f, 0.005297f},
                        {0.000966f, -0.999996f, 0.002648f},
                        {-0.001944f, -0.999962f, 0.008505f},
                        {0.000000f, -1.000000f, 0.000000f},
                        {0.002905f, -0.999995f, 0.001044f}};

void buildHighBody ( void )
{
  glShadeModel(GL_FLAT);

  int   normIndex = 0;
  int   tcIndex = 0;
  int   vertIndex = 0;

  // side walls
  glBegin(GL_TRIANGLES);

    //  1/1/1 2/2/1 3/3/2
    vertIndex = 1;tcIndex = 1;normIndex = 1;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 2;tcIndex = 2;normIndex = 1;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 3;tcIndex = 3;normIndex = 2;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    //  4/4/3 5/5/3 6/6/3
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 5;tcIndex = 5;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 6;tcIndex = 6;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 1/1/1 3/3/2 7/7/2
    vertIndex = 1;tcIndex = 1;normIndex = 1;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 3;tcIndex = 3;normIndex = 2;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 7;tcIndex = 7;normIndex = 2;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 7/7/2 3/3/2 8/8/3
    vertIndex = 7;tcIndex = 7;normIndex = 2;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 3;tcIndex = 3;normIndex = 2;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 8;tcIndex = 8;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 3/3/2 9/9/3 8/8/3
    vertIndex = 3;tcIndex = 3;normIndex = 2;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 9;tcIndex = 9;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 8;tcIndex = 8;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 9/9/3 10/10/3 8/8/3
    vertIndex = 9;tcIndex = 9;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 10;tcIndex = 10;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 8;tcIndex = 8;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 11/11/3 5/5/3 4/4/3
    vertIndex = 11;tcIndex = 11;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 5;tcIndex = 5;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 10/10/3 11/11/3 4/4/3
    vertIndex = 10;tcIndex = 10;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 11;tcIndex = 11;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 8/8/3 10/10/3 4/4/3
    vertIndex = 8;tcIndex = 8;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 10;tcIndex = 10;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 12/12/3 8/8/3 4/4/3
    vertIndex = 12;tcIndex = 12;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 8;tcIndex = 8;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 13/12/4 12/12/3 4/4/3
    vertIndex = 13;tcIndex = 12;normIndex = 4;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 12;tcIndex = 12;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 14/12/4 13/12/4 4/4/3
    vertIndex = 14;tcIndex = 12;normIndex = 4;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 13;tcIndex = 12;normIndex = 4;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 4;tcIndex = 4;normIndex = 3;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    glEnd();

    // 15/1/5 16/3/6 17/2/7
    vertIndex = 15;tcIndex = 1;normIndex = 5;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 16;tcIndex = 3;normIndex = 6;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 17;tcIndex = 2;normIndex = 7;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 18/5/8 19/4/8 20/6/8
    vertIndex = 18;tcIndex = 5;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 19;tcIndex = 4;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 20;tcIndex = 6;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 16/3/6 15/1/5 21/7/9
    vertIndex = 16;tcIndex = 3;normIndex = 6;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 15;tcIndex = 1;normIndex = 5;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 21;tcIndex = 7;normIndex = 9;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 16/3/6 21/7/9 22/8/8
    vertIndex = 16;tcIndex = 3;normIndex = 6;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 21;tcIndex = 7;normIndex = 9;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 22;tcIndex = 8;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 23/9/8 16/3/6 22/8/8
    vertIndex = 23;tcIndex = 9;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 16;tcIndex = 3;normIndex = 6;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 22;tcIndex = 8;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 24/10/8 23/9/8 22/8/8
    vertIndex = 24;tcIndex = 10;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 23;tcIndex = 9;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 22;tcIndex = 8;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 18/5/8 25/11/8 19/4/8
    vertIndex = 18;tcIndex = 5;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 25;tcIndex = 11;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 19;tcIndex = 4;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 24/10/8 22/8/8 26/12/8
    vertIndex = 24;tcIndex = 10;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 22;tcIndex = 8;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 26;tcIndex = 12;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 24/10/8 26/12/8 19/4/8
    vertIndex = 26;tcIndex = 12;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 19;tcIndex = 12;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 24;tcIndex = 10;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    // 25/11/8 24/10/8 19/4/8
    vertIndex = 19;tcIndex = 4;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 24;tcIndex = 10;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);
    vertIndex = 25;tcIndex = 11;normIndex = 8;
    glNormal3f(sideNormals[normIndex-1][0], sideNormals[normIndex-1][1], sideNormals[normIndex-1][2]);
    glTexCoord2f(sideUVs[tcIndex-1][0],sideUVs[tcIndex-1][1]);
    glVertex3f(sideVerts[vertIndex-1][0], sideVerts[vertIndex-1][1], sideVerts[vertIndex-1][2]);

    glEnd();

    glShadeModel(GL_FLAT);
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.984696f, 0.000000f, 0.174282f);
    glTexCoord2f(1.210f, 2.290f);
    glVertex3f(2.820f, -0.877f, 0.716f);
    glTexCoord2f(0.669f, 2.040f);
    glVertex3f(2.820f, 0.878f, 0.716f);
    glTexCoord2f(1.210f, 2.280f);
    glVertex3f(2.800f, -0.876f, 0.829f);
    glTexCoord2f(0.672f, 2.030f);
    glVertex3f(2.800f, 0.878f, 0.829f);
    glNormal3f(0.573462f, 0.000000f, 0.819232f);
    glTexCoord2f(1.240f, 2.210f);
    glVertex3f(2.570f, -0.877f, 0.990f);
    glTexCoord2f(0.705f, 1.960f);
    glVertex3f(2.570f, 0.877f, 0.990f);
    glNormal3f(0.049938f, 0.000000f, 0.998752f);
    glTexCoord2f(1.300f, 2.090f);
    glVertex3f(2.170f, -0.877f, 1.010f);
    glTexCoord2f(0.763f, 1.840f);
    glVertex3f(2.170f, 0.877f, 1.010f);
    glNormal3f(0.280899f, 0.000000f, 0.959737f);
    glTexCoord2f(1.360f, 1.970f);
    glVertex3f(1.760f, -0.877f, 1.130f);
    glTexCoord2f(0.822f, 1.710f);
    glVertex3f(1.760f, 0.877f, 1.130f);
    glNormal3f(0.000000f, 0.000000f, 1.000000f);
    glTexCoord2f(1.820f, 0.981f);
    glVertex3f(-1.460f, -0.877f, 1.130f);
    glTexCoord2f(1.280f, 0.729f);
    glVertex3f(-1.460f, 0.877f, 1.130f);
    glNormal3f(0.076167f, 0.000000f, 0.997095f);
    glTexCoord2f(2.030f, 0.541f);
    glVertex3f(-2.900f, -0.877f, 1.240f);
    glTexCoord2f(1.490f, 0.289f);
    glVertex3f(-2.900f, 0.877f, 1.240f);
    glNormal3f(-0.975668f, 0.000000f, -0.219251f);
    glTexCoord2f(2.000f, 0.590f);
    glVertex3f(-2.740f, -0.877f, 0.528f);
    glTexCoord2f(1.470f, 0.338f);
    glVertex3f(-2.740f, 0.877f, 0.528f);
    glNormal3f(-0.426419f, 0.000000f, -0.904526f);
    glTexCoord2f(1.840f, 0.932f);
    glVertex3f(-1.620f, -0.877f, 0.250f);
    glTexCoord2f(1.310f, 0.680f);
    glVertex3f(-1.620f, 0.877f, 0.250f);
    glNormal3f(0.000000f, 0.000000f, -1.000000f);
    glTexCoord2f(1.350f, 1.980f);
    glVertex3f(1.810f, -0.877f, 0.250f);
    glTexCoord2f(0.815f, 1.730f);
    glVertex3f(1.810f, 0.877f, 0.250f);
    glNormal3f(0.454326f, 0.000000f, -0.890835f);
    glTexCoord2f(1.240f, 2.230f);
    glVertex3f(2.610f, -0.877f, 0.408f);
    glTexCoord2f(0.700f, 1.970f);
    glVertex3f(2.610f, 0.877f, 0.408f);
    glNormal3f(0.969310f, 0.000000f, -0.245840f);
    glTexCoord2f(1.230f, 2.250f);
    glVertex3f(2.680f, -0.877f, 0.684f);
    glTexCoord2f(0.690f, 2.000f);
    glVertex3f(2.680f, 0.877f, 0.684f);
    glNormal3f(0.222825f, 0.000000f, -0.974858f);
    glTexCoord2f(1.210f, 2.290f);
    glVertex3f(2.820f, -0.877f, 0.716f);
    glTexCoord2f(0.669f, 2.040f);
    glVertex3f(2.820f, 0.878f, 0.716f);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(1.540f, 0.341f);
    glVertex3f(-2.820f, 0.686f, 1.070f);
    glTexCoord2f(1.580f, 0.261f);
    glVertex3f(-3.080f, 0.686f, 1.070f);
    glTexCoord2f(1.540f, 0.341f);
    glVertex3f(-2.820f, 0.686f, 1.170f);
    glTexCoord2f(1.580f, 0.261f);
    glVertex3f(-3.080f, 0.686f, 1.170f);
    glNormal3f(0.000000f, 0.000000f, 1.000000f);
    glTexCoord2f(1.640f, 0.387f);
    glVertex3f(-2.820f, 0.367f, 1.170f);
    glTexCoord2f(1.670f, 0.307f);
    glVertex3f(-3.080f, 0.367f, 1.170f);
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(1.640f, 0.387f);
    glVertex3f(-2.820f, 0.367f, 1.070f);
    glTexCoord2f(1.670f, 0.307f);
    glVertex3f(-3.080f, 0.367f, 1.070f);
    glNormal3f(0.000000f, 0.000000f, -1.000000f);
    glTexCoord2f(1.540f, 0.341f);
    glVertex3f(-2.820f, 0.686f, 1.070f);
    glTexCoord2f(1.580f, 0.261f);
    glVertex3f(-3.080f, 0.686f, 1.070f);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(-1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(1.580f, 0.261f);
    glVertex3f(-3.080f, 0.686f, 1.170f);
    glTexCoord2f(1.580f, 0.261f);
    glVertex3f(-3.080f, 0.686f, 1.070f);
    glTexCoord2f(1.670f, 0.307f);
    glVertex3f(-3.080f, 0.367f, 1.170f);
    glTexCoord2f(1.670f, 0.307f);
    glVertex3f(-3.080f, 0.367f, 1.070f);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.000000f, 1.000000f, 0.000000f);
    glTexCoord2f(1.780f, 0.445f);
    glVertex3f(-2.840f, -0.084f, 1.070f);
    glTexCoord2f(1.810f, 0.366f);
    glVertex3f(-3.100f, -0.084f, 1.070f);
    glTexCoord2f(1.780f, 0.445f);
    glVertex3f(-2.840f, -0.084f, 1.170f);
    glTexCoord2f(1.810f, 0.366f);
    glVertex3f(-3.100f, -0.084f, 1.170f);
    glNormal3f(0.000000f, 0.000000f, 1.000000f);
    glTexCoord2f(2.020f, 0.559f);
    glVertex3f(-2.840f, -0.877f, 1.170f);
    glTexCoord2f(2.060f, 0.480f);
    glVertex3f(-3.100f, -0.877f, 1.170f);
    glNormal3f(0.000000f, -1.000000f, 0.000000f);
    glTexCoord2f(2.020f, 0.559f);
    glVertex3f(-2.840f, -0.877f, 1.070f);
    glTexCoord2f(2.060f, 0.480f);
    glVertex3f(-3.100f, -0.877f, 1.070f);
    glNormal3f(0.000000f, 0.000000f, -1.000000f);
    glTexCoord2f(1.780f, 0.445f);
    glVertex3f(-2.840f, -0.084f, 1.070f);
    glTexCoord2f(1.810f, 0.366f);
    glVertex3f(-3.100f, -0.084f, 1.070f);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(-1.000000f, 0.000000f, 0.000000f);
    glTexCoord2f(1.810f, 0.366f);
    glVertex3f(-3.100f, -0.084f, 1.170f);
    glTexCoord2f(1.810f, 0.366f);
    glVertex3f(-3.100f, -0.084f, 1.070f);
    glTexCoord2f(2.060f, 0.480f);
    glVertex3f(-3.100f, -0.877f, 1.170f);
    glTexCoord2f(2.060f, 0.480f);
    glVertex3f(-3.100f, -0.877f, 1.070f);
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
