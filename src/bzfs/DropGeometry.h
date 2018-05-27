/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __DROP_GEOMETRY_H__
#define __DROP_GEOMETRY_H__


class WorldInfo;


namespace DropGeometry
{

bool dropFlag (float pos[3], float minZ, float maxZ);
bool dropPlayer (float pos[3], float minZ, float maxZ);
bool dropTeamFlag (float pos[3], float minZ, float maxZ, int team);
}


#endif  /* __DROP_GEOMETRY_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 4***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
