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

// bzflag common header
#include "common.h"

// interface header
#include "MeshPolySceneNode.h"

// system headers
#include <assert.h>
#include <math.h>

// common implementation headers
#include "Intersect.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

// FIXME - no tesselation is done on for shot lighting


//
// MeshPolySceneNode::Geometry
//

MeshPolySceneNode::Geometry::Geometry( MeshPolySceneNode *node, const GLfloat3Array &_vertices, const GLfloat3Array &_normals, const GLfloat2Array &_texcoords, const GLfloat *_normal ): vertices( _vertices ), normals( _normals ), texcoords( _texcoords )
{
	sceneNode = node;
	normal = _normal;
	style = 0;
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


MeshPolySceneNode::Geometry::~Geometry()
{
	// do nothing
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

inline void MeshPolySceneNode::Geometry::drawV()const
{
	const int count = vertices.getSize();
	glBegin( GL_TRIANGLE_FAN );
	for( int i = 0; i < count; i++ )
	{
		glVertex3fv( vertices[i] );
	}
	glEnd();
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


inline void MeshPolySceneNode::Geometry::drawVT()const
{
	const int count = vertices.getSize();
	glBegin( GL_TRIANGLE_FAN );
	for( int i = 0; i < count; i++ )
	{
		glTexCoord2fv( texcoords[i] );
		glVertex3fv( vertices[i] );
	}
	glEnd();
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


inline void MeshPolySceneNode::Geometry::drawVN()const
{
	const int count = vertices.getSize();
	glBegin( GL_TRIANGLE_FAN );
	for( int i = 0; i < count; i++ )
	{
		glNormal3fv( normals[i] );
		glVertex3fv( vertices[i] );
	}
	glEnd();
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


inline void MeshPolySceneNode::Geometry::drawVTN()const
{
	const int count = vertices.getSize();
	glBegin( GL_TRIANGLE_FAN );
	for( int i = 0; i < count; i++ )
	{
		glTexCoord2fv( texcoords[i] );
		glNormal3fv( normals[i] );
		glVertex3fv( vertices[i] );
	}
	glEnd();
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::Geometry::render()
{
	sceneNode->setColor();

	if( normals.getSize() != 0 )
	{
		if( style >= 2 )
		{
			drawVTN();
		}
		else
		{
			drawVN();
		}
	}
	else
	{
		glNormal3fv( normal );
		if( style >= 2 )
		{
			drawVT();
		}
		else
		{
			drawV();
		}
	}

	addTriangleCount( vertices.getSize() - 2 );
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::Geometry::renderRadar()
{
	drawV();
	addTriangleCount( vertices.getSize() - 2 );
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::Geometry::renderShadow()
{
	drawV();
	addTriangleCount( vertices.getSize() - 2 );
	return ;
}


//
// MeshPolySceneNode
//

MeshPolySceneNode::MeshPolySceneNode( const float _plane[4], bool _noRadar, bool _noShadow, const GLfloat3Array &vertices, const GLfloat3Array &normals, const GLfloat2Array &texcoords ): node( this, vertices, normals, texcoords, plane )
{
	int i, j;
	const int count = vertices.getSize();
	assert( texcoords.getSize() == count );
	assert(( normals.getSize() == 0 ) || ( normals.getSize() == count ));

	setPlane( _plane );

	noRadar = _noRadar || ( plane[2] <= 0.0f ); // pre-cull if we can
	noShadow = _noShadow;

	// choose axis to ignore (the one with the largest normal component)
	int ignoreAxis;
	const GLfloat *normal = getPlane();
	if( fabsf( normal[0] ) > fabsf( normal[1] ))
	{
		if( fabsf( normal[0] ) > fabsf( normal[2] ))
		{
			ignoreAxis = 0;
		}
		else
		{
			ignoreAxis = 2;
		}
	}
	else
	{
		if( fabsf( normal[1] ) > fabsf( normal[2] ))
		{
			ignoreAxis = 1;
		}
		else
		{
			ignoreAxis = 2;
		}
	}

	// project vertices onto plane
	GLfloat2Array flat( count );
	switch( ignoreAxis )
	{
		case 0:
			for( i = 0; i < count; i++ )
			{
				flat[i][0] = vertices[i][1];
				flat[i][1] = vertices[i][2];
			}
			break;
		case 1:
			for( i = 0; i < count; i++ )
			{
				flat[i][0] = vertices[i][2];
				flat[i][1] = vertices[i][0];
			}
			break;
		case 2:
			for( i = 0; i < count; i++ )
			{
				flat[i][0] = vertices[i][0];
				flat[i][1] = vertices[i][1];
			}
			break;
	}

	// compute area of polygon
	float *area = new float[1];
	area[0] = 0.0f;
	for( j = count - 1, i = 0; i < count; j = i, i++ )
	{
		area[0] += flat[j][0] *flat[i][1] - flat[j][1] *flat[i][0];
	}
	area[0] = 0.5f * fabsf( area[0] ) / normal[ignoreAxis];

	// set lod info
	setNumLODs( 1, area );

	// compute bounding sphere, put center at average of vertices
	GLfloat mySphere[4];
	mySphere[0] = mySphere[1] = mySphere[2] = mySphere[3] = 0.0f;
	for( i = 0; i < count; i++ )
	{
		mySphere[0] += vertices[i][0];
		mySphere[1] += vertices[i][1];
		mySphere[2] += vertices[i][2];
	}
	mySphere[0] /= ( float )count;
	mySphere[1] /= ( float )count;
	mySphere[2] /= ( float )count;
	for( i = 0; i < count; i++ )
	{
		const float dx = mySphere[0] - vertices[i][0];
		const float dy = mySphere[1] - vertices[i][1];
		const float dz = mySphere[2] - vertices[i][2];
		GLfloat r = (( dx *dx ) + ( dy *dy ) + ( dz *dz ));
		if( r > mySphere[3] )
		{
			mySphere[3] = r;
		}
	}
	setSphere( mySphere );

	// record extents info
	for( i = 0; i < count; i++ )
	{
		extents.expandToPoint( vertices[i] );
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


MeshPolySceneNode::~MeshPolySceneNode()
{
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshPolySceneNode::cull( const ViewFrustum &frustum )const
{
	// cull if eye is behind (or on) plane
	const GLfloat *eye = frustum.getEye();
	if((( eye[0] *plane[0] ) + ( eye[1] *plane[1] ) + ( eye[2] *plane[2] ) + plane[3] ) <= 0.0f )
	{
		return true;
	}

	// if the Visibility culler tells us that we're
	// fully visible, then skip the rest of these tests
	if( octreeState == OctreeVisible )
	{
		return false;
	}

	const Frustum *f = ( const Frustum* ) &frustum;
	if( testAxisBoxInFrustum( extents, f ) == Outside )
	{
		return true;
	}

	// probably visible
	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshPolySceneNode::inAxisBox( const Extents &exts )const
{
	if( !extents.touches( exts ))
	{
		return false;
	}

	return testPolygonInAxisBox( getVertexCount(), getVertices(), getPlane(), exts );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int MeshPolySceneNode::split( const float *splitPlane, SceneNode * &front, SceneNode * &back )const
{
	if( node.normals.getSize() > 0 )
	{
		return splitWallVTN( splitPlane, node.vertices, node.normals, node.texcoords, front, back );
	}
	else
	{
		return splitWallVT( splitPlane, node.vertices, node.texcoords, front, back );
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::addRenderNodes( SceneRenderer &renderer )
{
	node.setStyle( getStyle());
	const GLfloat *dyncol = getDynamicColor();
	if(( dyncol == NULL ) || ( dyncol[3] != 0.0f ))
	{
		renderer.addRenderNode( &node, getWallGState());
	}
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::addShadowNodes( SceneRenderer &renderer )
{
	if( !noShadow )
	{
		const GLfloat *dyncol = getDynamicColor();
		if(( dyncol == NULL ) || ( dyncol[3] != 0.0f ))
		{
			renderer.addShadowNode( &node );
		}
	}
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::renderRadar()
{
	if( !noRadar )
	{
		node.renderRadar();
	}
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int MeshPolySceneNode::splitWallVTN( const GLfloat *splitPlane, const GLfloat3Array &vertices, const GLfloat3Array &normals, const GLfloat2Array &texcoords, SceneNode * &front, SceneNode * &back )const
{
	int i;
	const int count = vertices.getSize();
	const float fudgeFactor = 0.001f;
	const unsigned char BACK_SIDE = ( 1 << 0 );
	const unsigned char FRONT_SIDE = ( 1 << 1 );

	// arrays for tracking each vertex's side
	// and distance from the splitting plane
	// (assuming stack allocation with be faster then heap, might be wrong)
	// wonder how that compares to static vs. stack access speeds
	const int staticSize = 64;
	float *dists;
	unsigned char *array;
	float staticDists[staticSize];
	unsigned char staticArray[staticSize];
	if( count > staticSize )
	{
		array = new unsigned char[count];
		dists = new float[count];
	}
	else
	{
		array = staticArray;
		dists = staticDists;
	}

	// determine on which side of the plane each point lies
	int bothCount = 0;
	int backCount = 0;
	int frontCount = 0;
	for( i = 0; i < count; i++ )
	{
		const GLfloat d = ( vertices[i][0] *splitPlane[0] ) + ( vertices[i][1] *splitPlane[1] ) + ( vertices[i][2] *splitPlane[2] ) + splitPlane[3];
		if( d <  - fudgeFactor )
		{
			array[i] = BACK_SIDE;
			backCount++;
		}
		else if( d > fudgeFactor )
		{
			array[i] = FRONT_SIDE;
			frontCount++;
		}
		else
		{
			array[i] = ( BACK_SIDE | FRONT_SIDE );
			bothCount++;
			backCount++;
			frontCount++;
		}
		dists[i] = d; // save for later
	}

	// see if we need to split
	if(( frontCount == 0 ) || ( frontCount == bothCount ))
	{
		if( count > staticSize )
		{
			delete []array;
			delete []dists;
		}
		return  - 1; // node is on the back side
	}
	if(( backCount == 0 ) || ( backCount == bothCount ))
	{
		if( count > staticSize )
		{
			delete []array;
			delete []dists;
		}
		return  + 1; // node is on the front side
	}

	// get the first old front and back points
	int firstFront =  - 1, firstBack =  - 1;

	for( i = 0; i < count; i++ )
	{
		const int next = ( i + 1 ) % count; // the next index
		if( array[next] &FRONT_SIDE )
		{
			if( !( array[i] &FRONT_SIDE ))
			{
				firstFront = next;
			}
		}
		if( array[next] &BACK_SIDE )
		{
			if( !( array[i] &BACK_SIDE ))
			{
				firstBack = next;
			}
		}
	}

	// get the last old front and back points
	int lastFront = ( firstFront + frontCount - 1 ) % count;
	int lastBack = ( firstBack + backCount - 1 ) % count;

	// add in extra counts for the splitting vertices
	if( firstFront != lastBack )
	{
		frontCount++;
		backCount++;
	}
	if( firstBack != lastFront )
	{
		frontCount++;
		backCount++;
	}

	// make space for new polygons
	GLfloat3Array vertexFront( frontCount );
	GLfloat3Array normalFront( frontCount );
	GLfloat2Array uvFront( frontCount );
	GLfloat3Array vertexBack( backCount );
	GLfloat3Array normalBack( backCount );
	GLfloat2Array uvBack( backCount );

	// fill in the splitting vertices
	int frontIndex = 0;
	int backIndex = 0;
	if( firstFront != lastBack )
	{
		GLfloat splitVertex[3], splitNormal[3], splitUV[2];
		splitEdgeVTN( dists[firstFront], dists[lastBack], vertices[firstFront], vertices[lastBack], normals[firstFront], normals[lastBack], texcoords[firstFront], texcoords[lastBack], splitVertex, splitNormal, splitUV );
		memcpy( vertexFront[0], splitVertex, sizeof( GLfloat[3] ));
		memcpy( normalFront[0], splitNormal, sizeof( GLfloat[3] ));
		memcpy( uvFront[0], splitUV, sizeof( GLfloat[2] ));
		frontIndex++; // bump up the head
		const int last = backCount - 1;
		memcpy( vertexBack[last], splitVertex, sizeof( GLfloat[3] ));
		memcpy( normalBack[last], splitNormal, sizeof( GLfloat[3] ));
		memcpy( uvBack[last], splitUV, sizeof( GLfloat[2] ));
	}
	if( firstBack != lastFront )
	{
		GLfloat splitVertex[3], splitNormal[3], splitUV[2];
		splitEdgeVTN( dists[firstBack], dists[lastFront], vertices[firstBack], vertices[lastFront], normals[firstBack], normals[lastFront], texcoords[firstBack], texcoords[lastFront], splitVertex, splitNormal, splitUV );
		memcpy( vertexBack[0], splitVertex, sizeof( GLfloat[3] ));
		memcpy( normalBack[0], splitNormal, sizeof( GLfloat[3] ));
		memcpy( uvBack[0], splitUV, sizeof( GLfloat[2] ));
		backIndex++; // bump up the head
		const int last = frontCount - 1;
		memcpy( vertexFront[last], splitVertex, sizeof( GLfloat[3] ));
		memcpy( normalFront[last], splitNormal, sizeof( GLfloat[3] ));
		memcpy( uvFront[last], splitUV, sizeof( GLfloat[2] ));
	}

	// fill in the old front side vertices
	const int endFront = ( lastFront + 1 ) % count;
	for( i = firstFront; i != endFront; i = ( i + 1 ) % count )
	{
		memcpy( vertexFront[frontIndex], vertices[i], sizeof( GLfloat[3] ));
		memcpy( normalFront[frontIndex], normals[i], sizeof( GLfloat[3] ));
		memcpy( uvFront[frontIndex], texcoords[i], sizeof( GLfloat[2] ));
		frontIndex++;
	}

	// fill in the old back side vertices
	const int endBack = ( lastBack + 1 ) % count;
	for( i = firstBack; i != endBack; i = ( i + 1 ) % count )
	{
		memcpy( vertexBack[backIndex], vertices[i], sizeof( GLfloat[3] ));
		memcpy( normalBack[backIndex], normals[i], sizeof( GLfloat[3] ));
		memcpy( uvBack[backIndex], texcoords[i], sizeof( GLfloat[2] ));
		backIndex++;
	}

	// make new nodes
	front = new MeshPolySceneNode( getPlane(), noRadar, noShadow, vertexFront, normalFront, uvFront );
	back = new MeshPolySceneNode( getPlane(), noRadar, noShadow, vertexBack, normalBack, uvBack );

	// free the arrays, if required
	if( count > staticSize )
	{
		delete []array;
		delete []dists;
	}

	return 0; // generated new front and back nodes
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::splitEdgeVTN( float d1, float d2, const GLfloat *p1, const GLfloat *p2, const GLfloat *n1, const GLfloat *n2, const GLfloat *uv1, const GLfloat *uv2, GLfloat *p, GLfloat *n, GLfloat *uv )const
{
	// compute fraction along edge where split occurs
	float t1 = ( d2 - d1 );
	if( t1 != 0.0f )
	{
		// shouldn't happen
		t1 =  - ( d1 / t1 );
	}

	// compute vertex
	p[0] = p1[0] + ( t1 *( p2[0] - p1[0] ));
	p[1] = p1[1] + ( t1 *( p2[1] - p1[1] ));
	p[2] = p1[2] + ( t1 *( p2[2] - p1[2] ));

	// compute normal
	const float t2 = 1.0f - t1;
	n[0] = ( n1[0] *t2 ) + ( n2[0] *t1 );
	n[1] = ( n1[1] *t2 ) + ( n2[1] *t1 );
	n[2] = ( n1[2] *t2 ) + ( n2[2] *t1 );
	// normalize
	float len = (( n[0] *n[0] ) + ( n[1] *n[1] ) + ( n[2] *n[2] ));
	if( len > 1.0e-20f )
	{
		// otherwise, let it go...
		len = 1.0f / sqrtf( len );
		n[0] = n[0] *len;
		n[1] = n[1] *len;
		n[2] = n[2] *len;
	}

	// compute texture coordinate
	uv[0] = uv1[0] + ( t1 *( uv2[0] - uv1[0] ));
	uv[1] = uv1[1] + ( t1 *( uv2[1] - uv1[1] ));

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int MeshPolySceneNode::splitWallVT( const GLfloat *splitPlane, const GLfloat3Array &vertices, const GLfloat2Array &texcoords, SceneNode * &front, SceneNode * &back )const
{
	int i;
	const int count = vertices.getSize();
	const float fudgeFactor = 0.001f;
	const unsigned char BACK_SIDE = ( 1 << 0 );
	const unsigned char FRONT_SIDE = ( 1 << 1 );

	// arrays for tracking each vertex's side
	// and distance from the splitting plane
	// (assuming stack allocation with be faster then heap, might be wrong)
	// wonder how that compares to static vs. stack access speeds
	const int staticSize = 64;
	float *dists;
	unsigned char *array;
	float staticDists[staticSize];
	unsigned char staticArray[staticSize];
	if( count > staticSize )
	{
		array = new unsigned char[count];
		dists = new float[count];
	}
	else
	{
		array = staticArray;
		dists = staticDists;
	}

	// determine on which side of the plane each point lies
	int bothCount = 0;
	int backCount = 0;
	int frontCount = 0;
	for( i = 0; i < count; i++ )
	{
		const GLfloat d = ( vertices[i][0] *splitPlane[0] ) + ( vertices[i][1] *splitPlane[1] ) + ( vertices[i][2] *splitPlane[2] ) + splitPlane[3];
		if( d <  - fudgeFactor )
		{
			array[i] = BACK_SIDE;
			backCount++;
		}
		else if( d > fudgeFactor )
		{
			array[i] = FRONT_SIDE;
			frontCount++;
		}
		else
		{
			array[i] = ( BACK_SIDE | FRONT_SIDE );
			bothCount++;
			backCount++;
			frontCount++;
		}
		dists[i] = d; // save for later
	}

	// see if we need to split
	if(( frontCount == 0 ) || ( frontCount == bothCount ))
	{
		if( count > staticSize )
		{
			delete []array;
			delete []dists;
		}
		return  - 1; // node is on the back side
	}
	if(( backCount == 0 ) || ( backCount == bothCount ))
	{
		if( count > staticSize )
		{
			delete []array;
			delete []dists;
		}
		return  + 1; // node is on the front side
	}

	// get the first old front and back points
	int firstFront =  - 1, firstBack =  - 1;

	for( i = 0; i < count; i++ )
	{
		const int next = ( i + 1 ) % count; // the next index
		if( array[next] &FRONT_SIDE )
		{
			if( !( array[i] &FRONT_SIDE ))
			{
				firstFront = next;
			}
		}
		if( array[next] &BACK_SIDE )
		{
			if( !( array[i] &BACK_SIDE ))
			{
				firstBack = next;
			}
		}
	}

	// get the last old front and back points
	int lastFront = ( firstFront + frontCount - 1 ) % count;
	int lastBack = ( firstBack + backCount - 1 ) % count;

	// add in extra counts for the splitting vertices
	if( firstFront != lastBack )
	{
		frontCount++;
		backCount++;
	}
	if( firstBack != lastFront )
	{
		frontCount++;
		backCount++;
	}

	// make space for new polygons
	GLfloat3Array vertexFront( frontCount );
	GLfloat3Array normalFront( 0 );
	GLfloat2Array uvFront( frontCount );
	GLfloat3Array vertexBack( backCount );
	GLfloat3Array normalBack( 0 );
	GLfloat2Array uvBack( backCount );

	// fill in the splitting vertices
	int frontIndex = 0;
	int backIndex = 0;
	if( firstFront != lastBack )
	{
		GLfloat splitVertex[3], splitUV[2];
		splitEdgeVT( dists[firstFront], dists[lastBack], vertices[firstFront], vertices[lastBack], texcoords[firstFront], texcoords[lastBack], splitVertex, splitUV );
		memcpy( vertexFront[0], splitVertex, sizeof( GLfloat[3] ));
		memcpy( uvFront[0], splitUV, sizeof( GLfloat[2] ));
		frontIndex++; // bump up the head
		const int last = backCount - 1;
		memcpy( vertexBack[last], splitVertex, sizeof( GLfloat[3] ));
		memcpy( uvBack[last], splitUV, sizeof( GLfloat[2] ));
	}
	if( firstBack != lastFront )
	{
		GLfloat splitVertex[3], splitUV[2];
		splitEdgeVT( dists[firstBack], dists[lastFront], vertices[firstBack], vertices[lastFront], texcoords[firstBack], texcoords[lastFront], splitVertex, splitUV );
		memcpy( vertexBack[0], splitVertex, sizeof( GLfloat[3] ));
		memcpy( uvBack[0], splitUV, sizeof( GLfloat[2] ));
		backIndex++; // bump up the head
		const int last = frontCount - 1;
		memcpy( vertexFront[last], splitVertex, sizeof( GLfloat[3] ));
		memcpy( uvFront[last], splitUV, sizeof( GLfloat[2] ));
	}

	// fill in the old front side vertices
	const int endFront = ( lastFront + 1 ) % count;
	for( i = firstFront; i != endFront; i = ( i + 1 ) % count )
	{
		memcpy( vertexFront[frontIndex], vertices[i], sizeof( GLfloat[3] ));
		memcpy( uvFront[frontIndex], texcoords[i], sizeof( GLfloat[2] ));
		frontIndex++;
	}

	// fill in the old back side vertices
	const int endBack = ( lastBack + 1 ) % count;
	for( i = firstBack; i != endBack; i = ( i + 1 ) % count )
	{
		memcpy( vertexBack[backIndex], vertices[i], sizeof( GLfloat[3] ));
		memcpy( uvBack[backIndex], texcoords[i], sizeof( GLfloat[2] ));
		backIndex++;
	}

	// make new nodes
	front = new MeshPolySceneNode( getPlane(), noRadar, noShadow, vertexFront, normalFront, uvFront );
	back = new MeshPolySceneNode( getPlane(), noRadar, noShadow, vertexBack, normalBack, uvBack );

	// free the arrays, if required
	if( count > staticSize )
	{
		delete []array;
		delete []dists;
	}

	return 0; // generated new front and back nodes
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::splitEdgeVT( float d1, float d2, const GLfloat *p1, const GLfloat *p2, const GLfloat *uv1, const GLfloat *uv2, GLfloat *p, GLfloat *uv )const
{
	// compute fraction along edge where split occurs
	float t1 = ( d2 - d1 );
	if( t1 != 0.0f )
	{
		// shouldn't happen
		t1 =  - ( d1 / t1 );
	}

	// compute vertex
	p[0] = p1[0] + ( t1 *( p2[0] - p1[0] ));
	p[1] = p1[1] + ( t1 *( p2[1] - p1[1] ));
	p[2] = p1[2] + ( t1 *( p2[2] - p1[2] ));

	// compute texture coordinate
	uv[0] = uv1[0] + ( t1 *( uv2[0] - uv1[0] ));
	uv[1] = uv1[1] + ( t1 *( uv2[1] - uv1[1] ));

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshPolySceneNode::getRenderNodes( std::vector < RenderSet >  &rnodes )
{
	RenderSet rs = 
	{
		 &node, getWallGState()
	};
	rnodes.push_back( rs );
	return ;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
