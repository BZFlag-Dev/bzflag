/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_BOX_H__
#define __CUSTOM_BOX_H__

/* interface header */
#include "WorldFileObstacle.h"

/* local interface headers */
#include "WorldInfo.h"

/* common interface headers */
#include "BzMaterial.h"


class CustomBox : public WorldFileObstacle {
  public:
    CustomBox();
    ~CustomBox();
    virtual bool read(const char* cmd, std::istream& input);
    virtual void writeToGroupDef(GroupDefinition*) const;

  private:
    enum {
      XP = 0,
      XN,
      YP,
      YN,
      ZP,
      ZN,
      FaceCount
    };

    bool isOldBox;

    int phydrv[FaceCount];
    float texsize[FaceCount][2];
    float texoffset[FaceCount][2];
    unsigned char drivethrough[FaceCount];
    unsigned char shootthrough[FaceCount];
    BzMaterial materials[FaceCount];

    static const char* faceNames[FaceCount];
};


#endif  /* __CUSTOM_BOX_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
