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


#include <stdlib.h>
#include <math.h>
#include "Occluder.h"
#include "SceneNode.h"
#include "Frustum.h"
#include "Intersect.h"
#include "StateDatabase.h"


//////////////////////////////////////////////////////////////////////////////
//
// The Occluder Manager
//

const int OccluderManager::MaxOccluders = MAX_OCCLUDERS;

OccluderManager::OccluderManager()
{
	activeOccluders = 0;
	allowedOccluders = 0;
	for( int i = 0; i < MaxOccluders; i++ )
	{
		occluders[i] = NULL;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


OccluderManager::~OccluderManager()
{
	clear();
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void OccluderManager::clear()
{
	activeOccluders = 0;
	for( int i = 0; i < MaxOccluders; i++ )
	{
		delete occluders[i];
		occluders[i] = NULL;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void OccluderManager::setMaxOccluders( int size )
{
	if( size > MaxOccluders )
	{
		size = MaxOccluders;
	}
	else if( size == 1 )
	{
		size = 2; // minimum of two: one active, one scanning
	}
	else if( size < 0 )
	{
		size = 0;
	}

	allowedOccluders = size;

	if( activeOccluders > allowedOccluders )
	{
		for( int i = allowedOccluders; i < activeOccluders; i++ )
		{
			delete occluders[i];
			occluders[i] = NULL;
		}
		activeOccluders = allowedOccluders;
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


IntersectLevel OccluderManager::occlude( const Extents &exts, unsigned int score )
{
	IntersectLevel level = Outside;

	for( int i = 0; i < activeOccluders; i++ )
	{
		Occluder *oc = occluders[i];
		IntersectLevel tmp = oc->doCullAxisBox( exts );
		if( tmp == Contained )
		{
			oc->addScore( score );
			return Contained;
			// FIXME - this only makes sense for randomly selected
			//	 occluders where there can be overlap
		}
		else if( tmp == Partial )
		{
			level = Partial;
		}
	}

	return level;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool OccluderManager::occludePeek( const Extents &exts )
{
	int i;
	bool result = false;

	// doesn't adjust occluder scores
	for( i = 0; i < activeOccluders; i++ )
	{
		Occluder *oc = occluders[i];
		if( oc->doCullAxisBox( exts ) == Contained )
		{
			result = true;
		}
	}

	return result;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void OccluderManager::update( const Frustum *frustum )
{
	//  const float* e = frustum->getEye ();
	//  printf ("Eye = %f, %f, %f\n", e[0], e[1], e[2]);

	for( int i = 0; i < activeOccluders; i++ )
	{
		if( !occluders[i]->makePlanes( frustum ))
		{
			delete occluders[i];
			activeOccluders--;
			occluders[i] = occluders[activeOccluders];
			occluders[activeOccluders] = NULL;
			i--; // this index is a different occluder, test it again
		}
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static void print_scores( Occluder **olist, int count, const char *str )
{
	return ;
	// FIXME - debugging
	bool first = true;
	for( int i = 0; i < count; i++ )
	{
		int score = olist[i]->getScore();
		if( score > 0 )
		{
			if( first )
			{
				printf( "%s(%i):", str, count );
				first = false;
			}
			printf( " %i", score );
		}
	}
	if( !first )
	{
		putchar( '\n' );
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void OccluderManager::select( const SceneNode *const * list, int listCount )
{
	int oc;

	// see if our limit has changed
	int max = BZDB.evalInt( StateDatabase::BZDB_CULLOCCLUDERS );
	if( max != allowedOccluders )
	{
		setMaxOccluders( max );
	}

	// don't need more occluders then scenenodes
	if( activeOccluders > listCount )
	{
		for( int i = listCount; i < activeOccluders; i++ )
		{
			delete occluders[i];
			occluders[i] = NULL;
		}
		activeOccluders = listCount;
	}

	// remove the useless occluders
	for( oc = 0; oc < activeOccluders; oc++ )
	{
		if(( occluders[oc]->getScore() <= 0 ) || ( oc == ( allowedOccluders - 1 )))
		{
			// always have a spare
			delete occluders[oc];
			activeOccluders--;
			occluders[oc] = occluders[activeOccluders];
			occluders[activeOccluders] = NULL;
		}
	}

	// sort before picking a new occluder
	// they are sorted in descending order  (occluders[0] has the best score)
	if( activeOccluders > 1 )
	{
		sort();
	}

	// pick a new one, this will obviously require a better algorithm,
	// i'm hoping to use a GL query extension (preferably ARB, then NV)
	// could also use the cross-sign value from update(), distance to eye,
	// area of scene node, etc...
	int target = allowedOccluders;
	if( listCount < allowedOccluders )
	{
		target = listCount;
	}
	while( activeOccluders < target )
	{
		const SceneNode *sceneNode = list[rand() % listCount];
		occluders[activeOccluders] = new Occluder( sceneNode );
		if( occluders[activeOccluders]->getVertexCount() == 0 )
		{
			delete occluders[activeOccluders];
			occluders[activeOccluders] = NULL;
			target--; // protect against a list full of nonvalid occluders.
			// could also tally the valid occluder sceneNodes, but
			// that would eat up CPU time in a large list
		}
		else
		{
			activeOccluders++;
		}
	}

	// FIXME  - debugging
	print_scores( occluders, activeOccluders, "prediv" );

	// decrease the scores
	for( oc = 0; oc < activeOccluders; oc++ )
	{
		occluders[oc]->divScore();
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static int compareOccluders( const void *a, const void *b )
{
	Occluder **ptrA = ( Occluder ** )a;
	Occluder **ptrB = ( Occluder ** )b;
	int scoreA = ( *ptrA )->getScore();
	int scoreB = ( *ptrB )->getScore();

	if( scoreA > scoreB )
	{
		return  - 1;
	}
	else if( scoreA < scoreB )
	{
		return  + 1;
	}
	else
	{
		return 0;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void OccluderManager::sort()
{
	qsort( occluders, activeOccluders, sizeof( Occluder* ), compareOccluders );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void OccluderManager::draw()const
{
	for( int i = 0; i < activeOccluders; i++ )
	{
		occluders[i]->draw();
	}
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
// The Occluders
//

const bool Occluder::DrawEdges = true;
const bool Occluder::DrawNormals = false;
const bool Occluder::DrawVertices = true;

Occluder::Occluder( const SceneNode *node )
{
	sceneNode = node;
	planes = NULL;
	vertices = NULL;
	cullScore = 0;

	vertexCount = node->getVertexCount();
	if( !node->isOccluder())
	{
		vertexCount = 0; // used to flag a bad occluder
		return ;
	}
	vertices = new float[vertexCount][3];

	planeCount = vertexCount + 1; // the occluder's plane normal
	planes = new float[planeCount][4];

	// counter-clockwise order as viewed from the front face
	for( int i = 0; i < vertexCount; i++ )
	{
		const float *vertex = node->getVertex( i );
		vertices[i][0] = vertex[0];
		vertices[i][1] = vertex[1];
		vertices[i][2] = vertex[2];
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


Occluder::~Occluder()
{
	// do nothing
	delete []planes;
	delete []vertices;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


IntersectLevel Occluder::doCullAxisBox( const Extents &exts )
{
	return testAxisBoxOcclusion( exts, planes, planeCount );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool Occluder::doCullSceneNode( SceneNode *node )
{
	// FIXME - not yet implemented
	node = node;
	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static bool makePlane( const float *p1, const float *p2, const float *pc, float *r )
{
	// make vectors from points
	float x[3] = 
	{
		p1[0] - pc[0], p1[1] - pc[1], p1[2] - pc[2]
	};
	float y[3] = 
	{
		p2[0] - pc[0], p2[1] - pc[1], p2[2] - pc[2]
	};
	float n[3];

	// cross product to get the normal
	n[0] = ( x[1] *y[2] ) - ( x[2] *y[1] );
	n[1] = ( x[2] *y[0] ) - ( x[0] *y[2] );
	n[2] = ( x[0] *y[1] ) - ( x[1] *y[0] );

	// normalize
	float len = ( n[0] *n[0] ) + ( n[1] *n[1] ) + ( n[2] *n[2] );
	if( len <  + 0.001f )
	{
		return false;
	}
	else
	{
		len = 1.0f / sqrtf( len );
	}
	r[0] = n[0] *len;
	r[1] = n[1] *len;
	r[2] = n[2] *len;

	// finish the plane equation: {rx*px + ry*py + rz+pz + rd = 0}
	r[3] =  - (( pc[0] *r[0] ) + ( pc[1] *r[1] ) + ( pc[2] *r[2] ));

	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool Occluder::makePlanes( const Frustum *frustum )
{
	// occluders can't have their back towards the camera
	const float *eye = frustum->getEye();
	const float *p = sceneNode->getPlane();
	float tmp = ( p[0] *eye[0] ) + ( p[1] *eye[1] ) + ( p[2] *eye[2] ) + p[3];
	if( tmp <  + 0.1f )
	{
		return false;
	}
	// FIXME - store/flag this so we don't have to do it again?

	// make the occluder's normal plane
	const float *plane = sceneNode->getPlane();
	planes[0][0] =  - plane[0];
	planes[0][1] =  - plane[1];
	planes[0][2] =  - plane[2];
	planes[0][3] =  - plane[3];

	// make the edges planes
	for( int i = 0; i < vertexCount; i++ )
	{
		int second = ( i + vertexCount - 1 ) % vertexCount;
		if( !makePlane( vertices[i], vertices[second], eye, planes[i + 1] ))
		{
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void Occluder::draw()const
{
	int v;
	GLfloat colors[5][4] = 
	{
		{
			1.0f, 0.0f, 1.0f, 1.0f
		}

		,  // purple  (occluder's normal)
		{
			1.0f, 0.0f, 0.0f, 1.0f
		}

		,  // red
		{
			0.0f, 1.0f, 0.0f, 1.0f
		}

		,  // green
		{
			0.0f, 0.0f, 1.0f, 1.0f
		}

		,  // blue
		{
			1.0f, 1.0f, 0.0f, 1.0f
		}

		,  // yellow
	};
	const float length = 5.0f;

	glLineWidth( 3.0f );
	glPointSize( 10.0f );
	glEnable( GL_POINT_SMOOTH );

	if( DrawNormals )
	{
		// the tri-wall 'getSphere()' center sucks...
		float center[3];
		for( int a = 0; a < 3; a++ )
		{
			center[a] = 0.0f;
			for( v = 0; v < vertexCount; v++ )
			{
				center[a] += vertices[v][a];
			}
			center[a] = center[a] / ( float )vertexCount;
		}

		float outwards[3];
		outwards[0] = center[0] - ( length *planes[0][0] );
		outwards[1] = center[1] - ( length *planes[0][1] );
		outwards[2] = center[2] - ( length *planes[0][2] );

		// draw the plane normal
		glBegin( GL_LINES );
		glColor4fv( colors[0] );
		glVertex3fv( center );
		glVertex3fv( outwards );
		glEnd();
	}

	// drawn the edges and normals
	if( DrawEdges || DrawNormals )
	{
		for( v = 0; v < vertexCount; v++ )
		{
			float midpoint[3];
			float outwards[3];
			int vn = ( v + 1 ) % vertexCount;
			for( int a = 0; a < 3; a++ )
			{
				midpoint[a] = 0.5f *( vertices[v][a] + vertices[vn][a] );
				outwards[a] = midpoint[a] - ( length *planes[vn + 1][a] );
			}
			glBegin( GL_LINES );
			glColor4fv( colors[( v % 4 ) + 1] );
			if( DrawEdges )
			{
				glVertex3fv( vertices[v] );
				glVertex3fv( vertices[vn] );
			}
			if( DrawNormals )
			{
				glVertex3fv( midpoint );
				glVertex3fv( outwards );
			}
			glEnd();
		}
	}

	// draw some nice vertex points
	if( DrawVertices )
	{
		for( v = 0; v < vertexCount; v++ )
		{
			glBegin( GL_POINTS );
			glColor4fv( colors[( v % 4 ) + 1] );
			glVertex3fv( vertices[v] );
			glEnd();
		}
	}

	glDisable( GL_POINT_SMOOTH );
	glPointSize( 1.0f );
	glLineWidth( 1.0f );

	//  print("Occluder::draw");

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void Occluder::print( const char *string )const
{
	// FIXME - debugging
	printf( "%s: %p, V = %i, P = %i\n", string, sceneNode, vertexCount, planeCount );
	for( int v = 0; v < vertexCount; v++ )
	{
		printf( "  v%i: %f, %f, %f\n", v, vertices[v][0], vertices[v][1], vertices[v][2] );
	}
	for( int p = 0; p < planeCount; p++ )
	{
		printf( "  p%i: %f, %f, %f, %f\n", p, planes[p][0], planes[p][1], planes[p][2], planes[p][3] );
	}

	return ;
}
