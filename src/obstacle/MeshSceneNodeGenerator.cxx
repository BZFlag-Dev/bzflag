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
#include "vectors.h"
#include "MeshSceneNodeGenerator.h"
#include "MeshObstacle.h"
#include "MeshFace.h"
#include "bzfgl.h"
#include "MeshDrawInfo.h"
#include "MeshSceneNode.h"
#include "MeshPolySceneNode.h"
#include "MeshFragSceneNode.h"
#include "OccluderSceneNode.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "TextureManager.h"
#include "OpenGLMaterial.h"
#include "StateDatabase.h"
#include "BZDBCache.h"


//
// MeshSceneNodeGenerator
//

MeshSceneNodeGenerator::MeshSceneNodeGenerator( const MeshObstacle *_mesh )
{
	mesh = _mesh;
	currentNode = 0;
	returnOccluders = false;
	setupOccluders();
	const MeshDrawInfo *drawInfo = mesh->getDrawInfo();
	useDrawInfo = ( drawInfo != NULL ) && drawInfo->isValid();
	if( !useDrawInfo )
	{
		setupFacesAndFrags();
	}
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


MeshSceneNodeGenerator::~MeshSceneNodeGenerator()
{
	// do nothing
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshSceneNodeGenerator::setupOccluders()
{
	// This is wasteful, only need separate
	// occluders if the face is invisible or
	// has been grouped into a mesh fragment.
	// If this is done, don't forget to make
	// the resulting combo sceneNode into an
	// occluder.
	const int faceCount = mesh->getFaceCount();
	for( int i = 0; i < faceCount; i++ )
	{
		const MeshFace *face = mesh->getFace( i );
		const BzMaterial *bzmat = face->getMaterial();
		if( bzmat->getOccluder())
		{
			OccluderSceneNode *onode = new OccluderSceneNode( face );
			occluders.push_back( onode );
		}
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static bool translucentMaterial( const BzMaterial *mat )
{
	// translucent texture?
	TextureManager &tm = TextureManager::instance();
	int faceTexture =  - 1;
	if( mat->getTextureCount() > 0 )
	{
		const std::string &texname = mat->getTextureLocal( 0 );
		if( texname.size() > 0 )
		{
			faceTexture = tm.getTextureID( texname.c_str());
			if( faceTexture >= 0 )
			{
				const ImageInfo &imageInfo = tm.getInfo( faceTexture );
				if( imageInfo.alpha && mat->getUseTextureAlpha( 0 ))
				{
					return true;
				}
			}
		}
	}

	// translucent color?
	bool translucentColor = false;
	const DynamicColor *dyncol = DYNCOLORMGR.getColor( mat->getDynamicColor());
	if( dyncol == NULL )
	{
		if( mat->getDiffuse()[3] != 1.0f )
		{
			translucentColor = true;
		}
	}
	else if( dyncol->canHaveAlpha())
	{
		translucentColor = true;
	}

	// is the color used?
	if( translucentColor )
	{
		if((( faceTexture >= 0 ) && mat->getUseColorOnTexture( 0 )) || ( faceTexture < 0 ))
		{
			// modulate with the color if asked to, or
			// if the specified texture was not available
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static bool groundClippedFace( const MeshFace *face )
{
	const float *plane = face->getPlane();
	if( plane[2] <  - 0.9f )
	{
		// plane is facing downwards
		const Extents &exts = face->getExtents();
		if( exts.maxs[2] < 0.001 )
		{
			// plane is on or below the ground, ditch it
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static int sortByMaterial( const void *a, const void *b )
{
	const MeshFace *faceA = *(( const MeshFace ** )a );
	const MeshFace *faceB = *(( const MeshFace ** )b );
	const bool noClusterA = faceA->noClusters();
	const bool noClusterB = faceB->noClusters();

	if( noClusterA && !noClusterB )
	{
		return  - 1;
	}
	if( noClusterB && !noClusterA )
	{
		return  + 1;
	}

	if( faceA->getMaterial() > faceB->getMaterial())
	{
		return  + 1;
	}
	else
	{
		return  - 1;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void MeshSceneNodeGenerator::setupFacesAndFrags()
{
	const int faceCount = mesh->getFaceCount();

	// NOTE: this is where MeshClusters start being called
	//       MeshFragments. it would be good to rename all
	//       of the MeshFragment files and classes to match
	//       with the MeshCluster naming convention.

	// only using regular MeshFaces?
	const bool noMeshClusters = BZDB.isTrue( "noMeshClusters" );
	if( mesh->noClusters() || noMeshClusters || !BZDBCache::zbuffer )
	{
		for( int i = 0; i < faceCount; i++ )
		{
			MeshNode mn;
			mn.isFace = true;
			mn.faces.push_back( mesh->getFace( i ));
			nodes.push_back( mn );
		}
		return ; // bail out
	}

	// build up a list of faces and fragments
	const MeshFace **sortList = new const MeshFace *[faceCount];

	// clip ground faces, and then sort the face list by material
	int count = 0;
	for( int i = 0; i < faceCount; i++ )
	{
		const MeshFace *face = mesh->getFace( i );
		if( !groundClippedFace( face ))
		{
			sortList[count] = face;
			count++;
		}
	}
	qsort( sortList, count, sizeof( MeshFace* ), sortByMaterial );

	// make the faces and fragments
	int first = 0;
	while( first < count )
	{
		const MeshFace *firstFace = sortList[first];
		const BzMaterial *firstMat = firstFace->getMaterial();

		// see if this face needs to be drawn individually
		if( firstFace->noClusters() || ( translucentMaterial( firstMat ) && !firstMat->getNoSorting() && !firstMat->getGroupAlpha()))
		{
			MeshNode mn;
			mn.isFace = true;
			mn.faces.push_back( firstFace );
			nodes.push_back( mn );
			first++;
			continue;
		}

		// collate similar materials
		int last = first + 1;
		while( last < count )
		{
			const MeshFace *lastFace = sortList[last];
			const BzMaterial *lastMat = lastFace->getMaterial();
			if( lastMat != firstMat )
			{
				break;
			}
			last++;
		}

		// make a face for singles, and a fragment otherwise
		if(( last - first ) == 1 )
		{
			MeshNode mn;
			mn.isFace = true;
			mn.faces.push_back( firstFace );
			nodes.push_back( mn );
		}
		else
		{
			MeshNode mn;
			mn.isFace = false;
			for( int i = first; i < last; i++ )
			{
				mn.faces.push_back( sortList[i] );
			}
			nodes.push_back( mn );
		}

		first = last;
	}

	delete []sortList;

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


WallSceneNode *MeshSceneNodeGenerator::getNextNode( bool /*lod*/ )
{
	const MeshNode *mn;
	const MeshFace *face;
	const BzMaterial *mat;

	// divert for Occluders
	if( returnOccluders )
	{
		if( currentNode < ( int )occluders.size())
		{
			currentNode++;
			return ( WallSceneNode* )occluders[currentNode - 1];
		}
		else
		{
			return NULL;
		}
	}

	// divert for the MeshSceneNode
	const MeshDrawInfo *drawInfo = mesh->getDrawInfo();
	if( useDrawInfo )
	{
		if( drawInfo->isInvisible())
		{
			if( occluders.size() <= 0 )
			{
				return NULL;
			}
			else
			{
				currentNode = 1;
				returnOccluders = true;
				return ( WallSceneNode* )occluders[0];
			}
		}
		else
		{
			currentNode = 0;
			returnOccluders = true;
			return ( WallSceneNode* )( new MeshSceneNode( mesh ));
		}
	}

	// remove any faces or frags that will not be displayed
	// also, return NULL if we are at the end of the face list
	while( true )
	{

		if( currentNode >= ( int )nodes.size())
		{
			// start sending out the occluders
			returnOccluders = true;
			if( occluders.size() > 0 )
			{
				currentNode = 1;
				return ( WallSceneNode* )occluders[0];
			}
			else
			{
				return NULL;
			}
		}

		mn = &nodes[currentNode];
		if( mn->isFace )
		{
			face = mn->faces[0];
			mat = face->getMaterial();
		}
		else
		{
			face = NULL;
			mat = mn->faces[0]->getMaterial();
		}

		if( mat->isInvisible())
		{
			currentNode++;
			continue;
		}

		if( mn->isFace && groundClippedFace( face ))
		{
			currentNode++;
			continue;
		}

		break; // break the loop if we haven't used 'continue'
	}

	WallSceneNode *node;
	if( mn->isFace )
	{
		node = getMeshPolySceneNode( face );
	}
	else
	{
		const MeshFace **faces = new const MeshFace *[mn->faces.size()];
		for( int i = 0; i < ( int )mn->faces.size(); i++ )
		{
			faces[i] = mn->faces[i];
		}
		// the MeshFragSceneNode will delete the faces
		node = new MeshFragSceneNode( mn->faces.size(), faces );
	}

	setupNodeMaterial( node, mat );

	currentNode++;

	return node;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


MeshPolySceneNode *MeshSceneNodeGenerator::getMeshPolySceneNode( const MeshFace *face )
{
	int i;

	// vertices
	const int vertexCount = face->getVertexCount();
	GLfloat3Array vertices( vertexCount );
	for( i = 0; i < vertexCount; i++ )
	{
		memcpy( vertices[i], face->getVertex( i ), sizeof( float[3] ));
	}

	// normals
	int normalCount = 0;
	if( face->useNormals())
	{
		normalCount = vertexCount;
	}
	GLfloat3Array normals( normalCount );
	for( i = 0; i < normalCount; i++ )
	{
		memcpy( normals[i], face->getNormal( i ), sizeof( float[3] ));
	}

	// texcoords
	GLfloat2Array texcoords( vertexCount );
	if( face->useTexcoords())
	{
		for( i = 0; i < vertexCount; i++ )
		{
			memcpy( texcoords[i], face->getTexcoord( i ), sizeof( float[2] ));
		}
	}
	else
	{
		makeTexcoords( face->getPlane(), vertices, texcoords );
	}

	bool noRadar = false;
	bool noShadow = false;
	const BzMaterial *bzmat = face->getMaterial();
	if( bzmat != NULL )
	{
		noRadar = bzmat->getNoRadar();
		noShadow = bzmat->getNoShadow();
	}
	MeshPolySceneNode *node = new MeshPolySceneNode( face->getPlane(), noRadar, noShadow, vertices, normals, texcoords );

	return node;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void MeshSceneNodeGenerator::setupNodeMaterial( WallSceneNode *node, const BzMaterial *mat )
{
	// cheat a little
	(( BzMaterial* )mat )->setReference();

	TextureManager &tm = TextureManager::instance();
	OpenGLMaterial oglMaterial( mat->getSpecular(), mat->getEmission(), mat->getShininess());

	int userTexture = ( mat->getTextureCount() > 0 );
	int faceTexture =  - 1;
	bool gotSpecifiedTexture = false;
	if( userTexture )
	{
		const std::string &texname = mat->getTextureLocal( 0 );
		if( texname.size() > 0 )
		{
			faceTexture = tm.getTextureID( texname.c_str());
		}
		if( faceTexture >= 0 )
		{
			gotSpecifiedTexture = true;
		}
		else
		{
			faceTexture = tm.getTextureID( "mesh", false /* no failure reports */ );
		}
	}

	if( !mat->getNoLighting())
	{
		node->setMaterial( oglMaterial );
	}

	// NOTE: the diffuse color is used, and not the ambient color
	//       could use the ambient color for non-lighted,and diffuse
	//       for lighted
	const DynamicColor *dyncol = DYNCOLORMGR.getColor( mat->getDynamicColor());
	const GLfloat *dc = NULL;
	if( dyncol != NULL )
	{
		dc = dyncol->getColor();
	}
	node->setDynamicColor( dc );
	node->setColor( mat->getDiffuse()); // redundant, see below
	node->setModulateColor( mat->getDiffuse());
	node->setLightedColor( mat->getDiffuse());
	node->setLightedModulateColor( mat->getDiffuse());
	node->setTexture( faceTexture );
	if(( userTexture && mat->getUseColorOnTexture( 0 )) || !gotSpecifiedTexture )
	{
		// modulate with the color if asked to, or
		// if the specified texture was not available
		node->setUseColorTexture( false );
	}
	else
	{
		node->setUseColorTexture( true );
	}
	const int texMatId = mat->getTextureMatrix( 0 );
	const TextureMatrix *texmat = TEXMATRIXMGR.getMatrix( texMatId );
	if( texmat != NULL )
	{
		const GLfloat *matrix = texmat->getMatrix();
		if( matrix != NULL )
		{
			node->setTextureMatrix( matrix );
		}
	}

	// deal with the blending setting for textures
	bool alpha = false;
	if(( faceTexture >= 0 ) && ( userTexture && mat->getUseTextureAlpha( 0 )))
	{
		const ImageInfo &imageInfo = tm.getInfo( faceTexture );
		alpha = imageInfo.alpha;
	}
	node->setBlending( alpha );
	node->setSphereMap( mat->getUseSphereMap( 0 ));

	// the current color can also affect the blending.
	// if blending is disabled then the alpha value from
	// one of these colors is used to set the stipple value.
	// we'll just set it to the middle value.
	if( dyncol )
	{
		const float color[4] = 
		{
			1.0f, 1.0f, 1.0f, 0.5f
		}; // alpha value != 1.0f
		if( dyncol->canHaveAlpha())
		{
			node->setColor( color ); // trigger transparency check
			node->setModulateColor( color );
			node->setLightedColor( color );
			node->setLightedModulateColor( color );
		}
	}

	node->setAlphaThreshold( mat->getAlphaThreshold());
	node->setNoCulling( mat->getNoCulling());
	node->setNoSorting( mat->getNoSorting());
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


bool MeshSceneNodeGenerator::makeTexcoords( const float *plane, const GLfloat3Array &vertices, GLfloat2Array &texcoords )
{
	float x[3], y[3];

	vec3sub( x, vertices[1], vertices[0] );
	vec3cross( y, plane, x );

	float len = vec3dot( x, x );
	if( len > 0.0f )
	{
		len = 1.0f / sqrtf( len );
		x[0] = x[0] *len;
		x[1] = x[1] *len;
		x[2] = x[2] *len;
	}
	else
	{
		return false;
	}

	len = vec3dot( y, y );
	if( len > 0.0f )
	{
		len = 1.0f / sqrtf( len );
		y[0] = y[0] *len;
		y[1] = y[1] *len;
		y[2] = y[2] *len;
	}
	else
	{
		return false;
	}

	const float uvScale = 8.0f;

	texcoords[0][0] = 0.0f;
	texcoords[0][1] = 0.0f;
	const int count = vertices.getSize();
	for( int i = 1; i < count; i++ )
	{
		float delta[3];
		vec3sub( delta, vertices[i], vertices[0] );
		texcoords[i][0] = vec3dot( delta, x ) / uvScale;
		texcoords[i][1] = vec3dot( delta, y ) / uvScale;
	}

	return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
