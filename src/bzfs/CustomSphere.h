/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_SPHERE_H__
	#define __CUSTOM_SPHERE_H__

/* interface header */
	#include "WorldFileObstacle.h"

/* local interface headers */
	#include "WorldInfo.h"

/* common interface headers */
	#include "BzMaterial.h"


class CustomSphere: public WorldFileObstacle
{
public:
	CustomSphere();
	~CustomSphere();
	virtual bool read( const char *cmd, std::istream &input );
	virtual void writeToGroupDef( GroupDefinition* )const;

private:
	enum
	{
		Edge, Bottom, MaterialCount
	};
	static const char *sideNames[MaterialCount];

	int divisions;
	float texsize[2];
	bool hemisphere;
	int phydrv;
	bool useNormals;
	bool smoothBounce;
	BzMaterial materials[MaterialCount];
};


#endif /* __CUSTOM_SPHERE_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
