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

#include <math.h>
#include "common.h"
#include "global.h"
#include "Pack.h"
#include "vectors.h"

#include "MeshFace.h"
#include "MeshObstacle.h"

#include "PhysicsDriver.h"
#include "Intersect.h"


const char *MeshFace::typeName = "MeshFace";


MeshFace::MeshFace( MeshObstacle *_mesh )
{
	mesh = _mesh;
	vertexCount = 0;
	vertices = NULL;
	normals = NULL;
	texcoords = NULL;
	noclusters = false;
	smoothBounce = false;
	driveThrough = false;
	shootThrough = false;
	edges = NULL;
	edgePlanes = NULL;
	specialData = NULL;
	specialState = 0;
	phydrv =  - 1;

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


MeshFace::MeshFace( MeshObstacle *_mesh, int _vertexCount, float **_vertices, float **_normals, float **_texcoords, const BzMaterial *_bzMaterial, int physics, bool _noclusters, bool bounce, bool drive, bool shoot )
{
	mesh = _mesh;
	vertexCount = _vertexCount;
	vertices = _vertices;
	normals = _normals;
	texcoords = _texcoords;
	bzMaterial = _bzMaterial;
	phydrv = physics;
	noclusters = _noclusters;
	smoothBounce = bounce;
	driveThrough = drive;
	shootThrough = shoot;
	edges = NULL;
	edgePlanes = NULL;
	specialData = NULL;
	specialState = 0;

	finalize();

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshFace::finalize()
{
	float maxCrossSqr = 0.0f;
	float bestCross[3] = 
	{
		0.0f, 0.0f, 0.0f
	};
	int bestSet[3] = 
	{
		 - 1,  - 1,  - 1
	};

	// find the best vertices for making the plane
	int i, j, k;
	for( i = 0; i < ( vertexCount - 2 ); i++ )
	{
		for( j = i; j < ( vertexCount - 1 ); j++ )
		{
			for( k = j; k < ( vertexCount - 0 ); k++ )
			{
				float edge1[3], edge2[3], cross[3];
				vec3sub( edge1, vertices[k], vertices[j] );
				vec3sub( edge2, vertices[i], vertices[j] );
				vec3cross( cross, edge1, edge2 );
				const float lenSqr = vec3dot( cross, cross );
				if( lenSqr > maxCrossSqr )
				{
					maxCrossSqr = lenSqr;
					bestSet[0] = i;
					bestSet[1] = j;
					bestSet[2] = k;
					memcpy( bestCross, cross, sizeof( float[3] ));
				}
			}
		}
	}

	if( maxCrossSqr <  +1.0e-20f )
	{

		DEBUG1( "invalid mesh face (%f)", maxCrossSqr );
		if(( debugLevel >= 3 ) && ( mesh != NULL ))
		{
			DEBUG0( ":" );
			for( i = 0; i < vertexCount; i++ )
			{
				DEBUG0( " %i", ( int )(( fvec3* )vertices[i] - mesh->getVertices()));
			}
			print( std::cerr, "" );
		}
		DEBUG1( "\n" );

		vertexCount = 0;
		return ;
	}

	// make the plane
	float scale = 1.0f / sqrtf( maxCrossSqr );
	const float *vert = vertices[bestSet[1]];
	plane[0] = bestCross[0] *scale;
	plane[1] = bestCross[1] *scale;
	plane[2] = bestCross[2] *scale;
	plane[3] =  - vec3dot( plane, vert );

	// see if the whole face is convex
	int v;
	for( v = 0; v < vertexCount; v++ )
	{
		float a[3], b[3], c[3];
		vec3sub( a, vertices[( v + 1 ) % vertexCount], vertices[( v + 0 ) % vertexCount] );
		vec3sub( b, vertices[( v + 2 ) % vertexCount], vertices[( v + 1 ) % vertexCount] );
		vec3cross( c, a, b );
		const float d = vec3dot( c, plane );
		if( d <= 0.0f )
		{

			DEBUG1( "non-convex mesh face (%f)", d );
			if(( debugLevel >= 3 ) && ( mesh != NULL ))
			{
				DEBUG0( ":" );
				for( i = 0; i < vertexCount; i++ )
				{
					DEBUG0( " %i", ( int )(( fvec3* )vertices[i] - mesh->getVertices()));
				}
				print( std::cerr, "" );
			}
			DEBUG1( "\n" );

			vertexCount = 0;
			return ;
		}
	}

	// see if the vertices are coplanar
	for( v = 0; v < vertexCount; v++ )
	{
		const float cross = vec3dot( vertices[v], plane );
		if( fabsf( cross + plane[3] ) > 1.0e-3 )
		{
			DEBUG1( "non-planar mesh face (%f)", cross + plane[3] );
			if(( debugLevel >= 3 ) && ( mesh != NULL ))
			{
				DEBUG0( ":" );
				for( i = 0; i < vertexCount; i++ )
				{
					DEBUG0( " %i", ( int )(( fvec3* )vertices[i] - mesh->getVertices()));
				}
				print( std::cerr, "" );
			}
			DEBUG1( "\n" );

			vertexCount = 0;
			return ;
		}
	}

	// setup extents
	for( v = 0; v < vertexCount; v++ )
	{
		extents.expandToPoint( vertices[v] );
	}

	// setup fake obstacle parameters
	pos[0] = ( extents.maxs[0] + extents.mins[0] ) / 2.0f;
	pos[1] = ( extents.maxs[1] + extents.mins[1] ) / 2.0f;
	pos[2] = extents.mins[2];
	size[0] = ( extents.maxs[0] - extents.mins[0] ) / 2.0f;
	size[1] = ( extents.maxs[1] - extents.mins[1] ) / 2.0f;
	size[2] = ( extents.maxs[2] - extents.mins[2] );
	angle = 0.0f;
	ZFlip = false;

	// make the edge planes
	edgePlanes = new fvec4[vertexCount];
	for( v = 0; v < vertexCount; v++ )
	{
		float edge[3];
		const int next = ( v + 1 ) % vertexCount;
		vec3sub( edge, vertices[next], vertices[v] );
		vec3cross( edgePlanes[v], edge, plane );
		scale = 1.0f / sqrtf( vec3dot( edgePlanes[v], edgePlanes[v] ));
		edgePlanes[v][0] = edgePlanes[v][0] *scale;
		edgePlanes[v][1] = edgePlanes[v][1] *scale;
		edgePlanes[v][2] = edgePlanes[v][2] *scale;
		edgePlanes[v][3] =  - vec3dot( vertices[v], edgePlanes[v] );
	}

	// set the plane type
	planeBits = 0;
	const float fudge = 1.0e-5f;
	if(( fabsf( plane[2] ) + fudge ) >= 1.0f )
	{
		planeBits |= ZPlane;
		if( plane[2] > 0.0f )
		{
			planeBits |= UpPlane;
			//FIXME
			plane[2] = 1.0f;
			plane[3] =  - pos[2];
		}
		else
		{
			planeBits |= DownPlane;
			//FIXME
			plane[2] =  - 1.0f;
			plane[3] =  + pos[2];
		}
		//FIXME
		plane[0] = 0.0f;
		plane[1] = 0.0f;
	}
	else if(( fabsf( plane[0] ) + fudge ) >= 1.0f )
	{
		planeBits |= XPlane;
	}
	else if(( fabsf( plane[1] ) + fudge ) >= 1.0f )
	{
		planeBits |= YPlane;
	}

	if( fabsf( plane[2] ) < fudge )
	{
		planeBits |= WallPlane;
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


MeshFace::~MeshFace()
{
	delete []vertices;
	delete []normals;
	delete []texcoords;
	delete []edges;
	delete []edgePlanes;
	delete specialData;
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


const char *MeshFace::getType()const
{
	return typeName;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


const char *MeshFace::getClassName() // const
{
	return typeName;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshFace::isValid()const
{
	// this is used as a tag in finalize()
	if( vertexCount == 0 )
	{
		return false;
	}
	else
	{
		return true;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshFace::isFlatTop()const
{
	return isUpPlane();
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


float MeshFace::intersect( const Ray &ray )const
{
	// NOTE: i'd use a quick test here first, but the
	//       plan is to use an octree for the collision
	//       manager which should get us close enough
	//       that a quick test might actually eat up time.
	//
	// find where the ray crosses each plane, and then
	// check the dot-product of the three bounding planes
	// to see if the intersection point is contained within
	// the face.
	//
	//  L - line unit vector	  Lo - line origin
	//  N - plane normal unit vector  d  - plane offset
	//  P - point in question	 t - time
	//
	//  (N dot P) + d = 0		      { plane equation }
	//  P = (t * L) + Lo		       { line equation }
	//  t (N dot L) + (N dot Lo) + d = 0
	//
	//  t = - (d + (N dot Lo)) / (N dot L)     { time of impact }
	//
	const float *dir = ray.getDirection();
	const float *origin = ray.getOrigin();
	float hitTime;

	// get the time until the shot would hit each plane
	const float linedot = ( plane[0] *dir[0] ) + ( plane[1] *dir[1] ) + ( plane[2] *dir[2] );
	if( linedot >=  - 0.001f )
	{
		// shot is either parallel, or going through backwards
		return  - 1.0f;
	}
	const float origindot = ( plane[0] *origin[0] ) + ( plane[1] *origin[1] ) + ( plane[2] *origin[2] );
	// linedot should be safe to divide with now
	hitTime =  - ( plane[3] + origindot ) / linedot;
	if( hitTime < 0.0f )
	{
		return  - 1.0f;
	}

	// get the contact location
	float point[3];
	point[0] = ( dir[0] *hitTime ) + origin[0];
	point[1] = ( dir[1] *hitTime ) + origin[1];
	point[2] = ( dir[2] *hitTime ) + origin[2];

	// now test against the edge planes
	for( int q = 0; q < vertexCount; q++ )
	{
		float d = ( edgePlanes[q][0] *point[0] ) + ( edgePlanes[q][1] *point[1] ) + ( edgePlanes[q][2] *point[2] ) + edgePlanes[q][3];
		if( d > 0.001f )
		{
			return  - 1.0f;
		}
	}

	return hitTime;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshFace::get3DNormal( const float *p, float *n )const
{
	if( !smoothBounce || !useNormals())
	{
		// just use the plain normal
		memcpy( n, plane, sizeof( float[3] ));
	}
	else
	{
		// FIXME: this isn't quite right
		// normal smoothing to fake curved surfaces
		int i;
		// calculate the triangle ares
		float totalArea = 0.0f;
		float *areas = new float[vertexCount];
		for( i = 0; i < vertexCount; i++ )
		{
			int next = ( i + 1 ) % vertexCount;
			float ea[3], eb[3], cross[3];
			vec3sub( ea, p, vertices[i] );
			vec3sub( eb, vertices[next], vertices[i] );
			vec3cross( cross, ea, eb );
			areas[i] = sqrtf( vec3dot( cross, cross ));
			totalArea = totalArea + areas[i];
		}
		float smallestArea = MAXFLOAT;
		float *twinAreas = new float[vertexCount];
		for( i = 0; i < vertexCount; i++ )
		{
			int next = ( i + 1 ) % vertexCount;
			twinAreas[i] = areas[i] + areas[next];
			if( twinAreas[i] < 1.0e-10f )
			{
				memcpy( n, normals[next], sizeof( float[3] ));
				delete []areas;
				delete []twinAreas;
				return ;
			}
			if( twinAreas[i] < smallestArea )
			{
				smallestArea = twinAreas[i];
			}
		}
		float normal[3] = 
		{
			0.0f, 0.0f, 0.0f
		};
		for( i = 0; i < vertexCount; i++ )
		{
			int next = ( i + 1 ) % vertexCount;
			float factor = smallestArea / twinAreas[i];
			normal[0] = normal[0] + ( normals[next][0] *factor );
			normal[1] = normal[1] + ( normals[next][1] *factor );
			normal[2] = normal[2] + ( normals[next][2] *factor );
		}
		float len = sqrtf( vec3dot( normal, normal ));
		if( len < 1.0e-10 )
		{
			memcpy( n, plane, sizeof( float[3] ));
			delete []areas;
			delete []twinAreas;
			return ;
		}
		len = 1.0f / len;
		n[0] = normal[0] *len;
		n[1] = normal[1] *len;
		n[2] = normal[2] *len;

		delete []areas;
		delete []twinAreas;
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshFace::getNormal( const float * /*p*/, float *n )const
{
	if( n )
	{
		memcpy( n, plane, sizeof( float[3] ));
	}
	return ;
}


///////////////////////////////////////////////////////////////
//  FIXME - all geometry after this point is currently JUNK! //
///////////////////////////////////////////////////////////////


bool MeshFace::getHitNormal( const float * /*oldPos*/, float /*oldAngle*/, const float * /*newPos*/, float /*newAngle*/, float /*dx*/, float /*dy*/, float /*height*/, float *normal )const
{
	if( normal )
	{
		memcpy( normal, plane, sizeof( float[3] ));
	}
	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshFace::inCylinder( const float *p, float radius, float height )const
{
	return inBox( p, 0.0f, radius, radius, height );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshFace::inBox( const float *p, float _angle, float dx, float dy, float height )const
{
	int i;

	// Z axis separation test
	if(( extents.mins[2] > ( p[2] + height )) || ( extents.maxs[2] < p[2] ))
	{
		return false;
	}

	// translate the face so that the box is an origin box
	// centered at 0,0,0  (this assumes that it is cheaper
	// to move the polygon then the box, tris and quads will
	// probably be the dominant polygon types).

	float pln[4]; // translated plane
	fvec3 *v = new fvec3[vertexCount]; // translated vertices
	const float cos_val = cosf(  - _angle );
	const float sin_val = sinf(  - _angle );
	for( i = 0; i < vertexCount; i++ )
	{
		float h[2];
		h[0] = vertices[i][0] - p[0];
		h[1] = vertices[i][1] - p[1];
		v[i][0] = ( cos_val *h[0] ) - ( sin_val *h[1] );
		v[i][1] = ( cos_val *h[1] ) + ( sin_val *h[0] );
		v[i][2] = vertices[i][2] - p[2];
	}
	pln[0] = ( cos_val *plane[0] ) - ( sin_val *plane[1] );
	pln[1] = ( cos_val *plane[1] ) + ( sin_val *plane[0] );
	pln[2] = plane[2];
	pln[3] = plane[3] + vec3dot( plane, p );

	// testPolygonInAxisBox() expects us to have already done all of the
	// separation tests with respect to the box planes. we could not do
	// the X and Y axis tests until we'd found the translated vertices,
	// so we will do them now.

	// X axis test
	float min, max;
	min =  + MAXFLOAT;
	max =  - MAXFLOAT;
	for( i = 0; i < vertexCount; i++ )
	{
		if( v[i][0] < min )
		{
			min = v[i][0];
		}
		if( v[i][0] > max )
		{
			max = v[i][0];
		}
	}
	if(( min > dx ) || ( max <  - dx ))
	{
		delete []v;
		return false;
	}

	// Y axis test
	min =  + MAXFLOAT;
	max =  - MAXFLOAT;
	for( i = 0; i < vertexCount; i++ )
	{
		if( v[i][1] < min )
		{
			min = v[i][1];
		}
		if( v[i][1] > max )
		{
			max = v[i][1];
		}
	}
	if(( min > dy ) || ( max <  - dy ))
	{
		delete []v;
		return false;
	}

	// FIXME: do not use testPolygonInAxisBox()
	Extents box;
	// mins
	box.mins[0] =  - dx;
	box.mins[1] =  - dy;
	box.mins[2] = 0.0f;
	// maxs
	box.maxs[0] =  + dx;
	box.maxs[1] =  + dy;
	box.maxs[2] = height;

	bool hit = testPolygonInAxisBox( vertexCount, v, pln, box );

	delete []v;

	return hit;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshFace::inMovingBox( const float *oldPos, float /*oldAngle*/, const float *newPos, float newAngle, float dx, float dy, float height )const
{
	// expand the box with respect to Z axis motion
	float _pos[3];
	_pos[0] = newPos[0];
	_pos[1] = newPos[1];
	if( oldPos[2] < newPos[2] )
	{
		_pos[2] = oldPos[2];
	}
	else
	{
		_pos[2] = newPos[2];
	}
	height = height + fabsf( oldPos[2] - newPos[2] );

	return inBox( _pos, newAngle, dx, dy, height );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshFace::isCrossing( const float * /*p*/, float /*angle*/, float /*dx*/, float /*dy*/, float /*height*/, float *_plane )const
{
	if( _plane != NULL )
	{
		memcpy( _plane, plane, sizeof( float[4] ));
	}
	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void *MeshFace::pack( void *buf )const
{
	// state byte
	unsigned char stateByte = 0;
	stateByte |= useNormals() ? ( 1 << 0 ): 0;
	stateByte |= useTexcoords() ? ( 1 << 1 ): 0;
	stateByte |= isDriveThrough() ? ( 1 << 2 ): 0;
	stateByte |= isShootThrough() ? ( 1 << 3 ): 0;
	stateByte |= smoothBounce ? ( 1 << 4 ): 0;
	stateByte |= noclusters ? ( 1 << 5 ): 0;
	buf = nboPackUByte( buf, stateByte );

	// vertices
	buf = nboPackInt( buf, vertexCount );
	for( int i = 0; i < vertexCount; i++ )
	{
		int32_t index = ( fvec3* )vertices[i] - mesh->getVertices();
		buf = nboPackInt( buf, index );
	}

	// normals
	if( useNormals())
	{
		for( int i = 0; i < vertexCount; i++ )
		{
			int32_t index = ( fvec3* )normals[i] - mesh->getNormals();
			buf = nboPackInt( buf, index );
		}
	}

	// texcoords
	if( useTexcoords())
	{
		for( int i = 0; i < vertexCount; i++ )
		{
			int32_t index = ( fvec2* )texcoords[i] - mesh->getTexcoords();
			buf = nboPackInt( buf, index );
		}
	}

	// material
	int matindex = MATERIALMGR.getIndex( bzMaterial );
	buf = nboPackInt( buf, matindex );

	// physics driver
	buf = nboPackInt( buf, phydrv );

	return buf;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void *MeshFace::unpack( void *buf )
{
	int32_t inTmp;
	driveThrough = shootThrough = smoothBounce = false;
	// state byte
	bool tmpNormals, tmpTexcoords;
	unsigned char stateByte = 0;
	buf = nboUnpackUByte( buf, stateByte );
	tmpNormals = ( stateByte &( 1 << 0 )) != 0;
	tmpTexcoords = ( stateByte &( 1 << 1 )) != 0;
	driveThrough = ( stateByte &( 1 << 2 )) != 0;
	shootThrough = ( stateByte &( 1 << 3 )) != 0;
	smoothBounce = ( stateByte &( 1 << 4 )) != 0;
	noclusters = ( stateByte &( 1 << 5 )) != 0;

	// vertices
	buf = nboUnpackInt( buf, inTmp );
	vertexCount = int( inTmp );
	vertices = new float *[vertexCount];
	for( int i = 0; i < vertexCount; i++ )
	{
		int32_t index;
		buf = nboUnpackInt( buf, index );
		vertices[i] = ( float* )mesh->getVertices()[index];
	}

	// normals
	if( tmpNormals )
	{
		normals = new float *[vertexCount];
		for( int i = 0; i < vertexCount; i++ )
		{
			int32_t index;
			buf = nboUnpackInt( buf, index );
			normals[i] = ( float* )mesh->getNormals()[index];
		}
	}

	// texcoords
	if( tmpTexcoords )
	{
		texcoords = new float *[vertexCount];
		for( int i = 0; i < vertexCount; i++ )
		{
			int32_t index;
			buf = nboUnpackInt( buf, index );
			texcoords[i] = ( float* )mesh->getTexcoords()[index];
		}
	}

	// material
	int32_t matindex;
	buf = nboUnpackInt( buf, matindex );
	bzMaterial = MATERIALMGR.getMaterial( matindex );

	// physics driver
	buf = nboUnpackInt( buf, inTmp );
	phydrv = int( inTmp );

	finalize();

	return buf;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int MeshFace::packSize()const
{
	int fullSize = sizeof( unsigned char );
	fullSize += sizeof( int32_t );
	fullSize += sizeof( int32_t ) *vertexCount;
	if( useNormals())
	{
		fullSize += sizeof( int32_t ) *vertexCount;
	}
	if( useTexcoords())
	{
		fullSize += sizeof( int32_t ) *vertexCount;
	}
	fullSize += sizeof( int32_t ); // material
	fullSize += sizeof( int32_t ); // physics driver

	return fullSize;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshFace::print( std::ostream &out, const std::string &indent )const
{
	if( mesh == NULL )
	{
		return ;
	}

	int i;
	out << indent << "  face" << std::endl;

	if( debugLevel >= 3 )
	{
		out << indent << "  # plane normal = " << plane[0] << " " << plane[1] << " " << plane[2] << " " << plane[3] << std::endl;
	}

	out << indent << "    vertices";
	for( i = 0; i < vertexCount; i++ )
	{
		int index = ( fvec3* )vertices[i] - mesh->getVertices();
		out << " " << index;
	}
	if( debugLevel >= 3 )
	{
		out << indent << " #";
		for( i = 0; i < vertexCount; i++ )
		{
			out << " " << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2];
		}
	}
	out << std::endl;

	if( normals != NULL )
	{
		out << indent << "    normals";
		for( i = 0; i < vertexCount; i++ )
		{
			int index = ( fvec3* )normals[i] - mesh->getNormals();
			out << " " << index;
		}
		if( debugLevel >= 3 )
		{
			out << " #";
			for( i = 0; i < vertexCount; i++ )
			{
				out << " " << normals[i][0] << " " << normals[i][1] << " " << normals[i][2];
			}
		}
		out << std::endl;
	}

	if( texcoords != NULL )
	{
		out << indent << "    texcoords";
		for( i = 0; i < vertexCount; i++ )
		{
			int index = ( fvec2* )texcoords[i] - mesh->getTexcoords();
			out << " " << index;
		}
		if( debugLevel >= 3 )
		{
			out << " #";
			for( i = 0; i < vertexCount; i++ )
			{
				out << " " << texcoords[i][0] << " " << texcoords[i][1];
			}
		}
		out << std::endl;
	}

	out << indent << "    matref ";
	MATERIALMGR.printReference( out, bzMaterial );
	out << std::endl;

	const PhysicsDriver *driver = PHYDRVMGR.getDriver( phydrv );
	if( driver != NULL )
	{
		out << indent << "    phydrv ";
		if( driver->getName().size() > 0 )
		{
			out << driver->getName();
		}
		else
		{
			out << phydrv;
		}
		out << std::endl;
	}

	if( noclusters && !mesh->noClusters())
	{
		out << indent << "    noclusters" << std::endl;
	}
	if( smoothBounce && !mesh->useSmoothBounce())
	{
		out << indent << "    smoothBounce" << std::endl;
	}
	if(( driveThrough && shootThrough ) && !( mesh->isDriveThrough() && mesh->isShootThrough()))
	{
		out << indent << "    passable" << std::endl;
	}
	else
	{
		if( driveThrough && !mesh->isDriveThrough())
		{
			out << indent << "    driveThrough" << std::endl;
		}
		if( shootThrough && !mesh->isShootThrough())
		{
			out << indent << "    shootThrough" << std::endl;
		}
	}

	out << indent << "  endface" << std::endl;

	return ;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
