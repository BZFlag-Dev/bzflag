/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_ARC_H__
#define __CUSTOM_ARC_H__

/* interface header */
#include "WorldFileObstacle.h"

/* local interface headers */
#include "WorldInfo.h"

/* common interface headers */
#include "BzMaterial.h"


class CustomArc : public WorldFileObstacle {
  public:
    CustomArc(bool boxStyle);
    ~CustomArc();
    virtual bool read(const char* cmd, std::istream& input);
    virtual void writeToGroupDef(GroupDefinition*) const;

  private:
    void makePie(bool isCircle, float angle, float rot, float height,
		 float radius, float squish, float texsz[4],
		 WorldInfo* world) const;
    void makeRing(bool isCircle, float angle, float rot, float height,
		  float inrad, float outrad, float squish, float texsz[4],
		  WorldInfo* world) const;
    enum {
      Top,
      Bottom,
      Inside,
      Outside,
      StartFace,
      EndFace,
      MaterialCount
    };
    static const char* sideNames[MaterialCount];

    bool boxStyle;
    int divisions;
    float angle;
    float ratio;
    float texsize[4];
    int phydrv;
    bool useNormals;
    bool smoothBounce;
    BzMaterial materials[MaterialCount];
};


#endif  /* __CUSTOM_ARC_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
