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

bool useSmothBounce = true;
bool flipYZ = false;

typedef std::vector<int> tvIndexList;

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
	float		shine;

	void clear ( void )
	{
		texture = "";
		ambient[0] = ambient[1] = ambient[2] = ambient[3] = 1;
		diffuse[0] = diffuse[1] = diffuse[2] = diffuse[3] = 1;
		specular[0] = specular[1] = specular[2] = specular[3] = 1;
		shine = 0;
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
	std::string material;
	tvFaceList	faces;

	bool valid ( void )
	{
		return material.size() || faces.size();
	}

	void clear ( void )
	{
		faces.clear();
		verts.clear();
		normals.clear();
		texCoords.clear();
		material = "";
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

	void clear ( void )
	{
		meshes.clear();
		materials.clear();
	}
};

void readMTL ( CModel &model, std::string file )
{
	FILE *fp = fopen(file.c_str(),"rb");
	if (!fp)
		return;

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

	TextUtils::replace_all(fileText,std::string("\r"),std::string(""));

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
					if (lineParts.size() > 1)
						model.materials[matName].shine = (float)atof(lineParts[1].c_str());
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

void readOBJ ( CModel &model, std::string file )
{
	model.clear();

	FILE *fp = fopen(file.c_str(),"r");
	if (!fp) {
	  printf ("Could not open %s\n", file.c_str());
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

	TextUtils::replace_all(fileText,std::string("\r"),std::string(""));

	std::vector<std::string> lines = TextUtils::tokenize(fileText,lineTerminator);

	if ( lines.size() < 2 )
		return;

	CMesh			mesh;
	tvVertList		temp_verts;
	tvVertList		temp_normals;
	tvTexCoordList	temp_texCoords;

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
				}
				else if (TextUtils::tolower(lineParts[0]) == "vt" && lineParts.size()>2)
				{
					CTexCoord uv;
					uv.u = (float)atof(lineParts[1].c_str());
					uv.v = (float)atof(lineParts[2].c_str());
					temp_texCoords.push_back(uv);
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
					mesh.material = lineParts[1];
				}
				else if (TextUtils::tolower(lineParts[0]) == "f" && lineParts.size()>3)
				{
					CFace face;

					int partCount = (int)lineParts.size();
					for ( int i = 1; i < partCount; i++ )
					{
						std::string section = lineParts[i];
						std::vector<std::string>	vertItems = TextUtils::tokenize(section,std::string("/"));
						if (vertItems.size() > 1)
						{
							std::string vertPart = vertItems[0];
							std::string uvPart = vertItems[1];
							std::string normPart = vertItems[2];

							face.verts.push_back(atoi(vertPart.c_str())-1);
							face.texCoords.push_back(atoi(uvPart.c_str())-1);
							face.normals.push_back(atoi(normPart.c_str())-1);
						}
					}	
					mesh.faces.push_back(face);
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

void writeBZW  ( CModel &model, std::string file )
{
	if (model.meshes.size() < 1 )
		return;

	FILE *fp = fopen (file.c_str(),"wt");
	if (!fp)
		return;

	tmMaterialMap::iterator materialItr = model.materials.begin();
	while ( materialItr != model.materials.end() )
	{
		CMaterial &material = materialItr->second;
		fprintf (fp,"material\n  name %s\n",materialItr->first.c_str());
		if ( material.texture.size())
			fprintf (fp,"  texture %s\n", material.texture.c_str());
		else
			fprintf (fp,"  notextures\n");

		fprintf (fp,"  ambient %f %f %f %f\n", material.ambient[0], material.ambient[1], material.ambient[2], material.ambient[3]);
		fprintf (fp,"  diffuse %f %f %f %f\n", material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]);
		fprintf (fp,"  specular %f %f %f %f\n", material.specular[0], material.specular[1], material.specular[2], material.specular[3]);
		fprintf (fp,"  shininess %f\n", material.shine);
		fprintf (fp,"end\n\n");

		materialItr++;
	}
	fprintf (fp,"\n");

	tvMeshList::iterator	meshItr = model.meshes.begin();

	while ( meshItr != model.meshes.end() )
	{
		CMesh	&mesh = *meshItr;

		mesh.reindex();

		fprintf (fp,"mesh # %s\n", mesh.name.c_str());
		if (mesh.material.size())
			fprintf (fp,"  matref %s\n", mesh.material.c_str());

		fprintf (fp,"# vertices: %d\n", (int)mesh.verts.size());
		fprintf (fp,"# normals: %d\n", (int)mesh.normals.size());
		fprintf (fp,"# texcoords: %d\n", (int)mesh.texCoords.size());
		fprintf (fp,"# faces: %d\n\n", (int) mesh.faces.size());

		if (useSmothBounce)
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

			indexItr = face.normals.begin();
			fprintf (fp,"    normals");
			while ( indexItr != face.normals.end() )
				fprintf(fp," %d",*indexItr++);
			fprintf (fp,"\n");

			indexItr = face.texCoords.begin();
			fprintf (fp,"    texcoords");
			while ( indexItr != face.texCoords.end() )
				fprintf(fp," %d",*indexItr++);
			fprintf (fp,"\n");

			fprintf (fp,"  endface\n");

			faceItr++;
		}

		fprintf (fp,"end\n\n");
		meshItr++;
	}
	fclose(fp);
}

int  dumpUsage ( char *exeName, const char* reason )
{
	printf("error: %s\n",reason);
	printf("usage: %s <input_file_name> -flipYZ -smoothBounce\n", exeName);
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
		argv[strlen(argv[1])-1] = 0;
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

	if ( argc > 2 )
	{
		for ( int i = 2; i < argc; i++)
		{
			std::string command = argv[i];

			if ( TextUtils::tolower(command) == "-flipyz")
				flipYZ = true;
			else if ( TextUtils::tolower(command) == "-smoothBounce")
				useSmothBounce = true;
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

	if (model.meshes.size() > 0) {
		writeBZW(model,output);
		printf("%s file %s converted to BZW as %s\n", extenstion.c_str(),input.c_str(),output.c_str());
	} else {
		printf("no valid meshes written from %s\n", input.c_str());
	}

	return 0;
}

int getNewIndex ( CVertex &vert, tvVertList &vertList )
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

int getNewIndex ( CTexCoord &vert, tvTexCoordList &vertList )
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


