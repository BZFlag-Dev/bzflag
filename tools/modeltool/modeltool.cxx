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


// globals/

static const char VersionString[] =
  "ModelTool v1.6  (WaveFront OBJ to BZFlag BZW converter)";

static std::string texdir = "";
static std::string groupName = "";
static bool useMaterials = true;
static bool useAmbient = true;
static bool useDiffuse = true;
static bool useSpecular = true;
static bool useShininess = true;
static bool useEmission = true;
static bool useNormals = true;
static bool useTexcoords = true;
static bool flipYZ = false;
static bool useSmoothBounce = false;
static float shineFactor = 1.0f;

static const float maxShineExponent = 128.0f; // OpenGL minimum shininess

typedef std::vector<int> tvIndexList;

typedef enum
{
	eXAxis,
	eYAxis,
	eZAxis
}teModelAxis;

class CTexCoord
{
public:
	CTexCoord(){u = v = 0;}
	~CTexCoord(){};
	float u,v;

	bool same ( const CTexCoord &c )
	{
		return u == c.u && v ==c.v;
	}
};

typedef std::vector<CTexCoord> tvTexCoordList;

class CVertex
{
public:
	CVertex(){x = y = z = 0;}
	~CVertex(){};
	float x,y,z;

	float get ( teModelAxis axis )
	{
		switch (axis)
		{
		case eXAxis:
			return x;
		case eYAxis:
			return y;
		default:
			return z;
		}
	}

	void translate ( float val, teModelAxis axis )
	{
		switch (axis)
		{
		case eXAxis:
			x += val;
			return;
		case eYAxis:
			y += val;
			return;
		case eZAxis:
			z += val;
		}
	}

	bool same ( const CVertex &v )
	{
		return x == v.x && y == v.y && z == v.z;
	}
};

typedef std::vector<CVertex> tvVertList;

class CFace
{
public:
	CFace(){};
	~CFace(){};

	std::string material;
	tvIndexList verts;
	tvIndexList	normals;
	tvIndexList	texCoords;
};
typedef std::vector<CFace> tvFaceList;


class CMaterial
{
public:
	CMaterial(){clear();}
	~CMaterial(){};

	std::string texture;
	float		ambient[4];
	float		diffuse[4];
	float		specular[4];
	float		emission[4];
	float		shine;

	void clear ( void )
	{
		texture = "";
		ambient[0] = ambient[1] = ambient[2] = 0.2f;
		ambient[3] = 1.0f;
		diffuse[0] = diffuse[1] = diffuse[2] = 1.0f;
		diffuse[3] = 1.0f;
		specular[0] = specular[1] = specular[2] = 0.0f;
		specular[3] = 1.0f;
		emission[0] = emission[1] = emission[2] = 0.0f;
		emission[3] = 1.0f;
		shine = 0.0f;
	}
};

typedef std::map<std::string,CMaterial> tmMaterialMap;


class CMesh
{
public:
	CMesh(){};
	~CMesh(){};

	tvVertList		verts;
	tvVertList		normals;
	tvTexCoordList	texCoords;

	std::string name;
	tvFaceList	faces;

	float getMaxAxisValue ( teModelAxis axis )
	{
		if (!valid())
			return 0.0f;

		float pt = verts[0].get(axis);

		for ( unsigned int i = 0; i < verts.size(); i++ )
			if ( verts[i].get(axis) > pt)
				pt = verts[i].get(axis);

		return pt;
	}

	float getMinAxisValue ( teModelAxis axis )
	{
		if (!valid())
			return 0.0f;

		float pt = verts[0].get(axis);

		for ( unsigned int i = 0; i < verts.size(); i++ )
			if ( verts[i].get(axis) < pt)
				pt = verts[i].get(axis);

		return pt;
	}

	void translate ( float value, teModelAxis axis )
	{
		for ( unsigned int i = 0; i < verts.size(); i++ )
			verts[i].translate(value,axis);
	}

	bool valid ( void )
	{
		return faces.size() != 0;
	}

	void clear ( void )
	{
		faces.clear();
		verts.clear();
		normals.clear();
		texCoords.clear();
		name = "";
	}
	void reindex ( void );
};

typedef std::vector<CMesh> tvMeshList;

class CModel
{
public:
	CModel(){};
	~CModel(){};

	tmMaterialMap	materials;
	tvMeshList		meshes;

	void pushAboveAxis ( teModelAxis axis )
	{
		if (!meshes.size())
			return;

		float minValue = meshes[0].getMinAxisValue(axis);

		for ( unsigned int i = 0; i < meshes.size(); i++ )
			if ( minValue > meshes[i].getMinAxisValue(axis))
				minValue = meshes[i].getMinAxisValue(axis);

		for ( unsigned int i = 0; i < meshes.size(); i++ )
			meshes[i].translate(-minValue,axis);
	}

	void clear ( void )
	{
		meshes.clear();
		materials.clear();
	}
};


static void underscoreBeforeNumbers(std::string& name)
{
  const char first = name[0];
  if ((first >= '0') && (first <= '9')) {
    std::string tmp = "_";
    tmp += name;
	  name = tmp;
	}
	return;
}


static void readMTL ( CModel &model, std::string file )
{
	FILE *fp = fopen(file.c_str(),"rb");
	if (!fp)
	{
		printf ("Could not open MTL file %s\n", file.c_str());
		return;
	}

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pData = (char*)malloc(size+1);
	fread(pData,size,1,fp);
	pData[size] = 0;

	fclose(fp);

	std::string lineTerminator = "\n";

	std::string fileText = pData;
	free(pData);

	fileText = TextUtils::replace_all(fileText,std::string("\r"),std::string(""));

	std::vector<std::string> lines = TextUtils::tokenize(fileText, lineTerminator);

	if ( lines.size() < 2 )
		return;

	CMaterial	material;
	std::string matName;
	material.clear();

	std::vector<std::string>::iterator lineItr = lines.begin();
	while ( lineItr != lines.end() )
	{
		// do a trim here

		std::vector<std::string> lineParts = TextUtils::tokenize(*lineItr,std::string(" "));

		if (lineParts.size() > 1)
		{
			std::string tag = lineParts[0];
			if (tag != "#")
			{
				if (TextUtils::tolower(tag) == "newmtl")
				{
					matName = lineParts[1];
					underscoreBeforeNumbers(matName);
					model.materials[matName] = material;
				}
				if (TextUtils::tolower(tag) == "ka")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].ambient[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].ambient[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].ambient[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (TextUtils::tolower(tag) == "kd")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].diffuse[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].diffuse[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].diffuse[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (TextUtils::tolower(tag) == "d")
				{
					if (lineParts.size() > 1)
					{
						model.materials[matName].ambient[3] = (float)atof(lineParts[1].c_str());
						model.materials[matName].diffuse[3] = (float)atof(lineParts[1].c_str());
					}
				}
				if (TextUtils::tolower(tag) == "ks")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].specular[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].specular[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].specular[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (TextUtils::tolower(tag) == "ns")
				{
					if (lineParts.size() > 1) {
					  float shine = (float)atof(lineParts[1].c_str());
					  // convert MTL "Ns" to OpenGL shininess  [0 - 1000] => [0 - 128]
					  shine = shine / 1000.0f;
					  if (shine < 0.0f) {
					    shine = 0.0f;
						} else if (shine > 1.0f) {
						  shine = 1.0f;
						}
						model.materials[matName].shine = (shine * maxShineExponent * shineFactor);
					}
				}
				if (TextUtils::tolower(tag) == "ke")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].emission[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].emission[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].emission[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (TextUtils::tolower(tag) == "map_kd")
				{
					if (lineParts.size() > 1)
					{
						std::string texture = lineParts[1];
						if (texture[0] == '.')
							texture = texture.c_str()+2;

						model.materials[matName].texture = texture;
					}
				}
			}
		}
		lineItr++;
	}
}

static void readOBJ ( CModel &model, std::string file )
{
	model.clear();

	FILE *fp = fopen(file.c_str(),"r");
	if (!fp) {
	  printf ("Could not open OBJ file %s\n", file.c_str());
		return;
	}

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pData = (char*)malloc(size+1);
	fread(pData,size,1,fp);
	pData[size] = 0;

	fclose(fp);

	// figure out the base path
	std::string baseFilePath;

	char *p = strrchr (file.c_str(),'\\');
	if ( !p )
		p = strrchr (file.c_str(),'/');

	if (p)
	{
		baseFilePath = file;
		baseFilePath.erase(baseFilePath.begin()+(p-file.c_str()+1),baseFilePath.end());
	}

	std::string lineTerminator = "\n";

	std::string fileText = pData;
	free(pData);

	fileText = TextUtils::replace_all(fileText,std::string("\r"),std::string(""));

	std::vector<std::string> lines = TextUtils::tokenize(fileText,lineTerminator);

	if ( lines.size() < 2 )
		return;

	CMesh			mesh;
	tvVertList		temp_verts;
	tvVertList		temp_normals;
	tvTexCoordList	temp_texCoords;
	
	int vCount = 0;
	int nCount = 0;
	int tCount = 0;


	std::string currentMaterial = "";

	std::vector<std::string>::iterator lineItr = lines.begin();
	while ( lineItr != lines.end() )
	{
		// do a trim here

		std::vector<std::string> lineParts = TextUtils::tokenize(*lineItr,std::string(" "));

		if (lineParts.size() > 1)
		{
			if (lineParts[0] != "#")
			{
				if (TextUtils::tolower(lineParts[0]) == "mtllib" && lineParts.size()>1)
					readMTL(model,baseFilePath+lineParts[1]);
				else if (TextUtils::tolower(lineParts[0]) == "v" && lineParts.size()>3)
				{
					CVertex vert;
					vert.x = (float)atof(lineParts[1].c_str());

					if (flipYZ)
					{
						vert.y = -1.0f*(float)atof(lineParts[3].c_str());
						vert.z = (float)atof(lineParts[2].c_str());
					}
					else
					{
						vert.y = (float)atof(lineParts[2].c_str());
						vert.z = (float)atof(lineParts[3].c_str());
					}
					temp_verts.push_back(vert);
					vCount++;
				}
				else if (TextUtils::tolower(lineParts[0]) == "vt" && lineParts.size()>2)
				{
					CTexCoord uv;
					uv.u = (float)atof(lineParts[1].c_str());
					uv.v = (float)atof(lineParts[2].c_str());
					temp_texCoords.push_back(uv);
					tCount++;
				}
				else if (TextUtils::tolower(lineParts[0]) == "vn" && lineParts.size()>3)
				{
					CVertex vert;
					vert.x = (float)atof(lineParts[1].c_str());
					if (flipYZ)
					{
						vert.y = -1.0f* (float)atof(lineParts[3].c_str());
						vert.z = (float)atof(lineParts[2].c_str());
					}
					else
					{
						vert.y = (float)atof(lineParts[2].c_str());
						vert.z = (float)atof(lineParts[3].c_str());
					}
					temp_normals.push_back(vert);
					nCount++;
				}
				else if (TextUtils::tolower(lineParts[0]) == "g" && lineParts.size()>1)
				{
					if ( mesh.valid())
					{
						mesh.verts = temp_verts;
						mesh.normals = temp_normals;
						mesh.texCoords = temp_texCoords;
					//	mesh.reindex();
						model.meshes.push_back(mesh);
					}

					mesh.clear();
					mesh.name = lineParts[1];
				}
				else if (TextUtils::tolower(lineParts[0]) == "usemtl" && lineParts.size()>1)
				{
					currentMaterial = lineParts[1];
					underscoreBeforeNumbers(currentMaterial);
				}
				else if (TextUtils::tolower(lineParts[0]) == "f" && lineParts.size()>3)
				{
					CFace face;
					face.material = currentMaterial;

					int partCount = (int)lineParts.size();
					for ( int i = 1; i < partCount; i++ )
					{
						std::string section = lineParts[i];

						// TextUtils::tokenize() does not make 3
						// strings from "1//2", so do it the hard way
						const std::string::size_type npos = std::string::npos;
									std::string::size_type pos1, pos2 = npos;
						pos1 = section.find_first_of('/');
						if (pos1 != npos) {
							pos2 = section.find_first_of('/', pos1 + 1);
						}

						std::string vertPart, uvPart, normPart;
						if (pos1 == npos) {
						  vertPart = section;
						} else {
						  vertPart = section.substr(0, pos1);
						  if (pos2 == npos) {
						    uvPart = section.substr(pos1 + 1, npos);
							} else {
						    uvPart = section.substr(pos1 + 1, pos2 - pos1 - 1);
						    normPart = section.substr(pos2 + 1, npos);
							}
						}

						if (vertPart.size() > 0) {
						  int index = atoi(vertPart.c_str());
						  if (index < 0) {
						    index = (vCount + 1) + index;
							}
							face.verts.push_back(index - 1);
						}
						if (uvPart.size() > 0) {
						  int index = atoi(uvPart.c_str());
						  if (index < 0) {
						    index = (tCount + 1) + index;
							}
							face.texCoords.push_back(index - 1);
						}
						if (normPart.size() > 0) {
						  int index = atoi(normPart.c_str());
						  if (index < 0) {
						    index = (nCount + 1) + index;
							}
							face.normals.push_back(index - 1);
						}
					}

					bool valid = true;
					const int vSize = (int)face.verts.size();
					const int nSize = (int)face.normals.size();
					const int tSize = (int)face.texCoords.size();
					if ((nSize != 0) && (nSize != vSize)) {
						printf ("vertex/normal count mismatch\n");
						valid = false;
					}
					if ((tSize != 0) && (tSize != vSize)) {
						printf ("vertex/texcoord count mismatch\n");
						valid = false;
					}
					if (valid) {
					  mesh.faces.push_back(face);
					}
				}
			}
		}
		lineItr++;
	}

	if (mesh.valid())
	{
		mesh.verts = temp_verts;
		mesh.normals = temp_normals;
		mesh.texCoords = temp_texCoords;
		model.meshes.push_back(mesh);
	}
}

static void writeBZW  ( CModel &model, std::string file )
{
	if (model.meshes.size() < 1 )
		return;

	FILE *fp = fopen (file.c_str(),"wt");
	if (!fp)
		return;

	if (useMaterials) {
	  tmMaterialMap::iterator materialItr = model.materials.begin();
		while ( materialItr != model.materials.end() )
		{
			CMaterial &material = materialItr->second;
			fprintf (fp,"material\n  name %s\n",materialItr->first.c_str());
			if ( material.texture.size()) {
			  std::string texName = texdir + material.texture;
				// change the extension to png
				char *p = strrchr(texName.c_str(), '.');
				if (p) {
					texName.resize(p - texName.c_str());
					texName += ".png";
				} else {
				  texName += ".png";
				}
			  fprintf (fp,"  texture %s\n", texName.c_str());
			} else {
				fprintf (fp,"  notextures\n");
			}
			if (useAmbient)
				fprintf (fp,"  ambient %f %f %f %f\n", material.ambient[0], material.ambient[1], material.ambient[2], material.ambient[3]);
			if (useDiffuse)
				fprintf (fp,"  diffuse %f %f %f %f\n", material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]);
			if (useSpecular)
				fprintf (fp,"  specular %f %f %f %f\n", material.specular[0], material.specular[1], material.specular[2], material.specular[3]);
			if (useShininess)
				fprintf (fp,"  shininess %f\n", material.shine);
			if (useEmission)
				fprintf (fp,"  emission %f %f %f %f\n", material.emission[0], material.emission[1], material.emission[2], material.emission[3]);

			fprintf (fp,"end\n\n");

			materialItr++;
		}
		fprintf (fp,"\n");
	}

	if (groupName.size() > 0) {
	  fprintf (fp, "define %s\n", groupName.c_str());
	}

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
			fprintf (fp,"  vertex %f %f %f\n", vertItr->x,vertItr->y,vertItr->z);
			vertItr++;
		}

		vertItr = mesh.normals.begin();
		while ( vertItr != mesh.normals.end() )
		{
			fprintf (fp,"  normal %f %f %f\n", vertItr->x,vertItr->y,vertItr->z);
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

      if (useNormals && (face.normals.size() > 0)) {
				indexItr = face.normals.begin();
				fprintf (fp,"    normals");
				while ( indexItr != face.normals.end() )
					fprintf(fp," %d",*indexItr++);
				fprintf (fp,"\n");
			}

      if (useTexcoords && (face.texCoords.size() > 0)) {
				indexItr = face.texCoords.begin();
				fprintf (fp,"    texcoords");
				while ( indexItr != face.texCoords.end() )
					fprintf(fp," %d",*indexItr++);
				fprintf (fp,"\n");
			}

			if (useMaterials && (face.material.size() > 0)) {
			  fprintf (fp, "    matref %s\n", face.material.c_str());
			}

			fprintf (fp,"  endface\n");

			faceItr++;
		}

		fprintf (fp,"end\n\n");
		meshItr++;
	}

	if (groupName.size() > 0) {
	  fprintf (fp, "enddef # %s\n", groupName.c_str());
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
	printf("       -sm        : use the smoothbounce property\n");
	printf("       -yz        : flip y and z coordinates\n");
	printf("       -n         : disable normals\n");
	printf("       -t         : disable texture coordinates\n");
	printf("       -m         : disable materials\n");
	printf("       -a         : disable ambient coloring\n");
	printf("       -d         : disable diffuse coloring\n");
	printf("       -s         : disable specular coloring\n");
	printf("       -sh        : disable shininess\n");
	printf("       -sf <val>  : shine multiplier\n");
	printf("       -e         : disable emission coloring\n\n");
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

	if (!p) {
		output = input + ".bzw";
	} else {
		*p = '\0'; // clip the old extension
		output = argv[1] + std::string(".bzw");
	}

	for ( int i = 2; i < argc; i++) {
		std::string command = argv[i];
		command = TextUtils::tolower(command);

		if (command == "-yz") {
			flipYZ = true;
		}
		else if (command == "-g") {
		  if ((i + 1) < argc) {
			  i++;
			  groupName = argv[i];
			} else {
			  printf ("missing -g argument\n");
			}
		}
		else if (command == "-tx") {
		  if ((i + 1) < argc) {
			  i++;
			  texdir = argv[i];
			  if (texdir[texdir.size()] != '/') {
			    texdir += '/';
				}
			} else {
			  printf ("missing -tx argument\n");
			}
		}
		else if (command == "-sm") {
			useSmoothBounce = true;
		}
		else if (command == "-n") {
			useNormals = false;
		}
		else if (command == "-t") {
			useTexcoords = false;
		}
		else if (command == "-m") {
			useMaterials = false;
		}
		else if (command == "-a") {
			useAmbient = false;
		}
		else if (command == "-d") {
			useDiffuse = false;
		}
		else if (command == "-s") {
			useSpecular = false;
		}
		else if (command == "-sh") {
			useShininess = false;
		}
		else if (command == "-sf") {
		  if ((i + 1) < argc) {
			  i++;
			  shineFactor = atof(argv[i]);
			} else {
			  printf ("missing -sf argument\n");
			}
		}
		else if (command == "-e") {
			useEmission = false;
		}
	}
	// make a model

	CModel	model;

	if ( TextUtils::tolower(extenstion) == "obj" )
	{
		readOBJ(model,input);
	}
	else
	{
		printf("unknown input format\n");
		return 2;
	}

	model.pushAboveAxis(eZAxis);

	if (model.meshes.size() > 0) {
		writeBZW(model,output);
		printf("%s file %s converted to BZW as %s\n", extenstion.c_str(),input.c_str(),output.c_str());
	} else {
		printf("no valid meshes written from %s\n", input.c_str());
	}

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


