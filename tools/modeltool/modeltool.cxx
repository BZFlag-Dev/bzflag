/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
//
// modeltool.cpp : Defines the entry point for the console application.
//
//
// If you're doing a command line compilation, and
// the Makefile is broken, then this may do the job:
//
//   g++ -o modeltool modeltool.cxx  -I../../include ../../src/common/libCommon.a
//

#include "common.h"

/* system headers */
#include <stdio.h>
#include <string>
#include <vector>
#include <map>

/* common headers */
#include "TextUtils.h"

#include "model.h"
#include "wavefrontOBJ.h"
#include "Q3BSP.h"

// globals/
const char VersionString[] = "ModelTool v1.8  (WaveFront OBJ/BZW to BZFlag BZW converter)";

std::string texdir = "";
std::string groupName = "";
bool useMaterials = true;
bool useAmbient = true;
bool useDiffuse = true;
bool useSpecular = true;
bool useShininess = true;
bool useEmission = true;
bool useNormals = true;
bool useTexcoords = true;
bool flipYZ = false;
bool useSmoothBounce = false;
float shineFactor = 1.0f;

float maxShineExponent = 128.0f; // OpenGL minimum shininess


float globalScale = 1.0f;
float globalShift[3] = {0,0,0};
std::vector<std::string> bspMaterialSkips; // materials to skip in a bsp map

typedef struct
{
  std::string staticFile;
  std::string boundingFile;
  std::vector<std::string> lodFiles;
  std::vector<float> lodPixelDistances;
  std::vector<std::string>  animComands;
}DrawInfoConfig;

typedef struct
{
  CModel staticMesh;
  CModel boundingMesh;
  std::vector<CModel> lodMeshes;
  std::vector<float>  lodPixelDistances;
  std::vector<std::string>  animComands;

  bool valid ( void  )
  {
    if (staticMesh.meshes.size())
      return true;
    if (boundingMesh.meshes.size())
      return true;
    if (lodMeshes.size())
      return true;

    return false;
  }
}DrawInfoMeshes;

void parseDrawInfoConfig ( DrawInfoConfig &config, std::string file );
void buildDrawInfoMeshesFromConfig ( DrawInfoConfig &config, DrawInfoMeshes &drawInfoMeshes );
void writeDrawInfoBZW ( DrawInfoMeshes &drawInfoMeshes, std::string file );

std::string writeMaterial ( CMaterial &material, const std::string &name )
{
  std::string out;

  out += TextUtils::format("material\n  name %s\n",name.c_str());
  if ( material.texture.size())
  {
    std::string texName = texdir + material.texture;
    // change the extension to png
    char *p = strrchr(texName.c_str(), '.');
    if (p) 
    {
      texName.resize(p - texName.c_str());
      texName += ".png";
    }
    else 
      texName += ".png";
    out += TextUtils::format("  texture %s\n", texName.c_str());
  }
  else
    out += TextUtils::format("  notextures\n");

  if (useAmbient)
    out += TextUtils::format("  ambient %f %f %f %f\n", material.ambient[0], material.ambient[1], material.ambient[2], material.ambient[3]);
  if (useDiffuse)
    out += TextUtils::format("  diffuse %f %f %f %f\n", material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]);
  if (useSpecular)
    out += TextUtils::format("  specular %f %f %f %f\n", material.specular[0], material.specular[1], material.specular[2], material.specular[3]);
  if (useShininess)
    out += TextUtils::format("  shininess %f\n", material.shine);
  if (useEmission)
    out += TextUtils::format("  emission %f %f %f %f\n", material.emission[0], material.emission[1], material.emission[2], material.emission[3]);

  out += TextUtils::format("end\n\n");
  return out;
}

static void writeBZW  ( CModel &model, std::string file )
{
  if (model.meshes.size() < 1 )
    return;

  FILE *fp = fopen (file.c_str(),"wt");
  if (!fp)
    return;

  if (useMaterials)
  {
    tmMaterialMap::iterator materialItr = model.materials.begin();
    while ( materialItr != model.materials.end() )
    {
      fprintf (fp,"%s",writeMaterial(materialItr->second,materialItr->first).c_str());
      materialItr++;
    }
    fprintf (fp,"\n");
  }

  if (groupName.size() > 0)
    fprintf (fp, "define %s\n", groupName.c_str());

  tvMeshList::iterator	meshItr = model.meshes.begin();

  while ( meshItr != model.meshes.end() )
  {
    CMesh	&mesh = *meshItr;

    mesh.reindex();

    fprintf (fp,"mesh # %s\n", mesh.name.c_str());

    fprintf (fp,"# vertices: %d\n", (int)mesh.verts.size());
    fprintf (fp,"# normals: %d\n", (int)mesh.normals.size());
    fprintf (fp,"# texcoords: %d\n", (int)mesh.texCoords.size());
    fprintf (fp,"# faces: %d\n\n", (int) mesh.faces.size());

    if (useSmoothBounce)
      fprintf (fp,"  smoothbounce\n");

    tvVertList::iterator vertItr = mesh.verts.begin();
    while ( vertItr != mesh.verts.end() )
    {
      fprintf (fp,"  vertex %f %f %f\n", vertItr->x*globalScale+globalShift[0],vertItr->y*globalScale+globalShift[1],vertItr->z*globalScale+globalShift[2]);
      vertItr++;
    }

    vertItr = mesh.normals.begin();
    while ( vertItr != mesh.normals.end() )
    {
      // normalise all normals before writing them
      float dist = sqrt(vertItr->x*vertItr->x+vertItr->y*vertItr->y+vertItr->z*vertItr->z);
      fprintf (fp,"  normal %f %f %f\n", vertItr->x/dist,vertItr->y/dist,vertItr->z/dist);
      vertItr++;
    }

    tvTexCoordList::iterator uvItr = mesh.texCoords.begin();
    while ( uvItr != mesh.texCoords.end() )
    {
      fprintf (fp,"  texcoord %f %f\n", uvItr->u,uvItr->v);
      uvItr++;
    }

    tvFaceList::iterator	faceItr = mesh.faces.begin();
    while ( faceItr != mesh.faces.end() )
    {
      CFace	&face = *faceItr;

      fprintf (fp,"  face\n");

      tvIndexList::iterator	indexItr = face.verts.begin();
      fprintf (fp,"    vertices");
      while ( indexItr != face.verts.end() )
	fprintf(fp," %d",*indexItr++);

      fprintf (fp,"\n");

      if (useNormals && (face.normals.size() > 0))
      {
	indexItr = face.normals.begin();
	fprintf (fp,"    normals");
	while ( indexItr != face.normals.end() )
	fprintf(fp," %d",*indexItr++);
	fprintf (fp,"\n");
      }

      if (useTexcoords && (face.texCoords.size() > 0))
      {
	indexItr = face.texCoords.begin();
	fprintf (fp,"    texcoords");
	while ( indexItr != face.texCoords.end() )
	  fprintf(fp," %d",*indexItr++);

	fprintf (fp,"\n");
      }

      if (useMaterials && (face.material.size() > 0)) 
	fprintf (fp, "    matref %s\n", face.material.c_str());

      fprintf (fp,"  endface\n");
      faceItr++;
    }
    fprintf (fp,"end\n\n");
    meshItr++;
  }

  if (groupName.size() > 0) 
    fprintf (fp, "enddef # %s\n", groupName.c_str());

  // do the custom objects.
  for ( unsigned int i = 0; i < model.customObjects.size(); i++ )
  {
    fprintf (fp, "%s\n", model.customObjects[i].name.c_str());
    for (unsigned int j = 0; j < model.customObjects[i].params.size(); j++ )
      fprintf (fp, "  %s\n", model.customObjects[i].params[j].c_str());
    fprintf (fp, "end\n\n");
  }

  fclose(fp);
}

static int  dumpUsage ( char *exeName, const char* reason )
{
  printf("\n%s\n\n", VersionString);
  printf("error: %s\n\n",reason);
  printf("usage: %s <input_file_name> [options]\n\n", exeName);
  printf("       -g <name>  : use group definition\n");
  printf("       -tx <dir>  : set texture prefix\n");
  printf("       -sm	: use the smoothbounce property\n");
  printf("       -yz	: flip y and z coordinates\n");
  printf("       -n	 : disable normals\n");
  printf("       -t	 : disable texture coordinates\n");
  printf("       -m	 : disable materials\n");
  printf("       -a	 : disable ambient coloring\n");
  printf("       -d	 : disable diffuse coloring\n");
  printf("       -s	 : disable specular coloring\n");
  printf("       -sh	: disable shininess\n");
  printf("       -sf <val>  : shine multiplier\n");
  printf("       -e	 : disable emission coloring\n\n");
  printf("       -gx <val>  : scale the model by this factor\n\n");
  printf("       -gsx <val> : shift the map by this value in X\n\n");
  printf("       -gsy <val> : shift the map by this value in Y\n\n");
  printf("       -gsz <val> : shift the map by this value in Z\n\n");
  printf("       -bspskip <val> : skip faces with this material when importing a bsp\n\n");
  return 1;
}

int main(int argc, char* argv[])
{
  std::string input;
  std::string extenstion = "OBJ";
  std::string output;

  // make sure we have all the right stuff
  if ( argc < 2)
    return dumpUsage(argv[0],"No input file specified");

  // get the input file
  // check argv for
  if ( argv[1][0] == '\"' )
  {
    argv[1]++;
    argv[1][strlen(argv[1])-1] = 0;
  }
  input = argv[1];

    // see if it has an extenstion
    char *p = strrchr(argv[1],'.');
    if (p)
	    extenstion = p+1;

  if (!p) 
    output = input + ".bzw";
  else 
  {
    *p = '\0'; // clip the old extension
    output = argv[1] + std::string(".bzw");
  }

  for ( int i = 2; i < argc; i++)
  {
    std::string command = argv[i];
    command = TextUtils::tolower(command);

    if (command == "-yz")
      flipYZ = true;
    else if (command == "-g")
    {
      if ((i + 1) < argc)
      {
	i++;
	groupName = argv[i];
      }
      else 
	printf ("missing -g argument\n");
    }
    else if (command == "-tx") 
    {
      if ((i + 1) < argc)
      {
	i++;
	texdir = argv[i];

	if (texdir[texdir.size()] != '/') 
	  texdir += '/';
      }
      else 
	printf ("missing -tx argument\n");
    }
    else if (command == "-sm") 
      useSmoothBounce = true;
    else if (command == "-n") 
      useNormals = false;
    else if (command == "-t")
      useTexcoords = false;
    else if (command == "-m")
      useMaterials = false;
    else if (command == "-a") 
      useAmbient = false;
    else if (command == "-d") 
      useDiffuse = false;
    else if (command == "-s")
      useSpecular = false;
    else if (command == "-sh") 
      useShininess = false;  
    else if (command == "-sf")
    {
      if ((i + 1) < argc)
      {
	i++;
	shineFactor = (float)atof(argv[i]);
      }
      else
	printf ("missing -sf argument\n");
    }
    else if (command == "-e") 
	    useEmission = false;
    else if (command == "-gx")
    {
      if ((i + 1) < argc)
      {
	i++;
	globalScale = (float)atof(argv[i]);
      }
      else
	printf ("missing -gx argument\n");
    }
    else if (command == "-gsx")
    {
      if ((i + 1) < argc)
      {
	i++;
	globalShift[0] = (float)atof(argv[i]);
      }
      else 
	printf ("missing -gsx argument\n");
    }
    else if (command == "-gsy")
    {
      if ((i + 1) < argc) 
      {
	i++;
	globalShift[1] = (float)atof(argv[i]);
      }
      else
	printf ("missing -gsy argument\n");
    }
    else if (command == "-gsz")
    {
      if ((i + 1) < argc)
      {
	i++;
	globalShift[2] = (float)atof(argv[i]);
      }
      else
	printf ("missing -gsz argument\n");
    }
    else if (command == "-bspskip")
    {
      if ((i + 1) < argc)
      {
	i++;
	bspMaterialSkips.push_back(std::string(argv[i]));
      }
      else
	printf ("missing -bspskip argument\n");
    }
  }
  // make a model

  CModel	model;

  if ( TextUtils::tolower(extenstion) == "obj" )
    readOBJ(model,input);
  else if ( TextUtils::tolower(extenstion) == "bsp" )
  {
    Quake3Level	level;
    level.loadFromFile(input.c_str());
    level.dumpToModel(model);
  }
  else if ( TextUtils::tolower(extenstion) == "diconf" )
  {
    DrawInfoConfig  config;
    DrawInfoMeshes  meshes;
    parseDrawInfoConfig(config,input);
    buildDrawInfoMeshesFromConfig(config,meshes);

    writeDrawInfoBZW(meshes,output);

    if ( meshes.valid())
      printf("no valid meshes written from %s\n", input.c_str());
    else
      printf("%s file %s converted to BZW as %s\n", extenstion.c_str(),input.c_str(),output.c_str());
    return 0; 
  }
  else
  {
    printf("unknown input format\n");
    return 2;
  }

  model.pushAboveAxis(eZAxis);

  if (model.meshes.size() > 0)
  {
    writeBZW(model,output);
    printf("%s file %s converted to BZW as %s\n", extenstion.c_str(),input.c_str(),output.c_str());
  }
  else 
    printf("no valid meshes written from %s\n", input.c_str());

  return 0; 
}

static int getNewIndex ( CVertex &vert, tvVertList &vertList )
{
  tvVertList::iterator itr = vertList.begin();

  int count = 0;
  while ( itr != vertList.end() )
  {
    if ( itr->same(vert) )
      return count;
    count++;
    itr++;
  }
  vertList.push_back(vert);
  return count;
}

static int getNewIndex ( CTexCoord &vert, tvTexCoordList &vertList )
{
  tvTexCoordList::iterator itr = vertList.begin();

  int count = 0;
  while ( itr != vertList.end() )
  {
    if ( itr->same(vert) )
      return count;
    count++;
    itr++;
  }
  vertList.push_back(vert);
  return count;
}

void CMesh::reindex ( void )
{
  tvVertList		temp_verts;
  tvVertList		temp_normals;
  tvTexCoordList	temp_texCoords;

  tvFaceList::iterator	faceItr = faces.begin();
  while ( faceItr != faces.end() )
  {
    CFace	&face = *faceItr;
    CFace	newFace;

    newFace.material = face.material;

    tvIndexList::iterator indexItr = face.verts.begin();
    while ( indexItr != face.verts.end() )
      newFace.verts.push_back(getNewIndex(verts[*indexItr++],temp_verts));

    indexItr = face.normals.begin();
    while ( indexItr != face.normals.end() )
      newFace.normals.push_back(getNewIndex(normals[*indexItr++],temp_normals));

    indexItr = face.texCoords.begin();
    while ( indexItr != face.texCoords.end() )
      newFace.texCoords.push_back(getNewIndex(texCoords[*indexItr++],temp_texCoords));

    *faceItr = newFace;
    faceItr++;
  }
  verts = temp_verts;
  normals = temp_normals;
  texCoords = temp_texCoords;
}

void parseDrawInfoConfig ( DrawInfoConfig &config, std::string file )
{
  std::string text;
  FILE	*fp = fopen(file.c_str(),"rb");
  if (!fp)
    return;

  fseek(fp,0,SEEK_END);
  int size = ftell(fp);
  fseek(fp,0,SEEK_SET);

  if(size)
  {
    char *t =(char*)malloc(size+1);
    fread(t,size,1,fp);
    t[size] = 0;
    text = t;
    free(t);
  }
  fclose(fp);

  if (!size)
    return;

  text = TextUtils::replace_all(text,std::string("\n"),std::string(""));
  std::vector<std::string> lines = TextUtils::tokenize(text,std::string("\r"));
  if (!lines.size())
    return;

  for ( int i = 0; i < (int)lines.size(); i++ )
  {
    std::string &line = lines[i];
    if (!line.size())
      continue;

    std::vector<std::string> chunks = TextUtils::tokenize(line,std::string(" "),0,true);
    if (!chunks.size())
      continue;

    if (TextUtils::tolower(chunks[0]) == "static")
      config.staticFile = chunks[1];
    else if (TextUtils::tolower(chunks[0]) == "bounding")
      config.boundingFile = chunks[1];
    else if (TextUtils::tolower(chunks[0]) == "anim" )
      config.animComands.push_back(chunks[1]);
    else if (TextUtils::tolower(chunks[0]) == "lod")
    {
      if ( chunks.size() > 2 )
      {
	config.lodPixelDistances.push_back((float)atof(chunks[1].c_str()));
	config.lodFiles.push_back(chunks[2].c_str());
      }
    }
  }
}

void buildDrawInfoMeshesFromConfig ( DrawInfoConfig &config, DrawInfoMeshes &drawInfoMeshes )
{
  if (config.staticFile.size())
    readOBJ(drawInfoMeshes.staticMesh,config.staticFile);

  if (config.boundingFile.size())
    readOBJ(drawInfoMeshes.boundingMesh,config.boundingFile);

  for ( int i = 0; i < (int)config.lodFiles.size(); i++ )
  {
    CModel  model;
    readOBJ(model,config.lodFiles[i]);
    if (model.meshes.size())
    {
      drawInfoMeshes.lodMeshes.push_back(model);
      drawInfoMeshes.lodPixelDistances.push_back(config.lodPixelDistances[i]);
    }
  }
  drawInfoMeshes.animComands = config.animComands;
}

typedef struct 
{
  float cpx,cpy,cpz;
  float rad;
  float minx,miny,minz,maxx,maxy,maxz;
}MeshExtents;

bool computeExtents ( CModel &model, MeshExtents &extents )
{
  bool didOne = false;
  for ( int m = 0; m < (int)model.meshes.size(); m++ )
  {
    CMesh &subMesh = model.meshes[m];
    for (int f = 0; f < (int)subMesh.faces.size();f++)
    {
      didOne = true;
      CFace &face = subMesh.faces[f];
      if (f == 0 && m ==0)
      {
	extents.minx = subMesh.verts[face.verts[0]].x;
	extents.miny = subMesh.verts[face.verts[0]].y;
	extents.minz = subMesh.verts[face.verts[0]].z;

	extents.maxx = extents.minx;
	extents.maxy = extents.miny;
	extents.maxz = extents.minz;
      }
  
      for ( int v = 0; v < (int)face.verts.size(); v++ )
      {
	CVertex &vert = subMesh.verts[face.verts[v]];
	if ( vert.x < extents.minx )
	  extents.minx = vert.x;
	if ( vert.y < extents.miny )
	  extents.miny = vert.y;
	if ( vert.z < extents.minz )
	  extents.minz = vert.z;

	if ( vert.x > extents.maxx )
	  extents.maxx = vert.x;
	if ( vert.y > extents.maxy )
	  extents.maxy = vert.y;
	if ( vert.z > extents.maxz )
	  extents.maxz = vert.z;
      }
    }
  }

  float dx,dy,dz;

  dx = extents.maxx-extents.minx;
  dy = extents.maxy-extents.miny;
  dz = extents.maxz-extents.minz;

  extents.cpx = extents.minx + dx*0.5f;
  extents.cpy = extents.miny + dy*0.5f;
  extents.cpz = extents.minz + dz*0.5f;

  extents.rad = sqrtf(dx*dx+dy*dy+dz*dz)*0.5f;
  return didOne;
}

bool computeExtents ( CModel &model, CMesh &subMesh, MeshExtents &extents )
{
  bool didOne = false;
  for (int f = 0; f < (int)subMesh.faces.size();f++)
  {
    CFace &face = subMesh.faces[f];
    didOne = true;
    if (f == 0)
    {
      extents.minx = subMesh.verts[face.verts[0]].x;
      extents.miny = subMesh.verts[face.verts[0]].y;
      extents.minz = subMesh.verts[face.verts[0]].z;

      extents.maxx = extents.minx;
      extents.maxy = extents.miny;
      extents.maxz = extents.minz;
    }

    for ( int v = 0; v < (int)face.verts.size(); v++ )
    {
      CVertex &vert = subMesh.verts[face.verts[v]];
      if ( vert.x < extents.minx )
	extents.minx = vert.x;
      if ( vert.y < extents.miny )
	extents.miny = vert.y;
      if ( vert.z < extents.minz )
	extents.minz = vert.z;

      if ( vert.x > extents.maxx )
	extents.maxx = vert.x;
      if ( vert.y > extents.maxy )
	extents.maxy = vert.y;
      if ( vert.z > extents.maxz )
	extents.maxz = vert.z;
    }
  }

  float dx,dy,dz;

  dx = extents.maxx-extents.minx;
  dy = extents.maxy-extents.miny;
  dz = extents.maxz-extents.minz;

  extents.cpx = extents.minx + dx*0.5f;
  extents.cpy = extents.miny + dy*0.5f;
  extents.cpz = extents.minz + dz*0.5f;

  extents.rad = sqrtf(dx*dx+dy*dy+dz*dz)*0.5f;
  return didOne;
}


int computeCorner ( CMesh &mesh, CFace &face, int index, tvVertList &v, tvVertList &n, tvTexCoordList &u, tvVertList &c )
{
  int vertIndex = getNewIndex(mesh.verts[face.verts[index]],v);
  int normalIndex = getNewIndex(mesh.normals[face.normals[index]],n);
  int uvIndex = getNewIndex(mesh.texCoords[face.texCoords[index]],u);


  CVertex vert;
  vert.x = (float)vertIndex;
  vert.y = (float)normalIndex;
  vert.z = (float)uvIndex;

  return getNewIndex(vert,c);
}

void writeDrawInfoBZW ( DrawInfoMeshes &drawInfoMeshes, std::string file )
{
  if (!drawInfoMeshes.valid())
    return;

// the idea here is to go and output each of the mesh sections into 
  // seperate buffers, bulding up the actual used vert and index lists.
  // then dump out those lists to a buffer, and composite the entire
  // thing into one bzw.
  // This way we can do things in any order and not worry about
  // duplicating indexes.
  std::string materialsSection;
  std::string inxexesSection;
  std::string staticGeoSection;
  std::string boundingGeoSection;
  std::string drawInfoSection;

  std::string invisibleMatName = "bounding.invisible";

  // the 3 major lists
  tvVertList  verts;
  tvVertList  norms;
  tvTexCoordList  uvs;

  // this is cheap, each corner is unique instance of a vert, normal, and uv coordinate.
  // even tho they are ints we can store them like a vert, and just cast them to ints.
  // to leverage the existing functions for sorting indexes.
  tvVertList  corners;

  // build the static geo into it's buffer
  if ( drawInfoMeshes.staticMesh.meshes.size())
  {
    CModel &staticModel = drawInfoMeshes.staticMesh;
    for ( int m = 0; m < (int)staticModel.meshes.size(); m++ )
    {
      CMesh &subMesh = staticModel.meshes[m];
      for (int f = 0; f < (int)subMesh.faces.size();f++)
      {
	CFace &face = subMesh.faces[f];

	staticGeoSection += "face\n";
	std::string vert = "vertices";
	std::string norm = "normals";
	std::string uv = "texcoords";

	for ( int v = 0; v < (int)face.verts.size(); v++ )
	{
	  vert += TextUtils::format(" %d",getNewIndex(subMesh.verts[face.verts[v]],verts));
	  norm += TextUtils::format(" %d",getNewIndex(subMesh.normals[face.normals[v]],norms));
	  if (face.texCoords.size())
	    uv += TextUtils::format(" %d",getNewIndex(subMesh.texCoords[face.texCoords[v]],uvs));
	}
	staticGeoSection += vert + "\n" + norm + "\n";
	if ( face.texCoords.size())
	  staticGeoSection += uv + "\n";

	if (useMaterials)
	  staticGeoSection += "matref " + face.material + "\n";
	staticGeoSection += "end\n\n";
      }
    }
  }

  // write the bounding geo to the bufer, with the invisible mesh
  if ( drawInfoMeshes.boundingMesh.meshes.size() )
  {
    CModel &boundingMesh = drawInfoMeshes.boundingMesh;
    for ( int m = 0; m < (int)boundingMesh.meshes.size(); m++ )
    {
      CMesh &subMesh = boundingMesh.meshes[m];
      for (int f = 0; f < (int)subMesh.faces.size();f++)
      {
	CFace &face = subMesh.faces[f];

	staticGeoSection += "face\n";
	std::string vert = "vertices";
	std::string norm = "normals";

	for ( int v = 0; v < (int)face.verts.size(); v++ )
	{
	  vert += TextUtils::format(" %d",getNewIndex(subMesh.verts[face.verts[v]],verts));
	  norm += TextUtils::format(" %d",getNewIndex(subMesh.normals[face.normals[v]],norms));
	}
	staticGeoSection += vert + "\n" + norm + "\n";

	staticGeoSection += "matref " + invisibleMatName + "\n";
	staticGeoSection += "endface\n\n";
      }
    }
  }

  // now we are doing the real draw info part
  // the first model must be valid, since we use that as lod 0, and for bounds calcs
 if (drawInfoMeshes.lodMeshes.size() && drawInfoMeshes.lodMeshes[0].meshes.size() )
  {
    // use the bounds of the first LOD for the bounds of the draw info
    
    std::string cornerSection;
    std::vector<std::string> lodSections;

    MeshExtents	lod0Extents;
    if (computeExtents(drawInfoMeshes.lodMeshes[0],lod0Extents))
    {
      drawInfoSection += "drawInfo\n";
      drawInfoSection += TextUtils::format("extents %f %f %f %f %f %f\n",lod0Extents.minx,lod0Extents.miny,lod0Extents.minz,lod0Extents.maxx,lod0Extents.maxy,lod0Extents.maxz);
      drawInfoSection += TextUtils::format("sphere %f %f %f %f\n",lod0Extents.cpx,lod0Extents.cpy,lod0Extents.cpz,lod0Extents.rad);

      for ( int a = 0; a < drawInfoMeshes.animComands.size(); a++ )
	drawInfoSection += drawInfoMeshes.animComands[a] + "\n";

      // compute the LOD sections
      for ( int l = 0; l < (int)drawInfoMeshes.lodMeshes.size(); l++ )
      {
	CModel &lodModel = drawInfoMeshes.lodMeshes[l];
	std::string section;
	if ( lodModel.meshes.size() )
	{
	  section += TextUtils::format("lod #%d\n",l );
	  if (l == 0)
	    section += "lengthPerPixel 0\n";
	  else
	    section += TextUtils::format("lengthPerPixel %f\n",drawInfoMeshes.lodPixelDistances[l]);
	  
	  for ( int m = 0; m < (int)lodModel.meshes.size(); m++ )
	  {
	    CMesh &mesh = lodModel.meshes[m];
	    if ( mesh.faces.size())
	    {
	      // we always use the first face's material for the lod
	      section += "matref ";
	      if (mesh.faces[0].material.size())
		section += mesh.faces[0].material.size();
	      else
		section += "-1";
	      section += "\n";

	      section += "dlist\n";

	      MeshExtents subMeshExtents;

	      computeExtents(lodModel,mesh,subMeshExtents);
	      section += TextUtils::format("sphere %f %f %f %f\n",subMeshExtents.cpx,subMeshExtents.cpy,subMeshExtents.cpz,subMeshExtents.rad);

	      bool lastTriangles = false;
	      int   maxTrianglesOnALine = 15;
	      int   triangleCount = 0;
	      for ( int f = 0; f < (int)mesh.faces.size(); f++ )
	      {
		CFace &face = mesh.faces[f];
		if ( f == 0 )
		{
		  lastTriangles = face.verts.size() == 3;

		  if (lastTriangles)
		    section += "tris";
		  else
		    section += "polygon";

		  for ( int v = 0; v < (int)face.verts.size(); v++ )
		    section += TextUtils::format(" %d",computeCorner(mesh,face,v,verts,norms,uvs,corners));

		  if (!lastTriangles)
		    section += "\n";
		  else
		    triangleCount++;
		}
		else
		{
		  bool trianglesThisTime = face.verts.size() == 3;
		  if ( (lastTriangles && !trianglesThisTime) || (trianglesThisTime && triangleCount > maxTrianglesOnALine) )
		    section += "\n";

		  if ( !trianglesThisTime )
		    section += "polygon";
		  else if ( (!lastTriangles && trianglesThisTime) || (trianglesThisTime && triangleCount > maxTrianglesOnALine) )
		    section += "tris";

		  for ( int v = 0; v < (int)face.verts.size(); v++ )
		    section += TextUtils::format(" %d",computeCorner(mesh,face,v,verts,norms,uvs,corners));

		  if (!trianglesThisTime)
		  {
		    section += "\n";
		    triangleCount = 0;
		  }
		  else
		  {
		    if ( triangleCount > maxTrianglesOnALine)
		      triangleCount = 0;
		    else
		      triangleCount++;
		  }
      
		  lastTriangles = trianglesThisTime;
		}
	      }
	      section += "\nend\n"; // this ends the material!
	    }
	  }
	  section += TextUtils::format("end #lod %d\n",l );

	  lodSections.push_back(section);
	}
      }
    
      // build up the corners
      
      for ( int c = 0; c < (int)corners.size(); c++ )
	cornerSection += TextUtils::format("corner %d %d %d\n",(int)corners[c].x, (int)corners[c].y, (int)corners[c].z);
    
      drawInfoSection += "\n#corners\n" + cornerSection;
      if (lodSections.size())
	drawInfoSection += "\n#lods\n";
      for ( int l = 0; l < (int)lodSections.size(); l++ )
      {
	drawInfoSection += "\n";
	drawInfoSection += lodSections[l];
	drawInfoSection += "\n";
      }
      drawInfoSection += "end #drawinfo\n";
    }
  }

  // generate the indexes section
  // first verts
  inxexesSection += "#indexes\n";
  for ( int v = 0; v < (int)verts.size(); v++ )
    inxexesSection += TextUtils::format("vertex %f %f %f\n",verts[v].x*globalScale+globalShift[0],verts[v].y*globalScale+globalShift[1],verts[v].z*globalScale+globalShift[2]);
  for ( int n = 0; n < (int)norms.size(); n++ )
    inxexesSection += TextUtils::format("normal %f %f %f\n",norms[n].x,norms[n].y,norms[n].z);
  for ( int u = 0; u < (int)uvs.size(); u++ )
    inxexesSection += TextUtils::format("texcoord %f %f\n",uvs[u].u,uvs[u].v);

  if ( boundingGeoSection.size() )
    materialsSection += "\nmaterial\nname " + invisibleMatName + "diffuse 0 0 0 0\nnoradar\nnoshadow\nnoculling\nnosorting\nend\n";

  tmMaterialMap::iterator matItr = drawInfoMeshes.staticMesh.materials.begin();
  while (matItr != drawInfoMeshes.staticMesh.materials.end())
  {
    materialsSection += writeMaterial(matItr->second,matItr->first) + "\n";
    matItr++;
  }
  
  for ( int l = 0; l < (int)drawInfoMeshes.lodMeshes.size(); l++ )
  {
    matItr = drawInfoMeshes.lodMeshes[l].materials.begin();
    while (matItr != drawInfoMeshes.lodMeshes[l].materials.end())
    {
      materialsSection += writeMaterial(matItr->second,matItr->first) + "\n";
      matItr++;
    }
  }

  FILE *fp = fopen (file.c_str(),"wt");
  if (!fp)
    return;

  if (useMaterials)
    fprintf(fp,"%s",materialsSection.c_str());

  if (groupName.size() > 0)
    fprintf (fp, "define %s\n", groupName.c_str());

  fprintf (fp,"mesh\n");

  fprintf (fp,"# vertices: %d\n", (int)verts.size());
  fprintf (fp,"# normals: %d\n", (int)norms.size());
  fprintf (fp,"# texcoords: %d\n", (int)uvs.size());

  if (useSmoothBounce)
    fprintf (fp,"  smoothbounce\n");

  fprintf(fp,"%s",inxexesSection.c_str());
  fprintf(fp,"%s",staticGeoSection.c_str());
  fprintf(fp,"%s",drawInfoSection.c_str());

  fprintf (fp,"end # mesh\n");

  if (groupName.size() > 0) 
    fprintf (fp, "enddef # %s\n", groupName.c_str());

  // do the custom objects.
  for ( unsigned int i = 0; i < drawInfoMeshes.staticMesh.customObjects.size(); i++ )
  {
    fprintf (fp, "%s\n", drawInfoMeshes.staticMesh.customObjects[i].name.c_str());
    for (unsigned int j = 0; j < drawInfoMeshes.staticMesh.customObjects[i].params.size(); j++ )
      fprintf (fp, "  %s\n", drawInfoMeshes.staticMesh.customObjects[i].params[j].c_str());
    fprintf (fp, "end\n\n");
  }

  fclose(fp);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8