/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

// system headers
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
using std::string;

// common headers
#include "TextUtils.h"

#include "model.h"
#include "wavefrontOBJ.h"
#include "Q3BSP.h"

#ifdef _MODEL_TOOL_GFX
#include "graphicApplication.h"
#include "camera.h"
int GFXMain(int argc, char* argv[]);
#endif


//============================================================================//

// globals
const char VersionString[] = "ModelTool v1.8.4.2 (WaveFront OBJ/BZW to BZFlag BZW converter)";

string texdir = "";
string groupName = "";
string floatFormat = "%g";
bool useMaterials    = true;
bool useAmbient      = true;
bool useDiffuse      = true;
bool useSpecular     = true;
bool useShininess    = true;
bool useEmission     = true;
bool useNormals      = true;
bool useTexcoords    = true;
bool useGrouping     = true;
bool useGroundShift  = true;
bool useSmoothBounce = false;
bool flipYZ          = false;
bool reportReindex   = false;
bool supressMats     = false;
float fudgeFactor = 0.0f;
float shineFactor = 1.0f;

size_t triStripLimit = 16;

bool outputBounds = false;
bool outputComments = false;

float maxShineExponent = 128.0f; // OpenGL minimum shininess

float globalScale = 1.0f;
float globalShift[3] = {0,0,0};
std::vector<string> bspMaterialSkips; // materials to skip in a bsp map


struct DrawInfoConfig {
  string staticFile;
  string boundingFile;
  std::vector<string> lodFiles;
  std::vector<float> lodPixelDistances;
  std::vector<string>  animComands;
};


struct DrawInfoMeshes {
  CModel staticMesh;
  CModel boundingMesh;
  std::vector<CModel> lodMeshes;
  std::vector<float>  lodPixelDistances;
  std::vector<string>  animComands;

  bool valid()
  {
    if (staticMesh.meshes.size())
      return true;
    if (boundingMesh.meshes.size())
      return true;
    if (lodMeshes.size())
      return true;

    return false;
  }
};


//============================================================================//

static string ftoa(float v)
{
  char buf[64];
  snprintf(buf, sizeof(buf), floatFormat.c_str(), v);
  return buf;
}


static string ftoa(const float* va, int count)
{
  char buf[64];
  string s;
  for (int i = 0; i < count; i++) {
    if (i != 0) {
      s += ' ';
    }
    snprintf(buf, sizeof(buf), floatFormat.c_str(), va[i]);
    s += buf;
  }
  return s;
}


//============================================================================//

void progressLog ( int value, int total, const string &text )
{
  printf("Working %d/%d(%f): %s\n",value,total,(float)value/(float)total,text.c_str());
}

void progressLog ( const string &text )
{
  printf("Working: %s\n",text.c_str());
}

void progressLog ( const  char* text )
{
  printf("Working: %s\n",text);
}


void parseDrawInfoConfig ( DrawInfoConfig &config, string file );
void buildDrawInfoMeshesFromConfig ( DrawInfoConfig &config, DrawInfoMeshes &drawInfoMeshes );
void writeDrawInfoBZW ( DrawInfoMeshes &drawInfoMeshes, string file );

string writeMaterial ( CMaterial &material, const string &name )
{
  string out;

  if (supressMats)
    return out;
  out += TextUtils::format("material\n  name %s\n",name.c_str());
  if ( material.texture.size())
  {
    string texName = texdir + material.texture;
    // change the extension to png
    const char *p = strrchr(texName.c_str(), '.');
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

  if (useAmbient)   { out += "  ambient "   + ftoa(material.ambient, 4) + "\n";  }
  if (useDiffuse)   { out += "  diffuse "   + ftoa(material.diffuse, 4) + "\n";  }
  if (useEmission)  { out += "  emission "  + ftoa(material.emission, 4) + "\n"; }
  if (useSpecular)  { out += "  specular "  + ftoa(material.specular, 4) + "\n"; }
  if (useShininess) { out += "  shininess " + ftoa(material.shine) + "\n";       }

  out += "end\n\n";

  return out;
}


static void writeBZW  ( CModel &model, string file )
{
  if (model.meshes.size() < 1 )
    return;

  FILE *fp = fopen (file.c_str(), "wt");
  if (!fp)
    return;

  if (useMaterials)
  {
    tmMaterialMap::iterator materialItr = model.materials.begin();
    while ( materialItr != model.materials.end() )
    {
      fprintf(fp,"%s",writeMaterial(materialItr->second,materialItr->first).c_str());
      materialItr++;
    }
    fprintf(fp,"\n");
  }

  if (groupName.size() > 0)
    fprintf(fp, "define %s\n", groupName.c_str());

  tvMeshList::iterator	meshItr = model.meshes.begin();
  for (meshItr = model.meshes.begin(); meshItr != model.meshes.end(); ++meshItr)
  {
    CMesh& mesh = *meshItr;

    mesh.reindex();

    fprintf(fp,"mesh # %s\n", mesh.name.c_str());

    if (outputComments)
    {
      fprintf(fp,"# vertices:  %d\n",   (int)mesh.verts.size());
      fprintf(fp,"# normals:   %d\n",   (int)mesh.normals.size());
      fprintf(fp,"# texcoords: %d\n",   (int)mesh.texCoords.size());
      fprintf(fp,"# faces:     %d\n\n", (int)mesh.faces.size());
    }

    if (useSmoothBounce)
      fprintf(fp,"  smoothbounce\n");

    for (int v = 0; v < (int)mesh.verts.size();v++)
    {
      const CVector3* vert = &mesh.verts[v];
      const string msg = "  vertex " + ftoa(vert->x) + " "
                                     + ftoa(vert->y) + " "
                                     + ftoa(vert->z);
      fprintf(fp, msg.c_str());
      if (outputComments)
	fprintf(fp,"\t# %d",v);
      fprintf(fp,"\n");
    }

    for (int n = 0; n < (int)mesh.normals.size();n++)
    {
      CVector3* norm = &mesh.normals[n];

      // normalise all normals before writing them
      const float dist = sqrtf((norm->x * norm->x) +
                               (norm->y * norm->y) +
                               (norm->z * norm->z));
      if (dist == 0.0f) {
        const string nums = ftoa(norm->x) + " "
                          + ftoa(norm->y) + " "
                          + ftoa(norm->z);
        fprintf(stderr, "BAD NORMAL: %s\n", nums.c_str());
        fprintf(fp,"  normal 0 0 1\t# %d  BAD NORMAL %s", n, nums.c_str());
      } else {
        const float scale = (1.0f / dist);
        const string msg = "  normal " + ftoa(norm->x * scale) + " "
                                       + ftoa(norm->y * scale) + " "
                                       + ftoa(norm->z * scale);
        fprintf(fp, msg.c_str());
        if (outputComments)
        {
          fprintf(fp, "\t# %d", n);
        }
      }
      fprintf(fp,"\n");
    }

    for (int t = 0; t < (int)mesh.texCoords.size(); t++)
    {
      const CVector2* coord = &mesh.texCoords[t];
      const string msg = "  texcoord " + ftoa(coord->u) + " " + ftoa(coord->v);
      fprintf(fp, msg.c_str());
      if (outputComments)
      {
	fprintf(fp, "\t# %d", t);
      }
      fprintf(fp,"\n");
    }

    tvFaceList::iterator	faceItr = mesh.faces.begin();
    for (int f = 0; f < (int)mesh.faces.size(); f++)
    {
      CFace	&face = mesh.faces[f];

      fprintf(fp,"  face");
      if (outputComments)
	fprintf(fp,"\t# %d",f);
      fprintf(fp,"\n");

      tvIndexList::iterator	indexItr = face.verts.begin();
      fprintf(fp,"    vertices");
      while ( indexItr != face.verts.end() )
	fprintf(fp," %d",*indexItr++);

      fprintf(fp,"\n");

      if (useNormals && (face.normals.size() > 0))
      {
	indexItr = face.normals.begin();
	fprintf(fp,"    normals");
	while ( indexItr != face.normals.end() )
	fprintf(fp," %d",*indexItr++);
	fprintf(fp,"\n");
      }

      if (useTexcoords && (face.texCoords.size() > 0))
      {
	indexItr = face.texCoords.begin();
	fprintf(fp,"    texcoords");
	while ( indexItr != face.texCoords.end() )
	  fprintf(fp," %d",*indexItr++);

	fprintf(fp,"\n");
      }

      if (useMaterials && (face.material.size() > 0))
	fprintf(fp, "    matref %s\n", face.material.c_str());

      fprintf(fp,"  endface\n");
    }
    fprintf(fp,"end\n\n");
  }

  if (groupName.size() > 0)
    fprintf(fp, "enddef # %s\n", groupName.c_str());

  // do the custom objects.
  for (unsigned int i = 0; i < model.customObjects.size(); i++ )
  {
    fprintf(fp, "%s\n", model.customObjects[i].name.c_str());
    for (unsigned int j = 0; j < model.customObjects[i].params.size(); j++ )
      fprintf(fp, "  %s\n", model.customObjects[i].params[j].c_str());
    fprintf(fp, "end\n\n");
  }

  fclose(fp);
}

static int  dumpUsage ( char *exeName, const char* reason )
{
  printf("\n%s\n\n", VersionString);
  printf("error: %s\n\n",reason);
  printf("usage: %s <input_file_name> [options]\n\n", exeName);
  printf("       -g <name>         : use group definition\n");
  printf("\n");
  printf("       -tx <dir>         : set texture prefix\n");
  printf("\n");
  printf("       -sm               : use the smoothbounce property\n");
  printf("\n");
  printf("       -yz               : flip y and z coordinates\n");
  printf("       -nogroup          : disable object grouping\n");
  printf("       -noground         : disable model ground clamp\n");
  printf("       -n                : disable normals\n");
  printf("       -t                : disable texture coordinates\n");
  printf("       -m                : disable materials\n");
  printf("       -a                : disable ambient coloring\n");
  printf("       -d                : disable diffuse coloring\n");
  printf("       -e                : disable emission coloring\n");
  printf("       -s                : disable specular coloring\n");
  printf("       -sh               : disable shininess\n");
  printf("       -sf <val>         : shine multiplier\n");
  printf("\n");
  printf("       -gx <val>         : scale the model by this factor\n");
  printf("       -gsx <val>        : shift the map by this value in X\n");
  printf("       -gsy <val>        : shift the map by this value in Y\n");
  printf("       -gsz <val>        : shift the map by this value in Z\n");
  printf("\n");
  printf("       -bspskip <val>    : skip faces with this material when importing a bsp\n");
  printf("\n");
  printf("       -bounds           : compute the bounds and sphere for draw info meshes and write them to the map\n");
  printf("\n");
  printf("       -comments         : add comments to the resulting bzw file (will make it a lot larger)\n");
  printf("\n");
  printf("       -striplimit <val> : the longest triangle strip to use for LODs )\n\n");
 return 1;
}

bool setupArgs (int argc, char* argv[], string &input, string &extenstion, string &output )
{
  // make sure we have all the right stuff
  if ( argc < 2)
  {
    dumpUsage(argv[0],"No input file specified");
    return false;
  }

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
    output = argv[1] + string(".bzw");
  }

  for (int i = 2; i < argc; i++)
  {
    string command = argv[i];
    command = TextUtils::tolower(command);

    if (command == "-yz")
    {
      flipYZ = true;
    }
    else if (command == "-noground")
    {
      useGroundShift = false;
    }
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
    else if (command == "-bounds")
      outputBounds = true;
    else if (command == "-nomats")
      supressMats = true;
    else if (command == "-comments")
      outputComments = true;
    else if (command == "-rr")
      reportReindex = true;
    else if (command == "-nogroup")
      useGrouping = false;
    else if (command == "-ff")
    {
      if ((i + 1) < argc)
      {
	i++;
	fudgeFactor = (float)atof(argv[i]);
      }
      else
	printf ("missing -ff argument\n");
    }
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
      else if (command == "-striplimit")
      {
	if ((i + 1) < argc)
	{
	  i++;
	  triStripLimit = (int)atoi(argv[i]);
	}
	else
	  printf ("missing -striplimit argument\n");
      }
      else if (command == "-bspskip")
      {
	if ((i + 1) < argc)
	{
	  i++;
	  bspMaterialSkips.push_back(string(argv[i]));
	}
	else
	  printf ("missing -bspskip argument\n");
      }
  }
  return true;
}

int main(int argc, char* argv[])
{
#ifdef  _MODEL_TOOL_GFX
  return GFXMain(argc,argv);
#endif

  string input;
  string extenstion = "OBJ";
  string output;

  if (!setupArgs(argc, argv, input,extenstion,output))
    return 1;

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

    if ( !meshes.valid())
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

  if (useGroundShift) {
    model.pushAboveAxis(eZAxis);
  }

  if (model.meshes.size() > 0)
  {
    writeBZW(model,output);
    printf("%s file %s converted to BZW as %s\n", extenstion.c_str(),input.c_str(),output.c_str());
  }
  else
    printf("no valid meshes written from %s\n", input.c_str());

  return 0;
}

static int getNewIndex ( const CVector3 &vert, tvVec3List &vertList )
{
  tvVec3List::iterator itr = vertList.begin();

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

static int getNewIndex ( const CVector2 &vert, tvVec2List &vertList )
{
  tvVec2List::iterator itr = vertList.begin();

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


//============================================================================//

void CMesh::getUsedIndices(std::set<int>& vertIndexSet,
                           std::set<int>& normIndexSet,
                           std::set<int>& txcdIndexSet)
{
  for (size_t f = 0; f < faces.size(); f++) {
    const CFace& face = faces[f];
    for (size_t i = 0; i < face.verts.size(); i++) {
      vertIndexSet.insert(face.verts[i]);
    }
    for (size_t i = 0; i < face.normals.size(); i++) {
      normIndexSet.insert(face.normals[i]);
    }
    for (size_t i = 0; i < face.texCoords.size(); i++) {
      txcdIndexSet.insert(face.texCoords[i]);
    }
  }
}


void CMesh::mapVec2List(const tsIndexSet& indices,
                        const tvVec2List& input,
                        tvVec2List& output,
                        std::map<int, int>& indexMap)
{
  output.clear();
  indexMap.clear();

  std::map<CVector2, int> dataMap;
  std::map<CVector2, int>::const_iterator dataIt;

  tsIndexSet::const_iterator it;
  for (it = indices.begin(); it != indices.end(); ++it) {
    const int index = *it;
    const CVector2& data = input[index];
    dataIt = dataMap.find(data);
    if (dataIt != dataMap.end()) {
      indexMap[index] = dataIt->second;
    }
    else {
      indexMap[index] = output.size();
      dataMap[data] = output.size();
      output.push_back(data);
    }
  }
}


void CMesh::mapVec3List(const tsIndexSet& indices,
                        const tvVec3List& input,
                        tvVec3List& output,
                        std::map<int, int>& indexMap)
{
  output.clear();
  indexMap.clear();

  std::map<CVector3, int> dataMap;
  std::map<CVector3, int>::const_iterator dataIt;

  tsIndexSet::const_iterator it;
  for (it = indices.begin(); it != indices.end(); ++it) {
    const int index = *it;
    const CVector3& data = input[index];
    dataIt = dataMap.find(data);
    if (dataIt != dataMap.end()) {
      indexMap[index] = dataIt->second;
    }
    else {
      indexMap[index] = output.size();
      dataMap[data] = output.size();
      output.push_back(data);
    }
  }
}


void CMesh::reindex()
{
  if (!needReindex) {
    return;
  }
  assignData(verts, normals, texCoords);
}


bool CMesh::assignData(const tvVec3List& vertInData,
                       const tvVec3List& normInData,
                       const tvVec2List& txcdInData)
{
  if (!valid() || vertInData.empty()) {
    return false;
  }

  tsIndexSet vertIndexSet;
  tsIndexSet normIndexSet;
  tsIndexSet txcdIndexSet;
  getUsedIndices(vertIndexSet, normIndexSet, txcdIndexSet);
  
  tvVec3List vertOutData;
  tvVec3List normOutData;
  tvVec2List txcdOutData;
  std::map<int, int> vertRemap;
  std::map<int, int> normRemap;
  std::map<int, int> txcdRemap;
  CMesh::mapVec3List(vertIndexSet, vertInData, vertOutData, vertRemap);
  CMesh::mapVec3List(normIndexSet, normInData, normOutData, normRemap);
  CMesh::mapVec2List(txcdIndexSet, txcdInData, txcdOutData, txcdRemap);

  for (size_t f = 0; f < faces.size(); f++) {
    CFace& face = faces[f];
    CFace  tmpFace;

    tmpFace.material = face.material;

    tmpFace.verts.resize(face.verts.size());
    for (size_t i = 0; i < face.verts.size(); i++) {
      tmpFace.verts[i] = vertRemap[face.verts[i]];
    }

    tmpFace.normals.resize(face.normals.size());
    for (size_t i = 0; i < face.normals.size(); i++) {
      tmpFace.normals[i] = normRemap[face.normals[i]];
    }

    tmpFace.texCoords.resize(face.texCoords.size());
    for (size_t i = 0; i < face.texCoords.size(); i++) {
      tmpFace.texCoords[i] = txcdRemap[face.texCoords[i]];
    }

    faces[f] = tmpFace;
  }

  verts = vertOutData;
  normals = normOutData;
  texCoords = txcdOutData;

  needReindex = false;

  if (reportReindex) {
    printf("reindexing \"%s\"  (%i faces):\n", name.c_str(), (int)faces.size());
    printf("  oldVerts = %i, newVerts = %i\n", (int)vertInData.size(), (int)vertOutData.size());
    printf("  oldNorms = %i, newNorms = %i\n", (int)normInData.size(), (int)normOutData.size());
    printf("  oldTxcds = %i, newTxcds = %i\n", (int)txcdInData.size(), (int)txcdOutData.size());
  }

  return true;
}


//============================================================================//

void parseDrawInfoConfig ( DrawInfoConfig &config, string file )
{
  string text;
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

  text = TextUtils::replace_all(text,string("\r"),string(""));
  std::vector<string> lines = TextUtils::tokenize(text,string("\n"));
  if (!lines.size())
    return;

  int numLines = (int)lines.size();
  for (int i = 0; i < numLines; i++ )
  {
    string &line = lines[i];
    if (!line.size())
      continue;

    std::vector<string> chunks = TextUtils::tokenize(line,string(" "),0,true);
    if (!chunks.size())
      continue;

    string key = TextUtils::tolower(chunks[0]);
    if (key == "static")
      config.staticFile = chunks[1];
    else if (key == "bounding")
      config.boundingFile = chunks[1];
    else if (key == "anim" )
      config.animComands.push_back(chunks[1]);
    else if (key == "lod")
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

  for (int i = 0; i < (int)config.lodFiles.size(); i++ )
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

  for (int m = 0; m < (int)model.meshes.size(); m++ )
  {
    CMesh &subMesh = model.meshes[m];
    bool extentsSet = false;
    for (int f = 0; f < (int)subMesh.faces.size();f++)
    {
      extentsSet = true;
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

      for (int v = 0; v < (int)face.verts.size(); v++ )
      {
	CVector3 &vert = subMesh.verts[face.verts[v]];
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

    for (int s = 0; s < (int)subMesh.strips.size();s++)
    {
      didOne = true;
      CTriStrip &strip = subMesh.strips[s];
      if (!extentsSet && s == 0 && m ==0)
      {
	extentsSet = true;
	extents.minx = subMesh.verts[strip.verts[0]].x;
	extents.miny = subMesh.verts[strip.verts[0]].y;
	extents.minz = subMesh.verts[strip.verts[0]].z;

	extents.maxx = extents.minx;
	extents.maxy = extents.miny;
	extents.maxz = extents.minz;
      }

      for (int v = 0; v < (int)strip.verts.size(); v++ )
      {
	CVector3 &vert = subMesh.verts[strip.verts[v]];
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

    for (int f = 0; f < (int)subMesh.fans.size();f++)
    {
      didOne = true;
      CTriFan &fan = subMesh.fans[f];
      if (!extentsSet && f == 0 && m ==0)
      {
	extentsSet = true;
	extents.minx = subMesh.verts[fan.verts[0]].x;
	extents.miny = subMesh.verts[fan.verts[0]].y;
	extents.minz = subMesh.verts[fan.verts[0]].z;

	extents.maxx = extents.minx;
	extents.maxy = extents.miny;
	extents.maxz = extents.minz;
      }

      for (int v = 0; v < (int)fan.verts.size(); v++ )
      {
	CVector3 &vert = subMesh.verts[fan.verts[v]];
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

bool computeExtents ( CModel &/*model*/, CMesh &subMesh, MeshExtents &extents )
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

    for (int v = 0; v < (int)face.verts.size(); v++ )
    {
      CVector3 &vert = subMesh.verts[face.verts[v]];
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


int computeCorner ( const CMesh &mesh, const CFace &face, int index, tvVec3List &v, tvVec3List &n, tvVec2List &u, tvVec3List &c )
{
  int vertIndex = getNewIndex(mesh.verts[face.verts[index]],v);
  int normalIndex = getNewIndex(mesh.normals[face.normals[index]],n);
  int uvIndex;
  if (!face.texCoords.size())
    uvIndex = getNewIndex(CVector2(0,0),u);
  else
    uvIndex = getNewIndex(mesh.texCoords[face.texCoords[index]],u);

  CVector3 vert;
  vert.x = (float)vertIndex;
  vert.y = (float)normalIndex;
  vert.z = (float)uvIndex;

  return getNewIndex(vert,c);
}

int computeCorner ( const CMesh &mesh, const CTriStrip &strip, int index, tvVec3List &v, tvVec3List &n, tvVec2List &u, tvVec3List &c )
{
  // it's highly likely that the strip will use
  // the vert index for both the normal and UV, so try those
  // if the strip has no indexes. if not use a dummy one

  int vertIndex = getNewIndex(mesh.verts[strip.verts[index]],v);
  int normalIndex;
  if (!strip.normals.size())
  {
    if (mesh.normals.size() && (strip.verts[index] < (int)mesh.normals.size()) )
      normalIndex = getNewIndex(mesh.normals[strip.verts[index]],n);
    else
      normalIndex = getNewIndex(CVector3(0,0,1),n);
  }
  else
    normalIndex = getNewIndex(mesh.normals[strip.normals[index]],n);

  int uvIndex;
  if (!strip.texCoords.size())
  {
    if (mesh.normals.size() && (strip.verts[index] < (int)mesh.texCoords.size()) )
      uvIndex = getNewIndex(mesh.texCoords[strip.verts[index]],u);
    else
      uvIndex = getNewIndex(CVector2(0,0),u);
  }
  else
    uvIndex = getNewIndex(mesh.texCoords[strip.texCoords[index]],u);

  CVector3 vert;
  vert.x = (float)vertIndex;
  vert.y = (float)normalIndex;
  vert.z = (float)uvIndex;

  return getNewIndex(vert,c);
}


int computeCorner ( const CMesh &mesh, const CTriFan &fan, int index, tvVec3List &v, tvVec3List &n, tvVec2List &u, tvVec3List &c )
{
  int vertIndex = getNewIndex(mesh.verts[fan.verts[index]],v);
  int normalIndex = getNewIndex(mesh.normals[fan.normals[index]],n);
  int uvIndex;
  if (!fan.texCoords.size())
    uvIndex = getNewIndex(CVector2(0,0),u);
  else
    uvIndex = getNewIndex(mesh.texCoords[fan.texCoords[index]],u);

  CVector3 vert;
  vert.x = (float)vertIndex;
  vert.y = (float)normalIndex;
  vert.z = (float)uvIndex;

  return getNewIndex(vert,c);
}

void writeDrawInfoBZW ( DrawInfoMeshes &drawInfoMeshes, string file )
{
  if (!drawInfoMeshes.valid())
    return;

  // the idea here is to go and output each of the mesh sections into
  // seperate buffers, bulding up the actual used vert and index lists.
  // then dump out those lists to a buffer, and composite the entire
  // thing into one bzw.
  // This way we can do things in any order and not worry about
  // duplicating indexes.
  string materialsSection;
  string inxexesSection;
  string staticGeoSection;
  string boundingGeoSection;
  string drawInfoSection;

  string invisibleMatName = "bounding.invisible";

  // the 3 major lists
  tvVec3List  verts;
  tvVec3List  norms;
  tvVec2List  uvs;

  // this is cheap, each corner is unique instance of a vert, normal, and uv coordinate.
  // even tho they are ints we can store them like a vert, and just cast them to ints.
  // to leverage the existing functions for sorting indexes.
  tvVec3List  corners;

  progressLog("starting drawInfo Mesh");

  // build the static geo into it's buffer
  if ( drawInfoMeshes.staticMesh.meshes.size())
  {
    CModel &staticModel = drawInfoMeshes.staticMesh;
    for (int m = 0; m < (int)staticModel.meshes.size(); m++ )
    {
      CMesh &subMesh = staticModel.meshes[m];
      progressLog(TextUtils::format("static model sub mesh%d",m));

      for (int f = 0; f < (int)subMesh.faces.size();f++)
      {
	CFace &face = subMesh.faces[f];

	staticGeoSection += "face";
	if (outputComments)
	  staticGeoSection += TextUtils::format("\t#%d",f);
	staticGeoSection += "\n";

	string vert = "vertices";
	string norm = "normals";
	string uv = "texcoords";

	for (int v = 0; v < (int)face.verts.size(); v++ )
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
	staticGeoSection += "endface\n\n";
      }
    }
  }

  // write the bounding geo to the bufer, with the invisible mesh
  if ( drawInfoMeshes.boundingMesh.meshes.size() )
  {
    CModel &boundingMesh = drawInfoMeshes.boundingMesh;
    for (int m = 0; m < (int)boundingMesh.meshes.size(); m++ )
    {
      CMesh &subMesh = boundingMesh.meshes[m];
      progressLog(TextUtils::format("bounding model sub mesh%d",m));

      for (int f = 0; f < (int)subMesh.faces.size();f++)
      {
	CFace &face = subMesh.faces[f];

	boundingGeoSection += "face\n";
	if (outputComments)
	  boundingGeoSection += TextUtils::format("\t#%d",f);
	staticGeoSection += "\n";

	string vert = "vertices";
	string norm = "normals";

	for (int v = 0; v < (int)face.verts.size(); v++ )
	{
	  vert += TextUtils::format(" %d",getNewIndex(subMesh.verts[face.verts[v]],verts));
	  norm += TextUtils::format(" %d",getNewIndex(subMesh.normals[face.normals[v]],norms));
	}
	boundingGeoSection += vert + "\n" + norm + "\n";

	boundingGeoSection += "matref " + invisibleMatName + "\n";
	boundingGeoSection += "endface\n\n";
      }
    }
  }

  // now we are doing the real draw info part
  // the first model must be valid, since we use that as lod 0, and for bounds calcs
 if (drawInfoMeshes.lodMeshes.size() && drawInfoMeshes.lodMeshes[0].meshes.size() )
  {
    // use the bounds of the first LOD for the bounds of the draw info

    string cornerSection;
    std::vector<string> lodSections;

    MeshExtents	lod0Extents;
    if (computeExtents(drawInfoMeshes.lodMeshes[0],lod0Extents))
    {
      drawInfoSection += "drawInfo\n";
      if (outputBounds)
      {
	drawInfoSection += TextUtils::format("extents %f %f %f %f %f %f\n",lod0Extents.minx,lod0Extents.miny,lod0Extents.minz,lod0Extents.maxx,lod0Extents.maxy,lod0Extents.maxz);
	drawInfoSection += TextUtils::format("sphere %f %f %f %f\n",lod0Extents.cpx,lod0Extents.cpy,lod0Extents.cpz,lod0Extents.rad);
      }

      for (int a = 0; a < (int)drawInfoMeshes.animComands.size(); a++ )
	drawInfoSection += drawInfoMeshes.animComands[a] + "\n";

      // compute the LOD sections
      for (int l = 0; l < (int)drawInfoMeshes.lodMeshes.size(); l++ )
      {
	CModel &lodModel = drawInfoMeshes.lodMeshes[l];
	string section;
	if ( lodModel.meshes.size() )
	{
	  section += TextUtils::format("lod #%d\n",l );
	  if (l == 0)
	    section += "lengthPerPixel 0\n";
	  else
	    section += TextUtils::format("lengthPerPixel %f\n",drawInfoMeshes.lodPixelDistances[l]);

	  for (int m = 0; m < (int)lodModel.meshes.size(); m++ )
	  {
	    CMesh &mesh = lodModel.meshes[m];
	    progressLog(TextUtils::format("LOD %d model sub mesh%d",l,m));

	    if ( mesh.faces.size())
	    {
	      // we always use the first face's material for the lod
	      section += "matref ";
	      if (mesh.faces[0].material.size())
		section += mesh.faces[0].material;
	      else
		section += "-1";
	      section += "\n";

	      section += "dlist\n";

	      if (outputBounds)
	      {
		MeshExtents subMeshExtents;

		computeExtents(lodModel,mesh,subMeshExtents);
		section += TextUtils::format("sphere %f %f %f %f\n",subMeshExtents.cpx,subMeshExtents.cpy,subMeshExtents.cpz,subMeshExtents.rad);
	      }

	      bool lastTriangles = false;
	      int   maxTrianglesOnALine = 15;
	      int   triangleCount = 0;
	      for (int f = 0; f < (int)mesh.faces.size(); f++ )
	      {
		CFace &face = mesh.faces[f];
		if ( f == 0 )
		{
		  lastTriangles = face.verts.size() == 3;

		  if (lastTriangles)
		    section += "tris";
		  else
		    section += "polygon";

		  for (int v = 0; v < (int)face.verts.size(); v++ )
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

		  for (int v = 0; v < (int)face.verts.size(); v++ )
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
	      section += "\nend #matref\n"; // this ends the material!
	    }

	    // now do any tristrips we happen to have
	    if ( mesh.strips.size())
	    {
	      // we always use the first strips's material for the lod
	      section += "\nmatref ";
	      if (mesh.strips[0].material.size())
		section += mesh.strips[0].material;
	      else
		section += "-1";
	      section += "\n";

	      section += "dlist\n";

	      if (outputBounds)
	      {
		MeshExtents subMeshExtents;

		computeExtents(lodModel,mesh,subMeshExtents);
		section += TextUtils::format("sphere %f %f %f %f\n",subMeshExtents.cpx,subMeshExtents.cpy,subMeshExtents.cpz,subMeshExtents.rad);
	      }

	      for (int s = 0; s < (int)mesh.strips.size(); s++ )
	      {
		CTriStrip &strip = mesh.strips[s];

		if ( strip.verts.size() <= triStripLimit)
		{
		  section += "tristrip";

		  for (int v = 0; v < (int)strip.verts.size(); v++ )
		    section += TextUtils::format(" %d",computeCorner(mesh,strip,v,verts,norms,uvs,corners));
		  section += "\n";
		}
		else
		{
		  std::vector<tvIndexList>  stripChunks;
		  size_t pos = 0;
		  size_t listSise = strip.verts.size();
		  while ( pos < strip.verts.size() )
		  {
		    if ( pos + triStripLimit < listSise-3) // see if there is at least a full iteration, leaving 3 verts at the end ( min strip size)
		    {
		      section += "tristrip";
		      for (size_t v = pos; v < pos + triStripLimit; v++ )
			section += TextUtils::format(" %d",computeCorner(mesh,strip,(int)v,verts,norms,uvs,corners));
		      section += "\n";
		      pos += triStripLimit;
		    }
		    else if ( listSise - pos <= triStripLimit) // there are less then a limit left, just dump them
		    {
		      section += "tristrip";
		      for (size_t v = pos; v < listSise; v++ )
			section += TextUtils::format(" %d",computeCorner(mesh,strip,(int)v,verts,norms,uvs,corners));
		      section += "\n";
		      pos = listSise;
		    }
		    else // there is more then the limit left but there weould be less then 3 left
		    {
		      section += "tristrip";
		      for (size_t v = pos; v < listSise-3; v++ )
			section += TextUtils::format(" %d",computeCorner(mesh,strip,(int)v,verts,norms,uvs,corners));
		      section += "\n";
		      pos = listSise-3;

		      section += "tristrip";
		      for (size_t v = pos; v < listSise; v++ )
			section += TextUtils::format(" %d",computeCorner(mesh,strip,(int)v,verts,norms,uvs,corners));
		      section += "\n";
		      pos = listSise;
		    }
		  }
		}

	      }
	      section += "\nend #matref\n"; // this ends the material!

	    }

	    // now do any fans we happen to have
	    if ( mesh.fans.size())
	    {
	      // we always use the first strips's material for the lod
	      section += "\nmatref ";
	      if (mesh.fans[0].material.size())
		section += mesh.fans[0].material;
	      else
		section += "-1";
	      section += "\n";

	      section += "dlist\n";

	      if (outputBounds)
	      {
		MeshExtents subMeshExtents;

		computeExtents(lodModel,mesh,subMeshExtents);
		section += TextUtils::format("sphere %f %f %f %f\n",subMeshExtents.cpx,subMeshExtents.cpy,subMeshExtents.cpz,subMeshExtents.rad);
	      }

	      for (int f = 0; f < (int)mesh.fans.size(); f++ )
	      {
		CTriFan &fan = mesh.fans[f];
		section += "trifan";

		for (int v = 0; v < (int)fan.verts.size(); v++ )
		  section += TextUtils::format(" %d",computeCorner(mesh,fan,v,verts,norms,uvs,corners));
		section += "\n";
	      }
	      section += "\nend #matref\n"; // this ends the material!

	    }


	  }
	  section += TextUtils::format("end #lod %d\n",l );

	  lodSections.push_back(section);
	}
      }

      // build up the corners

      for (int c = 0; c < (int)corners.size(); c++ )
      {
	cornerSection += TextUtils::format("corner %d %d %d",(int)corners[c].x, (int)corners[c].y, (int)corners[c].z);

	if (outputComments)
	  cornerSection += TextUtils::format("\t#%d",c);
	cornerSection += "\n";

      }

      drawInfoSection += "\n";
      if (outputComments)
	drawInfoSection += "#corners\n";
      drawInfoSection += cornerSection;

      drawInfoSection += "\n";
      if (lodSections.size() && outputComments)
	drawInfoSection += "#lods\n";

      for (int l = 0; l < (int)lodSections.size(); l++ )
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
  progressLog("Generating indexes");

  if (outputComments)
    inxexesSection += TextUtils::format("#indexes: %d\n",(int)verts.size());

  for (int v = 0; v < (int)verts.size(); v++ )
  {
    inxexesSection += TextUtils::format("vertex %f %f %f",verts[v].x,verts[v].y,verts[v].z);

    if (outputComments)
      inxexesSection += TextUtils::format("\t#%d",v);
    inxexesSection += "\n";
  }

  for (int n = 0; n < (int)norms.size(); n++ )
  {
    inxexesSection += TextUtils::format("normal %f %f %f",norms[n].x,norms[n].y,norms[n].z);

    if (outputComments)
      inxexesSection += TextUtils::format("\t#%d",n);
    inxexesSection += "\n";
  }

  for (int u = 0; u < (int)uvs.size(); u++ )
  {
    inxexesSection += TextUtils::format("texcoord %f %f",uvs[u].u,uvs[u].v);

    if (outputComments)
      inxexesSection += TextUtils::format("\t#%d",u);
    inxexesSection += "\n";
  }

  progressLog("Generating materials");

  if ( boundingGeoSection.size() )
    materialsSection += "\nmaterial\nname " + invisibleMatName + "\ndiffuse 0 0 0 0\nnoradar\nnoshadow\nnoculling\nnosorting\nend\n";

  tmMaterialMap::iterator matItr = drawInfoMeshes.staticMesh.materials.begin();
  while (matItr != drawInfoMeshes.staticMesh.materials.end())
  {
    materialsSection += writeMaterial(matItr->second,matItr->first) + "\n";
    matItr++;
  }

  for (int l = 0; l < (int)drawInfoMeshes.lodMeshes.size(); l++ )
  {
    matItr = drawInfoMeshes.lodMeshes[l].materials.begin();
    while (matItr != drawInfoMeshes.lodMeshes[l].materials.end())
    {
      materialsSection += writeMaterial(matItr->second,matItr->first) + "\n";
      matItr++;
    }
  }

  progressLog("writing BZW file");

  FILE *fp = fopen (file.c_str(),"wt");
  if (!fp)
    return;

  if (useMaterials)
    fprintf(fp,"%s",materialsSection.c_str());

  if (groupName.size() > 0)
    fprintf(fp, "define %s\n", groupName.c_str());

  fprintf(fp,"mesh\n");

  fprintf(fp,"# vertices: %d\n", (int)verts.size());
  fprintf(fp,"# normals: %d\n", (int)norms.size());
  fprintf(fp,"# texcoords: %d\n", (int)uvs.size());

  if (useSmoothBounce)
    fprintf(fp,"  smoothbounce\n");

  fprintf(fp,"%s",inxexesSection.c_str());
  fprintf(fp,"%s",staticGeoSection.c_str());
  fprintf(fp,"%s",boundingGeoSection.c_str());
  fprintf(fp,"%s",drawInfoSection.c_str());

  fprintf(fp,"end # mesh\n");

  if (groupName.size() > 0)
    fprintf(fp, "enddef # %s\n", groupName.c_str());

  // do the custom objects.
  for (unsigned int i = 0; i < drawInfoMeshes.staticMesh.customObjects.size(); i++ )
  {
    
    fprintf(fp, "%s\n", drawInfoMeshes.staticMesh.customObjects[i].name.c_str());
    for (unsigned int j = 0; j < drawInfoMeshes.staticMesh.customObjects[i].params.size(); j++) {
      fprintf(fp, "  %s\n", drawInfoMeshes.staticMesh.customObjects[i].params[j].c_str());
    }
    fprintf(fp, "end #custom\n\n");
  }

  fclose(fp);
}

// all the stuff for a the graphic display
#ifdef _MODEL_TOOL_GFX

void CMesh::draw()
{
  // do the material here
  glColor4f(1,1,1,1);

  for (int f =0; f < (int)faces.size(); f++)
  {
    CFace &face = faces[f];

    glBegin(GL_POLYGON);
    for (int v = 0; v < (int)face.verts.size(); v++ )
    {
      glNormal3f(normals[face.normals[v]].x,normals[face.normals[v]].y,normals[face.normals[v]].z);
      glVertex3f(verts[face.verts[v]].x,verts[face.verts[v]].y,verts[face.verts[v]].z);
    }
    glEnd();
  }
}

void CModel::draw()
{
  for (int i =0; i < (int)meshes.size(); i++)
    meshes[i].draw();
}

class ModelToolApp : public GraphicApplication
{
public:
  void init ( int argc, char* argv[] );
  virtual void setupDisplay();
  virtual bool getStartupInfo ( int &x, int &y, bool &fullScreen, string &title, bool &resizeable );
  virtual bool drawView();
  virtual bool drawOverlay();

  virtual void preFrameUpdate();

  virtual void contextInvalidated ( bool release ){};

  virtual void inputEvent ( int id, float value );
  virtual void mouseMovedEvent ( float x, float y, float z );
  virtual void keyEvent ( int key, bool down, ModiferKeys mods );
  virtual void mouseButtonEvent ( int button, bool down );

protected:
  CModel	model;

  Camera	camera;

  void drawZZeroGrid();

  bool buttonStates[3];
  float dragPos[2];
  float xRot;
  float zRot;
  float zoom;
  float pan[3];
  float gridSpacing;
  float gridExtents;
};

ModelToolApp	app;

int GFXMain(int argc, char* argv[])
{
  app.args.Set(argc,argv);
  app.init (argc,argv);
  app.run();

  return 0;
}

void ModelToolApp::init ( int argc, char* argv[] )
{
  xRot = 45;
  zRot = 0;
  zoom = 15;
  pan[0] = pan[1] = pan[2] = 0.0f;
  buttonStates[0] = buttonStates[1] = buttonStates[2] = false;
  dragPos[0] = dragPos[1] = 0;

  gridSpacing = 1.0f;
  gridExtents = 15.0f;

  string input;
  string extenstion = "OBJ";
  string output;

  if (!setupArgs(argc, argv, input,extenstion,output))
  {
    quit();
    return;
  }

  if (TextUtils::tolower(extenstion) == "obj")
    readOBJ(model,input);
}

bool ModelToolApp::getStartupInfo ( int &x, int &y, bool &fullScreen, string &title, bool &resizeable )
{
  x = 640;
  y = 480;
  fullScreen = false;
  title = "";

  orthoDepth = 1000.0;

  camera.setTargetMode(true);

  return true;
}

void ModelToolApp::setupDisplay()
{
  GraphicApplication::setupDisplay();
}

void ModelToolApp::preFrameUpdate()
{
  // do animation stuff here
}

void ModelToolApp::mouseMovedEvent ( float x, float y, float z )
{
  if(z != 0)
    zoom += z * 0.125f;

  float delta[2];
  delta[0] = x - dragPos[0];
  delta[1] = y - dragPos[1];

  if ( buttonStates[2] )
    camera.rotate(delta[0] * 0.25f,delta[1] * -0.125f);
  else if ( buttonStates[1] )
    camera.pan(delta[0] * -0.0125f,delta[1]* -0.0125f,0);

  dragPos[0] = x;
  dragPos[1] = y;
}

void ModelToolApp::keyEvent ( int key, bool down, ModiferKeys mods )
{
}

void ModelToolApp::mouseButtonEvent ( int button, bool down )
{
  int x,y,z;
  getMouseCursorPos(x,y,z);
  dragPos[0] = (float)x;
  dragPos[1] = (float)y;

  if( button > 3 )
  {
    float scale = 0.125f;

    if ( button == 5 )
      scale *= -1;
    camera.setTargetPullback(scale);
  }
  else
    buttonStates[button-1] = down;
}


bool ModelToolApp::drawView()
{
  glPushMatrix();

  camera.execute();
  /*
  glTranslatef(0,0,-zoom);						// pull back on allong the zoom vector
  glRotatef(xRot, 1.0f, 0.0f, 0.0f);					// pops us to the tilt
  glRotatef(-zRot, 0.0f, 1.0f, 0.0f);					// gets us on our rot
  glTranslatef(-pan[0],-pan[2],pan[1]);					// take us to the pos
  glRotatef(-90, 1.0f, 0.0f, 0.0f);					// gets us into XY
  */
  drawZZeroGrid();

  glEnable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  model.draw();

  glPopMatrix();
  return true;
}

bool ModelToolApp::drawOverlay()
{
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glPushMatrix();
  glTranslatef(25,25,-50.0f);
  glRotatef(-90,1,0,0);
  float rot[3] = {xRot,0,-zRot};
  glAxesWidget(50,rot);
  glPopMatrix();

  return true;
}

void ModelToolApp::inputEvent ( int id, float value )
{
  // do stuff here

}

void ModelToolApp::drawZZeroGrid()
{
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor4f(1,1,1,0.5f);
  glBegin(GL_LINES);
  for (float f = -gridExtents; f <= gridExtents; f += gridSpacing)
  {
    glVertex3f(-gridExtents,f,0);
    glVertex3f(gridExtents,f,0);
    glVertex3f(f,-gridExtents,0);
    glVertex3f(f,gridExtents,0);
  }
  glEnd();
}

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
