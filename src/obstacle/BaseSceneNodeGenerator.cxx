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

#include "BaseSceneNodeGenerator.h"
#include "BaseBuilding.h"
#include "bzfgl.h"
#include "QuadWallSceneNode.h"


BaseSceneNodeGenerator::BaseSceneNodeGenerator( const BaseBuilding *_base ): base( _base )
{
	// do nothing
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

BaseSceneNodeGenerator::~BaseSceneNodeGenerator()
{
	// do nothing
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

WallSceneNode *BaseSceneNodeGenerator::getNextNode( float uRepeats, float vRepeats, bool lod )
{
	const float height = base->getHeight() + base->getPosition()[2];
	if( getNodeNumber() >= 1 && height == 0 )
		return NULL;
	if( getNodeNumber() >= 6 )
		return NULL;
	GLfloat bPoint[3], sCorner[3], tCorner[3];
	if( height == 0 )
	{
		incNodeNumber();
		base->getCorner( 0, bPoint );
		base->getCorner( 3, tCorner );
		base->getCorner( 1, sCorner );
	}
	else
	{
		switch( incNodeNumber())
		{
			case 1:
				// This is the top polygon
				base->getCorner( 4, bPoint );
				base->getCorner( 5, sCorner );
				base->getCorner( 7, tCorner );
				break;
			case 2:
				base->getCorner( 0, bPoint );
				base->getCorner( 1, sCorner );
				base->getCorner( 4, tCorner );
				break;
			case 3:
				base->getCorner( 1, bPoint );
				base->getCorner( 2, sCorner );
				base->getCorner( 5, tCorner );
				break;
			case 4:
				base->getCorner( 2, bPoint );
				base->getCorner( 3, sCorner );
				base->getCorner( 6, tCorner );
				break;
			case 5:
				base->getCorner( 3, bPoint );
				base->getCorner( 0, sCorner );
				base->getCorner( 7, tCorner );
				break;
			case 6:
				// This is the bottom polygon
				if( base->getPosition()[2] > 0.0f )
				{
					// Only generate if above ground level
					base->getCorner( 0, bPoint );
					base->getCorner( 3, sCorner );
					base->getCorner( 1, tCorner );
				}
				else
					return NULL;
				break;

		}
	}
	GLfloat color[4];
	switch( base->getTeam())
	{
		case 1:
			color[0] = 0.7f;
			color[1] = 0.0f;
			color[2] = 0.0f;
			break;
		case 2:
			color[0] = 0.0f;
			color[1] = 0.7f;
			color[2] = 0.0f;
			break;
		case 3:
			color[0] = 0.0f;
			color[1] = 0.0f;
			color[2] = 0.7f;
			break;
		case 4:
			color[0] = 0.7f;
			color[1] = 0.0f;
			color[2] = 0.7f;
			break;
	}
	color[3] = 1.0;

	GLfloat sEdge[3];
	GLfloat tEdge[3];
	sEdge[0] = sCorner[0] - bPoint[0];
	sEdge[1] = sCorner[1] - bPoint[1];
	sEdge[2] = sCorner[2] - bPoint[2];
	tEdge[0] = tCorner[0] - bPoint[0];
	tEdge[1] = tCorner[1] - bPoint[1];
	tEdge[2] = tCorner[2] - bPoint[2];

	WallSceneNode *retval = new QuadWallSceneNode( bPoint, sEdge, tEdge, uRepeats, vRepeats, lod );
	retval->setColor( color );
	return retval;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
