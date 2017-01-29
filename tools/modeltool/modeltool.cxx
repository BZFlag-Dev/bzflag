//
// modeltool.cpp : Defines the entry point for the console application.
//
//
// If you're doing a command line compilation, and
// the Makefile is broken, then this may do the job:
//
//   g++ -o modeltool modeltool.cxx  -I../../include ../../src/common/libCommon.a
//

//#include "common.h"

/* system headers */
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <vector>
#include <stdarg.h>

/* common headers */
/*#include "TextUtils.h"*/

#include "model.h"
#include "wavefrontOBJ.h"
#include "Q3BSP.h"

std::string tolower(const std::string& s)
{
	std::string trans = s;

	for (std::string::iterator i = trans.begin(), end = trans.end(); i != end; ++i)
		*i = ::tolower(*i);
	return trans;
}

/** returns a string converted to uppercase
*/
 std::string toupper(const std::string& s)
{
	std::string trans = s;
	for (std::string::iterator i = trans.begin(), end = trans.end(); i != end; ++i)
		*i = ::toupper(*i);
	return trans;
}

std::string vformat(const char* fmt, va_list args) {
	const int fixedbs = 8192;
	char buffer[fixedbs];
	const int bs = vsnprintf(buffer, fixedbs, fmt, args) + 1;
	if (bs > fixedbs) {
		char *bufp = new char[bs];
		vsnprintf(bufp, bs, fmt, args);
		std::string ret = bufp;
		delete[] bufp;
		return ret;
	}

	return buffer;
}

// text functions needed
std::string format(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::string result = vformat(fmt, args);
	va_end(args);
	return result;
}

std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe)
{
	std::string::size_type beginPos = 0;
	std::string::size_type endPos = 0;
	std::ostringstream tempStream;

	endPos = in.find(replaceMe);
	if (endPos == std::string::npos)
		return in; // can't find anything to replace
	if (replaceMe.empty()) return in; // can't replace nothing with something -- can do reverse

	while (endPos != std::string::npos) {
		// push the  part up to
		tempStream << in.substr(beginPos, endPos - beginPos);
		tempStream << withMe;
		beginPos = endPos + replaceMe.size();
		endPos = in.find(replaceMe, beginPos);
	}
	tempStream << in.substr(beginPos);
	return tempStream.str();
}

std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens, const bool useQuotes) {
	std::vector<std::string> tokens;
	int numTokens = 0;
	bool inQuote = false;

	std::ostringstream currentToken;

	const std::string::size_type len = in.size();
	std::string::size_type pos = in.find_first_not_of(delims);

	int currentChar = (pos == std::string::npos) ? -1 : in[pos];
	bool enoughTokens = (maxTokens && (numTokens >= (maxTokens - 1)));

	while (pos < len && pos != std::string::npos && !enoughTokens) {

		// get next token
		bool tokenDone = false;
		bool foundSlash = false;

		currentChar = (pos < len) ? in[pos] : -1;
		while ((currentChar != -1) && !tokenDone) {

			tokenDone = false;

			if (delims.find(currentChar) != std::string::npos && !inQuote) { // currentChar is a delim
				pos++;
				break; // breaks out of inner while loop
			}

			if (!useQuotes) {
				currentToken << char(currentChar);
			}
			else {

				switch (currentChar) {
				case '\\': // found a backslash
					if (foundSlash) {
						currentToken << char(currentChar);
						foundSlash = false;
					}
					else {
						foundSlash = true;
					}
					break;
				case '\"': // found a quote
					if (foundSlash) { // found \"
						currentToken << char(currentChar);
						foundSlash = false;
					}
					else { // found unescaped "
						if (inQuote) { // exiting a quote
									   // finish off current token
							tokenDone = true;
							inQuote = false;
							//slurp off one additional delimeter if possible
							if (pos + 1 < len &&
								delims.find(in[pos + 1]) != std::string::npos) {
								pos++;
							}

						}
						else { // entering a quote
							   // finish off current token
							tokenDone = true;
							inQuote = true;
						}
					}
					break;
				default:
					if (foundSlash) { // don't care about slashes except for above cases
						currentToken << '\\';
						foundSlash = false;
					}
					currentToken << char(currentChar);
					break;
				}
			}

			pos++;
			currentChar = (pos < len) ? in[pos] : -1;
		} // end of getting a Token

		if (currentToken.str().size() > 0) { // if the token is something add to list
			tokens.push_back(currentToken.str());
			currentToken.str("");
			numTokens++;
		}

		enoughTokens = (maxTokens && (numTokens >= (maxTokens - 1)));
		if ((pos < len) && (pos != std::string::npos)) {
			pos = in.find_first_not_of(delims, pos);
		}

	} // end of getting all tokens -- either EOL or max tokens reached

	if (enoughTokens && pos != std::string::npos) {
		std::string lastToken = in.substr(pos);
		if (lastToken.size() > 0)
			tokens.push_back(lastToken);
	}

	return tokens;
}

// globals/
const char VersionString[] = "ModelTool v1.7  (WaveFront OBJ/BZW to BZFlag BZW converter)";

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
bool useRicoMat = false;

float maxShineExponent = 128.0f; // OpenGL minimum shininess


float globalScale = 1.0f;
float globalShift[3] = {0,0,0};
std::vector<std::string> bspMaterialSkips; // materials to skip in a bsp map


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
				const char *p = strrchr(texName.c_str(), '.');
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

			if (useRicoMat && face.material.size() > 0 && strstr(face.material.c_str(),"rico_") != NULL) {
				fprintf (fp, "    ricochetn");
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
	printf("       -usericomat : faces that use materials that start with rico_ will have the ricochet property\n\n");
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
		command = tolower(command);

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
		else if (command == "-ricomat") {
			useRicoMat = true;
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
			  shineFactor = (float)atof(argv[i]);
			} else {
			  printf ("missing -sf argument\n");
			}
		}
		else if (command == "-e") {
			useEmission = false;
		}
		else if (command == "-gx") {
			if ((i + 1) < argc) {
				i++;
				globalScale = (float)atof(argv[i]);
			} else {
				printf ("missing -gx argument\n");
			}
		}
		else if (command == "-gsx") {
			if ((i + 1) < argc) {
				i++;
				globalShift[0] = (float)atof(argv[i]);
			} else {
				printf ("missing -gsx argument\n");
			}
		}
		else if (command == "-gsy") {
			if ((i + 1) < argc) {
				i++;
				globalShift[1] = (float)atof(argv[i]);
			} else {
				printf ("missing -gsy argument\n");
			}
		}
		else if (command == "-gsz") {
			if ((i + 1) < argc) {
				i++;
				globalShift[2] = (float)atof(argv[i]);
			} else {
				printf ("missing -gsz argument\n");
			}
		}
		else if (command == "-bspskip") {
			if ((i + 1) < argc) {
				i++;
				bspMaterialSkips.push_back(std::string(argv[i]));
			} else {
				printf ("missing -bspskip argument\n");
			}
		}
	}
	// make a model

	CModel	model;

	if ( tolower(extenstion) == "obj" )
	{
		readOBJ(model,input);
	}
	else if ( tolower(extenstion) == "bsp" )
	{
		Quake3Level	level;
		level.loadFromFile(input.c_str());
		level.dumpToModel(model);
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


