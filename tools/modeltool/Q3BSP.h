// Quake 3 data definitions
// Copyright (C) 1999-2000 Id Software, Inc.
//
// This file must be identical in the quake and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!
#ifndef __Quake3_H__
#define __Quake3_H__

#include <string>
#include "model.h"

#define BSP_HEADER_ID   (*(int*)"IBSP")
#define BSP_HEADER_VER  (46)

#define BSP_ENTITIES_LUMP   (0)
#define BSP_SHADERS_LUMP    (1)
#define BSP_PLANES_LUMP     (2)
#define BSP_NODES_LUMP      (3)
#define BSP_LEAVES_LUMP     (4)
#define BSP_LFACES_LUMP     (5)
#define BSP_LBRUSHES_LUMP   (6)
#define BSP_MODELS_LUMP     (7)
#define BSP_BRUSH_LUMP      (8)
#define BSP_BRUSHSIDES_LUMP (9)
#define BSP_VERTICES_LUMP   (10)
#define BSP_ELEMENTS_LUMP   (11)
#define BSP_FOG_LUMP	(12)
#define BSP_FACES_LUMP      (13)
#define BSP_LIGHTMAPS_LUMP  (14)
#define BSP_LIGHTVOLS_LUMP  (15)
#define BSP_VISIBILITY_LUMP (16)

#define BSP_LIGHTMAP_BANKSIZE   (128*128*3)


#define    CONTENTS_SOLID	    1	// an eye is never valid in a solid
#define    CONTENTS_LAVA	    8
#define    CONTENTS_SLIME	    16
#define    CONTENTS_WATER	    32
#define    CONTENTS_FOG	    64

#define    CONTENTS_AREAPORTAL	0x8000

#define    CONTENTS_PLAYERCLIP	0x10000
#define    CONTENTS_MONSTERCLIP    0x20000
//bot specific contents types
#define    CONTENTS_TELEPORTER	0x40000
#define    CONTENTS_JUMPPAD	0x80000
#define CONTENTS_CLUSTERPORTAL    0x100000
#define CONTENTS_DONOTENTER	0x200000

#define    CONTENTS_ORIGIN	    0x1000000    // removed before bsping an entity

#define    CONTENTS_BODY	    0x2000000    // should never be on a brush, only in game
#define    CONTENTS_CORPSE	    0x4000000
#define    CONTENTS_DETAIL	    0x8000000    // brushes not used for the bsp
#define    CONTENTS_STRUCTURAL	0x10000000    // brushes used for the bsp
#define    CONTENTS_TRANSLUCENT    0x20000000    // don't consume surface fragments inside
#define    CONTENTS_TRIGGER	0x40000000
#define    CONTENTS_NODROP	    0x80000000    // don't leave bodies or items (death fog, lava)

#define    SURF_NODAMAGE	    0x1	// never give falling damage
#define    SURF_SLICK		0x2	// effects game physics
#define    SURF_SKY		0x4	// lighting from environment map
#define    SURF_LADDER		0x8
#define    SURF_NOIMPACT	    0x10    // don't make missile explosions
#define    SURF_NOMARKS	    0x20    // don't leave missile marks
#define    SURF_FLESH		0x40    // make flesh sounds and effects
#define    SURF_NODRAW		0x80    // don't generate a drawsurface at all
#define    SURF_HINT		0x100    // make a primary bsp splitter
#define    SURF_SKIP		0x200    // completely ignore, allowing non-closed brushes
#define    SURF_NOLIGHTMAP	    0x400    // surface doesn't need a lightmap
#define    SURF_POINTLIGHT	    0x800    // generate lighting info at vertexes
#define    SURF_METALSTEPS	    0x1000    // clanking footsteps
#define    SURF_NOSTEPS	    0x2000    // no footstep sounds
#define    SURF_NONSOLID	    0x4000    // don't collide against curves with this set
#define SURF_LIGHTFILTER	0x8000    // act as a light filter during q3map -light
#define    SURF_ALPHASHADOW	0x10000    // do per-pixel light shadow casting in q3map
#define    SURF_NODLIGHT	    0x20000    // don't dlight even if solid (solid lava, skies)

/* Shader flags */
enum
{
	SHADER_NOCULL	= 1 << 0,
	SHADER_TRANSPARENT   = 1 << 1,
	SHADER_DEPTHWRITE    = 1 << 2,
	SHADER_SKY	   = 1 << 3,
	SHADER_NOMIPMAPS     = 1 << 4,
	SHADER_NEEDCOLOURS   = 1 << 5,
	SHADER_DEFORMVERTS   = 1 << 6
};

/* Shaderpass flags */
enum
{
	SHADER_LIGHTMAP   = 1 << 0,
	SHADER_BLEND      = 1 << 1,
	SHADER_ALPHAFUNC  = 1 << 3,
	SHADER_TCMOD      = 1 << 4,
	SHADER_ANIMMAP    = 1 << 5,
	SHADER_TCGEN_ENV  = 1 << 6
};

/* Transform functions */
enum WaveType
{
	SHADER_FUNC_NONE	    = 0,
	SHADER_FUNC_SIN	     = 1,
	SHADER_FUNC_TRIANGLE	= 2,
	SHADER_FUNC_SQUARE	  = 3,
	SHADER_FUNC_SAWTOOTH	= 4,
	SHADER_FUNC_INVERSESAWTOOTH = 5
};

/* *Gen functions */
enum GenFunc
{
	SHADER_GEN_IDENTITY = 0,
	SHADER_GEN_WAVE     = 1,
	SHADER_GEN_VERTEX   = 2
};

enum TexGen
{
	TEXGEN_BASE = 0,	// Coord set 0
	TEXGEN_LIGHTMAP = 1,    // Coord set 1
	TEXGEN_ENVIRONMENT = 2  // Neither, generated
};

enum DeformFunc
{
	DEFORM_FUNC_NONE = 0,
	DEFORM_FUNC_BULGE = 1,
	DEFORM_FUNC_WAVE = 2,
	DEFORM_FUNC_NORMAL = 3,
	DEFORM_FUNC_MOVE = 4,
	DEFORM_FUNC_AUTOSPRITE = 5,
	DEFORM_FUNC_AUTOSPRITE2 = 6

};
/////////////////////////////////////////////////////////
//
// bsp contents
//

struct bsp_plane_t {
	float normal[3];
	float dist;
};

struct bsp_model_t {
	float bbox[6];
	int face_start;
	int face_count;
	int brush_start;
	int brush_count;
};

struct bsp_node_t {
	int plane;	  // dividing plane
	//int children[2];    // left and right nodes,
	// negative are leaves
	int front;
	int back;
	int bbox[6];
};

struct bsp_leaf_t {
	int cluster;    // visibility cluster number
	int area;
	int bbox[6];
	int face_start;
	int face_count;
	int brush_start;
	int brush_count;
};

#define BSP_FACETYPE_NORMAL (1)
#define BSP_FACETYPE_PATCH  (2)
#define BSP_FACETYPE_MESH   (3)
#define BSP_FACETYPE_FLARE  (4)

struct bsp_face_t {
	int shader;	 // shader ref
	int unknown;
	int type;	   // face type
	int vert_start;
	int vert_count;
	int elem_start;
	int elem_count;
	int lm_texture;     // lightmap
	int lm_offset[2];
	int lm_size[2];
	float org[3];       // facetype_normal only
	float bbox[6];      // facetype_patch only
	float normal[3];    // facetype_normal only
	int mesh_cp[2];     // patch control point dims
};

struct bsp_shader_t {
	char name[64];
	int surface_flags;
	int content_flags;
};

struct bsp_vertex_t {
	float point[3];
	float texture[2];
	float lightmap[2];
	float normal[3];
	int color;
};

struct bsp_vis_t {
	int cluster_count;
	int row_size;
	unsigned char data[1];
};

// OGRE additions
struct bsp_lump_entry_t {
	int offset;
	int size;
};
struct bsp_header_t {
	char magic[4];
	int version;
	bsp_lump_entry_t lumps[17];
};

//
// Brushes sides in BSP tree
//
struct bsp_brushside_t {
	int planenum;
	int content;			// ¿?shader¿?
};


//
// Brushes in BSP tree
//
struct bsp_brush_t {
	int firstside;
	int numsides;
	int shaderIndex;
};


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

/** Support for loading and extracting data from a Quake3 level file.
This class implements the required methods for opening Quake3 level files
and extracting the pertinent data within. Ogre supports BSP based levels
through it's own BspLevel class, which is not specific to any file format,
so this class is here to source that data from the Quake3 format.</p>
Quake3 levels include far more than just data for rendering - typically the
<strong>leaves</strong> of the tree are used for rendering, and <strong>brushes,</strong>
are used to    define convex hulls made of planes for collision detection. There are also
<strong>entities</strong> which define non-visual elements like player start
points, triggers etc and <strong>models</strong> which are used for movable
scenery like doors and platforms. <strong>Shaders</strong> meanwhile are textures
with extra effects and 'content flags' indicating special properties like
water or lava.</p>
I will try to support as much of this as I can in Ogre, but I won't duplicate
the structure or necesarily use the same terminology. Quake3 is designed for a very specific
purpose and code structure, whereas Ogre is designed to be more flexible,
so for example I'm likely to separate game-related properties like surface flags
from the generics of materials in my implementation.</p>
This is a utility class only - a single call to loadFromChunk should be
enough. You should not expect the state of this object to be consistent
between calls, since it uses pointers to memory which may no longer
be valid after the original call. This is why it has no accessor methods
for reading it's internal state.
*/
class Quake3Level
{
public:
	Quake3Level();
	~Quake3Level();

	/** Reads Quake3 bsp data from a chunk of memory as read from the file.
	Since ResourceManagers generally locate data in a variety of
	places they typically manipulate them as a chunk of data, rather than
	a file pointer since this is unsupported through compressed archives.</p>
	Quake3 files are made up of a header (which contains version info and
	a table of the contents) and 17 'lumps' i.e. sections of data,
	the offsets to which are kept in the table of contents. The 17 types
	are predefined (You can find them in OgreQuake3Types.h)

	@param inChunk Input chunk of memory containing Quake3 data
	*/
//	void loadFromChunk(DataChunk& inChunk);
	void loadFromFile(const char* filename);

	/* Extracts the embedded lightmap texture data and loads them as textures.
	Calling this method makes the lightmap texture data embedded in
	the .bsp file available to the renderer. Lightmaps are extracted
	and loaded as Texture objects (subclass specific to RenderSystem
	subclass) and are named "@lightmap1", "@lightmap2" etc.
	*/
	void extractLightmaps(void) const;

	/** Utility function read the header and set up pointers. */
	void initialise(void);

	/** Utility function to return a pointer to a lump. */
	void* getLump(int lumpType);
	int getLumpSize(int lumpType);


	/** Debug method. */
	void dumpContents(void);

	bool dumpToModel ( CModel &model );

	// Internal storage
	// This is ALL temporary. Don't rely on it being static

	// NB no brushes, fog or local lightvolumes yet
//	DataChunk mChunk;
	std::string theFileName;

	char *data;

	bsp_header_t* mHeader;
	unsigned char* mLumpStart;

	int* mElements; // vertex indexes for faces
	int mNumElements;

	void* mEntities;
	int mNumEntities;

	bsp_model_t* mModels;
	int mNumModels;

	bsp_node_t* mNodes;
	int mNumNodes;

	bsp_leaf_t* mLeaves;
	int mNumLeaves;

	int* mLeafFaces;     // Indexes to face groups by leaf
	int mNumLeafFaces;

	bsp_plane_t* mPlanes;
	int mNumPlanes;

	bsp_face_t* mFaces;      // Groups of faces
	int mNumFaces;

	bsp_vertex_t* mVertices;
	int mNumVertices;

	bsp_shader_t* mShaders;
	int mNumShaders;

	unsigned char* mLightmaps;
	int mNumLightmaps;

	bsp_vis_t* mVis;

	bsp_brush_t* mBrushes;
	int mNumBrushes;

	bsp_brushside_t* mBrushSides;
	int mNumBrushSides;

	int* mLeafBrushes;      // Groups of indexes to brushes by leaf
	int mNumLeafBrushes;
};

#endif
