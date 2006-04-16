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

#include "common.h"

/* interface header */
#include "CustomPyramid.h"

/* system implementation headers */
#include <math.h>

/* common implementation headers */
#include "PyramidBuilding.h"
#include "StateDatabase.h"
#include "ObstacleMgr.h"


CustomPyramid::CustomPyramid()
{
	size[0] = size[1] = BZDB.eval( StateDatabase::BZDB_PYRBASE );
	size[2] = BZDB.eval( StateDatabase::BZDB_PYRHEIGHT );
	flipZ = false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool CustomPyramid::read( const char *cmd, std::istream &input )
{
	if( strcasecmp( cmd, "flipz" ) == 0 )
		flipZ = true;
	else
		return WorldFileObstacle::read( cmd, input );
	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void CustomPyramid::writeToGroupDef( GroupDefinition *groupdef )const
{
	bool flipit = flipZ;
	if( size[2] < 0.0f )
	{
		flipit = true;
	}

	PyramidBuilding *pyr = new PyramidBuilding( pos, rotation, fabsf( size[0] ), fabsf( size[1] ), fabsf( size[2] ), driveThrough, shootThrough );
	if( flipit )
	{
		pyr->setZFlip();
	}

	groupdef->addObstacle( pyr );
}

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
