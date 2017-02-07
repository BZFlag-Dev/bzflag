
//
// wavefrontOBJ.cpp : reader for the OBJ files
//
//

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

/* common headers */
#include "SimpleTextUtils.h"
#include "wavefrontOBJ.h"

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

	fileText = SimpleTextUtils::replace_all(fileText,std::string("\r"),std::string(""));

	std::vector<std::string> lines = SimpleTextUtils::tokenize(fileText, lineTerminator);

	if ( lines.size() < 2 )
		return;

	CMaterial	material;
	std::string matName;
	material.clear();

	std::vector<std::string>::iterator lineItr = lines.begin();
	while ( lineItr != lines.end() )
	{
		// do a trim here

		std::vector<std::string> lineParts = SimpleTextUtils::tokenize(*lineItr,std::string(" "));

		if (lineParts.size() > 1)
		{
			std::string tag = lineParts[0];
			if (tag != "#")
			{
				if (SimpleTextUtils::tolower(tag) == "newmtl")
				{
					matName = lineParts[1];
					underscoreBeforeNumbers(matName);
					model.materials[matName] = material;
				}
				if (SimpleTextUtils::tolower(tag) == "ka")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].ambient[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].ambient[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].ambient[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (SimpleTextUtils::tolower(tag) == "kd")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].diffuse[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].diffuse[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].diffuse[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (SimpleTextUtils::tolower(tag) == "d")
				{
					if (lineParts.size() > 1)
					{
						model.materials[matName].ambient[3] = (float)atof(lineParts[1].c_str());
						model.materials[matName].diffuse[3] = (float)atof(lineParts[1].c_str());
					}
				}
				if (SimpleTextUtils::tolower(tag) == "ks")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].specular[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].specular[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].specular[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (SimpleTextUtils::tolower(tag) == "ns")
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
				if (SimpleTextUtils::tolower(tag) == "ke")
				{
					if (lineParts.size() > 3)
					{
						model.materials[matName].emission[0] = (float)atof(lineParts[1].c_str());
						model.materials[matName].emission[1] = (float)atof(lineParts[2].c_str());
						model.materials[matName].emission[2] = (float)atof(lineParts[3].c_str());
					}
				}
				if (SimpleTextUtils::tolower(tag) == "map_kd")
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

	const char *p = strrchr (file.c_str(),'\\');
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

	fileText = SimpleTextUtils::replace_all(fileText,std::string("\r"),std::string(""));

	std::vector<std::string> lines = SimpleTextUtils::tokenize(fileText,lineTerminator);

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

		std::vector<std::string> lineParts = SimpleTextUtils::tokenize(*lineItr,std::string(" "));

		if (lineParts.size() > 1)
		{
			if (lineParts[0] != "#")
			{
				if (SimpleTextUtils::tolower(lineParts[0]) == "mtllib" && lineParts.size()>1)
					readMTL(model,baseFilePath+lineParts[1]);
				else if (SimpleTextUtils::tolower(lineParts[0]) == "v" && lineParts.size()>3)
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
				else if (SimpleTextUtils::tolower(lineParts[0]) == "vt" && lineParts.size()>2)
				{
					CTexCoord uv;
					uv.u = (float)atof(lineParts[1].c_str());
					uv.v = (float)atof(lineParts[2].c_str());
					temp_texCoords.push_back(uv);
					tCount++;
				}
				else if (SimpleTextUtils::tolower(lineParts[0]) == "vn" && lineParts.size()>3)
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
				else if (SimpleTextUtils::tolower(lineParts[0]) == "g" && lineParts.size()>1)
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
				else if (SimpleTextUtils::tolower(lineParts[0]) == "usemtl" && lineParts.size()>1)
				{
					currentMaterial = lineParts[1];
					underscoreBeforeNumbers(currentMaterial);
				}
				else if (SimpleTextUtils::tolower(lineParts[0]) == "f" && lineParts.size()>3)
				{
					CFace face;
					face.material = currentMaterial;

					int partCount = (int)lineParts.size();
					for ( int i = 1; i < partCount; i++ )
					{
						std::string section = lineParts[i];

						// SimpleTextUtils::tokenize() does not make 3
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
