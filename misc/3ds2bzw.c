//
// 3DS2BZW
//
// Author: Anonymous
// Date: Aug 26, 2004
//
// Utility program to convert 3DS model
// files into BZFlag 1.12 meshy goodness.
//
// To use the program, you might type this:
//
// ./3ds2bzw model.3ds > model.bzw
//
// NOTE: this program requires lib3ds to
// compile. to compile the program,
// use something like this:
//
// gcc -O3 -W -Wall -o 3ds2bzw 3ds2bzw.c -l3ds -lm
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>


static Lib3dsFile *File3DS = NULL;


static int Invert = 0;
static int ColorSwap = 0;
static int Colors = 1;
static int Ambient = 1;
static int Diffuse = 1;
static int Specular = 1;
static int Shininess = 1;
static int Normals = 1;
static int Textures = 1;


//////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
	Lib3dsMesh *mesh;
	const char *execname = argv[0];

	// collect the options
	while( argc > 1 )
	{
		if( strcmp( "-i", argv[1] ) == 0 )
		{
			Invert = 1;
		}
		else if( strcmp( "-cs", argv[1] ) == 0 )
		{
			ColorSwap = 1;
		}
		else if( strcmp( "-t", argv[1] ) == 0 )
		{
			Textures = 0;
		}
		else if( strcmp( "-n", argv[1] ) == 0 )
		{
			Normals = 0;
		}
		else if( strcmp( "-c", argv[1] ) == 0 )
		{
			Colors = 0;
		}
		else if( strcmp( "-a", argv[1] ) == 0 )
		{
			Ambient = 0;
		}
		else if( strcmp( "-d", argv[1] ) == 0 )
		{
			Diffuse = 0;
		}
		else if( strcmp( "-s", argv[1] ) == 0 )
		{
			Specular = 0;
		}
		else if( strcmp( "-sh", argv[1] ) == 0 )
		{
			Shininess = 0;
		}
		else
		{
			break;
		}
		argc--;
		argv++;
	}

	// print the help message
	if( argc != 2 )
	{
		printf( "\n" );
		printf( "usage:  %s [opts] <filename>\n", execname );
		printf( "\n" );
		printf( "  -t    disable textures\n" );
		printf( "  -n    disable normals\n" );
		printf( "  -c    disable all colors\n" );
		printf( "  -a    disable ambient\n" );
		printf( "  -d    disable diffuse\n" );
		printf( "  -s    disable specular\n" );
		printf( "  -sh   disable shininess\n" );
		printf( "  -i    invert normals\n" );
		printf( "  -cs   swap ambient and diffuse\n" );
		printf( "\n" );
		return 1;
	}

	// load the file
	File3DS = lib3ds_file_load( argv[1] );
	if( File3DS == NULL )
	{
		printf( "Problems loading file\n" );
		return 1;
	}

	// evaluate the first frame
	lib3ds_file_eval( File3DS, 0.0f /* the frame time */ );

	// dump all of the meshes
	for( mesh = File3DS->meshes; mesh != NULL; mesh = mesh->next )
	{
		// FIXME - Lib3dsMatrix* matrix = &mesh->matrix;
		// comments on statistics
		unsigned int i;
		printf( "mesh  # %s\n", mesh->name );
		printf( "# vertices:  %i\n", ( int )mesh->points );
		if( Normals )
		{
			printf( "# normals:   %i\n", ( int )mesh->faces *3 );
		}
		if( Textures )
		{
			printf( "# texcoords: %i\n", ( int )mesh->texels );
		}
		printf( "# faces:     %i\n", ( int )mesh->faces );

		// vertices
		for( i = 0; i < mesh->points; i++ )
		{
			Lib3dsPoint *point = &( mesh->pointL[i] );
			printf( "  vertex %f %f %f  # %i\n", point->pos[0], point->pos[1], point->pos[2], i );
		}

		// normals
		if( Normals )
		{
			Lib3dsVector *normals = ( Lib3dsVector* )malloc( 3 *mesh->faces *sizeof( Lib3dsVector ));
			lib3ds_mesh_calculate_normals( mesh, normals );
			for( i = 0; i < ( mesh->faces *3 ); i++ )
			{
				if( Invert )
				{
					printf( "  normal %f %f %f  # %i\n",  - normals[i][0],  - normals[i][1],  - normals[i][2], i );
				}
				else
				{
					printf( "  normal %f %f %f  # %i\n", normals[i][0], normals[i][1], normals[i][2], i );
				}
			}
			free( normals );
		}

		// texcoords
		if( Textures )
		{
			for( i = 0; i < mesh->texels; i++ )
			{
				Lib3dsTexel *texel = &( mesh->texelL[i] );
				printf( "  texcoord %f %f  # %i\n", ( *texel )[0], ( *texel )[1], i );
			}
		}

		// faces
		for( i = 0; i < mesh->faces; i++ )
		{
			Lib3dsFace *face = &( mesh->faceL[i] );
			Lib3dsWord *points = face->points;
			printf( "  face  # material = %s\n", face->material );
			printf( "    vertices %i %i %i\n", points[0], points[1], points[2] );
			if( Normals )
			{
				printf( "    normals %i %i %i\n", ( i *3 ) + 0, ( i *3 ) + 1, ( i *3 ) + 2 );
			}
			Lib3dsMaterial *mat = lib3ds_file_material_by_name( File3DS, face->material );
			if( mat )
			{
				if( Textures && ( mesh->texels != 0 ))
				{
					printf( "    texture %s\n", mat->texture1_map.name );
					printf( "    texcoords %i %i %i\n", points[0], points[1], points[2] );

					// BZ isn't ready for these, yet...
					// printf (" #texture %s\n", material->texture2_map.name);
					// printf (" #texture %s\n", material->texture1_mask.name);
					// printf (" #texture %s\n", material->texture2_mask.name);
				}
				if( Colors )
				{
					Lib3dsRgba *ambient = &mat->ambient;
					Lib3dsRgba *diffuse = &mat->diffuse;
					if( ColorSwap )
					{
						ambient = &mat->diffuse;
						diffuse = &mat->ambient;
					}
					if( Ambient )
					{
						printf( "    ambient %f %f %f %f\n", ( *ambient )[0], ( *ambient )[1], ( *ambient )[2], ( *ambient )[3] );
					}
					if( Diffuse )
					{
						printf( "    diffuse %f %f %f %f\n", ( *diffuse )[0], ( *diffuse )[1], ( *diffuse )[2], ( *diffuse )[3] );
					}
					if( Specular )
					{
						printf( "    specular %f %f %f %f\n", mat->specular[0], mat->specular[1], mat->specular[2], mat->specular[3] );
					}
					if( Shininess )
					{
						printf( "    shininess %f\n", mat->shininess );
					}
				}
			}
			printf( "  endface\n" );
		}

		printf( "end  # %s\n\n", mesh->name );
	}

	lib3ds_file_free( File3DS );

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
