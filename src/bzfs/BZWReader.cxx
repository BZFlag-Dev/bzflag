/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "BZWReader.h"

// implementation-specific system headers
#include <sstream>
#include <ctype.h>
#include <string>
#include <vector>

// implementation-specific bzflag headers
#include "bzfstream.h"
#include "BZDBCache.h"

// implementation-specific bzfs-specific headers
#include "TeamBases.h"
#include "WorldFileObject.h"
#include "CustomArc.h"
#include "CustomBase.h"
#include "CustomBox.h"
#include "CustomCone.h"
#include "CustomDynamicColor.h"
#include "CustomTeleporter.h"
#include "CustomGroup.h"
#include "CustomLink.h"
#include "CustomMaterial.h"
#include "CustomMesh.h"
#include "CustomMeshTransform.h"
#include "CustomPhysicsDriver.h"
#include "CustomPyramid.h"
#include "CustomSphere.h"
#include "CustomTetra.h"
#include "CustomTextureMatrix.h"
#include "CustomWaterLevel.h"
#include "CustomWeapon.h"
#include "CustomWorld.h"
#include "CustomWorldText.h"
#include "CustomZone.h"

// common headers
#include "ObstacleMgr.h"
#include "BaseBuilding.h"
#include "MeshObstacle.h"
#include "MeshFace.h"
#include "TextUtils.h"
#include "StateDatabase.h"

// bzfs specific headers
#include "bzfs.h"


BZWReader::BZWReader(const std::string &filename)
: cURLManager()
, location(filename)
, input(NULL)
, fromBlob(false)
{
  static const std::string httpProtocol("http://");
  static const std::string ftpProtocol("ftp://");
  static const std::string fileProtocol("file:/");

  errorHandler = new BZWError(location);

  if ((filename.substr(0, httpProtocol.size()) == httpProtocol)
      || (filename.substr(0, ftpProtocol.size()) == ftpProtocol)
      || (filename.substr(0, fileProtocol.size()) == fileProtocol)) {
    setURL(location);
    performWait();
    input = new std::istringstream(httpData);
  } else {
    input = createIFStream(filename);
  }

  // .BZW is the official worldfile extension, warn for others
  if ((filename.length() < 4) ||
      (strcasecmp(filename.substr(filename.length() - 4, 4).c_str(),
		  ".bzw") != 0)) {
    errorHandler->warning(
      "world file extension is not .bzw, trying to load anyway", 0);
  }

  if (input->peek() == EOF) {
    errorHandler->fatalError("could not find bzflag world file", 0);
  }
}


BZWReader::BZWReader(std::istream &in)
: cURLManager()
, location("blob")
, input(&in)
, fromBlob(true)
{
  errorHandler = new BZWError(location);
  if (input->peek() == EOF) {
    errorHandler->fatalError("could not find bzflag world file", 0);
  }
}


BZWReader::~BZWReader()
{
  // clean up
  delete errorHandler;
  if (!fromBlob) delete input;
}


void BZWReader::finalization(char *data, unsigned int length, bool good)
{
  if (good)
    httpData = std::string(data, length);
  else
    httpData = "";
}


void BZWReader::readToken(char *buffer, int bufSize)
{
  int c = -1;

  // skip whitespace (but not newlines)
  while (input->good()) {
    c = input->get();
    if ((c == -1) || !isspace(c) || (c == '\n')) {
      break;
    }
  }

  // read up to whitespace or (bufSize - 1) characters into buffer
  int i = 0;
  if ((c != -1) && (c != '\n')) {
    buffer[i] = (char)c;
    i++;
    while (input->good() && (i < (bufSize - 1))) {
      c = input->get();
      if ((c == -1) || isspace(c)) {
        break;
      }
      buffer[i] = (char)c;
      i++;
    }
  }

  // terminate string
  buffer[i] = 0;

  // put back last character we didn't use
  if ((c != -1) && isspace(c)) {
    input->putback(c);
  }
}


bool BZWReader::parseNormalObject(const char* token, WorldFileObject** object)
{
  const std::string lower = TextUtils::tolower(token);

  WorldFileObject*& obj = *object;

  char name[256];
  readToken(name, sizeof(name));
  if (name[0] == '#') {
    name[0] = 0;
  }

       if (lower == "box")           { obj = new CustomBox(false);          }
  else if (lower == "meshbox")       { obj = new CustomBox(true);           }
  else if (lower == "pyramid")       { obj = new CustomPyramid(false);      }
  else if (lower == "meshpyr")       { obj = new CustomPyramid(true);       }
  else if (lower == "base")          { obj = new CustomBase;                }
  else if (lower == "arc")           { obj = new CustomArc;                 }
  else if (lower == "cone")          { obj = new CustomCone;                }
  else if (lower == "sphere")        { obj = new CustomSphere;              }
  else if (lower == "tetra")         { obj = new CustomTetra;               }
  else if (lower == "waterlevel")    { obj = new CustomWaterLevel;          }
  else if (lower == "text")          { obj = new CustomWorldText;           }
  else if (lower == "link")          { obj = new CustomLink(false);         }
  else if (lower == "linkset")       { obj = new CustomLink(true);          }
  else if (lower == "weapon")        { obj = new CustomWeapon;              }
  else if (lower == "zone")          { obj = new CustomZone;                }
  else if (lower == "transform")     { obj = new CustomMeshTransform;       }
  else if (lower == "mesh")          { obj = new CustomMesh(name);          }
  else if (lower == "teleporter")    { obj = new CustomTeleporter(name);    }
  else if (lower == "dynamiccolor")  { obj = new CustomDynamicColor(name);  }
  else if (lower == "texturematrix") { obj = new CustomTextureMatrix(name); }
  else if (lower == "material")      { obj = new CustomMaterial(name);      }
  else if (lower == "physics")       { obj = new CustomPhysicsDriver(name); }
  else {
    // put the name token back into the stream
    const int nameLen = (int)strlen(name);
    for (int i = (nameLen - 1); i >= 0; i--) {
      input->putback(name[i]);
    }
    if (nameLen > 0) {
      input->putback(' ');
    }
    return false; // no match found
  }

  if (obj != NULL) {
    obj->typeName = lower;
  }

  return true;
}


bool BZWReader::parseCustomObject(const char* token, bool& error, int& lineNum,
                                  std::vector<WorldFileObject*>& wlist,
                                  GroupDefinition* groupDef, bool& gotWorld)
{
  static int depth = 0;
  depth++;

  const std::string upperToken = TextUtils::toupper(token);
  CustomObjectMap::iterator it = customObjectMap.find(upperToken);
  if (it == customObjectMap.end()) {
    depth--;
    return false;
  }

  const std::string& endToken = it->second.endToken;
  std::string args;
  std::vector<std::string> customLines;
  if (!readRawLines(args, customLines, endToken, lineNum)) {
    std::string msg;
    msg += "missing \"" + TextUtils::tolower(endToken) + "\"";
    msg += " for \"" + TextUtils::tolower(upperToken) + "\"";
    errorHandler->fatalError(msg, lineNum);
    error = true;
    depth--;
  }
  else {
    bz_CustomMapObjectInfo data;

    data.fileName = location;
    data.lineNum = lineNum;
    data.name = bz_ApiString(upperToken);
    data.args = bz_ApiString(args);
    for (unsigned int i = 0; i < customLines.size(); i++) {
      data.data.push_back(customLines[i]);
    }

    bz_ApiString objName = bz_ApiString(upperToken);
    customObjectMap[upperToken].handler->handle(objName, &data);

    // embedded objects
    if (data.newData.size() > 0) {
      logDebugMessage(1, "%s: (line %i): embedding for \"%s\"\n",
                      location.c_str(), lineNum, data.name.c_str());
      std::istringstream newData(data.newData.c_str());
      BZWReader incStream(newData);
      std::vector<WorldFileObject*> incWlist;
      if (incStream.readWorldStream(incWlist, groupDef, gotWorld)) {
        // add the included objects
        for (unsigned int i = 0; i < incWlist.size(); i++) {
          wlist.push_back(incWlist[i]);
        }
      }
      else {
        // empty the failed list
        emptyWorldFileObjectList(incWlist);
        std::string msg = "embedding \"";
        msg += data.name.c_str();
        msg += "\"";
        errorHandler->fatalError(msg, lineNum);
        error = true;
        depth--;
      }
    }
  }

  depth--;
  return true;
}


bool BZWReader::readRawLines(std::string& args, std::vector<std::string>& lines,
                             const std::string& endToken, int& lineNum)
{
  std::getline(*input, args);
  if (endToken.empty()) {
    input->putback('\n'); // only read the args, single-line object
    return true;
  }

  while (!input->eof() && !input->fail() && input->good()) {
    std::string line;
    std::getline(*input, line);
    lineNum++;

    if (!line.empty()) {
      logDebugMessage(4, "reading worldfile raw line: %s\n", line.c_str());
    }

    const char* start = TextUtils::skipWhitespace(line.c_str());
    const char* end   = TextUtils::skipNonWhitespace(start);
    const std::string token(start, end - start);
    if (strcasecmp(token.c_str(), endToken.c_str()) == 0) {
      input->putback('\n');
      return true;
    }

    lines.push_back(line);
  }
  return false;
}


bool BZWReader::readWorldStream(std::vector<WorldFileObject*>& wlist,
				GroupDefinition* groupDef,
				bool& gotWorld)
{
  // make sure input is valid
  if (input->peek() == EOF) {
    errorHandler->fatalError("unexpected EOF", 0);
    return false;
  }

  int lineNum = 1;
  char buffer[4096];
  WorldFileObject* object = NULL;
  WorldFileObject* newObject = NULL;
  GroupDefinition* worldDef = (GroupDefinition*)&OBSTACLEMGR.getWorld();
  GroupDefinition* startDef = groupDef;
  bool error = false;

  while (!input->eof() && !input->fail() && input->good()) {
    // watch out for starting a new object when one is already in progress
    if (newObject) {
      if (object) {
	errorHandler->warning("discarding incomplete object", lineNum);
      }
      object = newObject;
      newObject = NULL;
    }

    // read first token but do not skip newlines
    readToken(buffer, sizeof(buffer));
    if (buffer[0] != '\0') {
      logDebugMessage(4, "reading worldfile token %s\n",buffer);
    }

    if (buffer[0] == '\0') {
      // ignore blank line
    }
    else if (buffer[0] == '#') {
      // ignore comment
    }
    else if (strncmp(buffer, "//", 2) == 0) {
      // ignore comment
    }
    else if (strncmp(buffer, "/*", 2) == 0) {
      std::string args;
      std::vector<std::string> commentLines;
      if (!readRawLines(args, commentLines, "*/", lineNum)) {
        errorHandler->fatalError("missing block comment termination, \"*/\"", lineNum);
        return false;
      }
    }
    else if (strcasecmp(buffer, "end") == 0) {
      if (!object) {
	errorHandler->fatalError("unexpected \"end\" token", lineNum);
	return false;
      }
      else {
        if (object->usesManager()) {
          object->writeToManager();
          delete object;
        }
        else if (object->usesGroupDef()) {
          object->writeToGroupDef(groupDef);
          delete object;
        }
        else {
          wlist.push_back(object);
        }
	object = NULL;
      }
    }
    else if (object) { // filling the current object
      if (!object->read(buffer, *input)) {
        // unknown token
        errorHandler->warning(
          std::string("unknown ") + object->typeName +
          std::string(" parameter \"") +
          std::string(buffer) + std::string("\" - skipping"), lineNum);
        // delete object;
        // return false;
      }
    }
    else if (parseCustomObject(buffer, error, lineNum,
                               wlist, groupDef, gotWorld)) {
      if (error) {
       return false;
      }
    }
    else if (parseNormalObject(buffer, &newObject)) {
      // newObject has been assigned
    }
    else if (strcasecmp(buffer, "define") == 0) {
      if (groupDef != worldDef) {
	errorHandler->warning(
	  std::string("group definitions can not be nested \"") +
	  std::string(buffer) + std::string("\" - skipping"), lineNum);
      } else {
	readToken(buffer, sizeof(buffer));
	if (strlen(buffer) > 0) {
	  if (OBSTACLEMGR.findGroupDef(buffer) != NULL) {
	    errorHandler->warning(
	      std::string("duplicate group definition \"") +
	      std::string(buffer) + std::string("\" - using newest"), lineNum);
	  }
	  groupDef = new GroupDefinition(buffer);
	} else {
	  errorHandler->warning("missing group definition name", lineNum);
	}
      }
    }
    else if (strcasecmp(buffer, "enddef") == 0) {
      if (groupDef == worldDef) {
	errorHandler->warning("enddef without define - skipping", lineNum);
      } else {
	OBSTACLEMGR.addGroupDef(groupDef);
	groupDef = worldDef;
      }
    }
    else if (strcasecmp(buffer, "group") == 0) {
      readToken(buffer, sizeof(buffer));
      if (strlen(buffer) <= 0) {
	errorHandler->warning("missing group definition reference", lineNum);
      }
      char nameBuf[256];
      readToken(nameBuf, sizeof(nameBuf));
      newObject = new CustomGroup(buffer, nameBuf);
      newObject->typeName = "group";
    }
    else if (strcasecmp(buffer, "world") == 0) {
      if (!gotWorld) {
	newObject = new CustomWorld();
	newObject->typeName = "world";
	gotWorld = true;
      } else {
	errorHandler->warning("multiple \"world\" sections found", lineNum);
      }
    }
    else if (strcasecmp(buffer, "options") == 0) {
      std::string args;
      std::vector<std::string> optionLines;
      if (!readRawLines(args, optionLines, "end", lineNum)) {
        errorHandler->fatalError("missing \"end\" for \"options\"", lineNum);
        return false;
      }
      if (clOptions == NULL) {
        errorHandler->fatalError("INTERNAL ERROR: options without clOptions",
                                 lineNum);
        return false;
      }
      clOptions->parseWorldOptions(optionLines);
    }
    else if (strcasecmp(buffer, "info") == 0) {
      std::string args;
      if (!readRawLines(args, mapInfoLines, "end", lineNum)) {
        errorHandler->fatalError("missing \"end\" for \"info\"", lineNum);
        return false;
      }
    }
    else if (strcasecmp(buffer, "include") == 0) {
      // NOTE: intentionally undocumented  (at the moment)
      readToken(buffer, sizeof(buffer));
      std::string incName = buffer;
      if (object == NULL) {
	// FIXME - check for recursion
	//       - better filename handling ("", spaces, and / vs. \\)
	//       - make relative names work from the base file location
	logDebugMessage(1,"%s: (line %i): including \"%s\"\n",
	                location.c_str(), lineNum, incName.c_str());
	BZWReader incFile(incName);
	std::vector<WorldFileObject*> incWlist;
	if (incFile.readWorldStream(incWlist, groupDef, gotWorld)) {
	  // add the included objects
	  for (unsigned int i = 0; i < incWlist.size(); i++) {
	    wlist.push_back(incWlist[i]);
	  }
	} else {
	  // empty the failed list
	  emptyWorldFileObjectList(incWlist);
	  errorHandler->fatalError(
	    TextUtils::format("including \"%s\"", incName.c_str()), lineNum);
	  return false;
	}
      }
      else {
	errorHandler->warning(
	  TextUtils::format("including \"%s\" within an obstacle, skipping",
			    incName.c_str()), lineNum);
      }
    }
    else {
      errorHandler->warning(
        TextUtils::format("unknown token \"%s\"", buffer), lineNum);
    }

    // discard remainder of line
    while (input->good() && input->peek() != '\n') {
      input->get(buffer, sizeof(buffer));
    }

    input->getline(buffer, sizeof(buffer));

    ++lineNum;
  }

  bool retval = true;
  if (object) {
    errorHandler->fatalError("missing \"end\" parameter", lineNum);
    delete object;
    retval = false;
  }

  if (groupDef != startDef) {
    errorHandler->fatalError("missing \"enddef\" parameter", lineNum);
    if (startDef == worldDef) {
      delete groupDef;
    }
    retval = false;
  }

  return retval;
}


WorldInfo* BZWReader::defineWorldFromFile()
{
  // create world object
  WorldInfo *myWorld = new WorldInfo;
  if (!myWorld) {
    errorHandler->fatalError("WorldInfo failed to initialize", 0);
    return NULL;
  }

  // read file
  bool gotWorld = false;
  std::vector<WorldFileObject*> list;
  GroupDefinition* worldDef = (GroupDefinition*)&OBSTACLEMGR.getWorld();
  if (!readWorldStream(list, worldDef, gotWorld)) {
    emptyWorldFileObjectList(list);
    errorHandler->fatalError("world file failed to load.", 0);
    delete myWorld;
    return NULL;
  }

  if (!gotWorld) {
    // add fake empty "world" section so walls will be created
    WorldFileObject* object = new CustomWorld();
    list.push_back(object);
    gotWorld = true;
  }

  // set the mapinfo
  if (!mapInfoLines.empty()) {
    myWorld->setMapInfo(mapInfoLines);
  }

  unsigned int i;

  // add objects
  const unsigned int n = list.size();
  for (i = 0; i < n; ++i) {
    list[i]->writeToWorld(myWorld);
  }

  // generate group instances
  OBSTACLEMGR.makeWorld();

  // BaseBuilding bases
  const ObstacleList& baseList = OBSTACLEMGR.getBases();
  for (i = 0; i < baseList.size(); i++) {
    const BaseBuilding* base = (const BaseBuilding*) baseList[i];
    TeamColor color = (TeamColor)base->getBaseTeam();
    if (bases.find(color) == bases.end()) {
      bases[color] = TeamBases((TeamColor)color);
    }
    bases[color].addBase(base);
  }

  // MeshFace bases
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  for (i = 0; i < meshes.size(); i++) {
    const MeshObstacle* mesh = (const MeshObstacle*) meshes[i];
    if (!mesh->getHasSpecialFaces()) {
      continue;
    }
    const int faceCount = mesh->getFaceCount();
    for (int f = 0; f < faceCount; f++) {
      const MeshFace* face = mesh->getFace(f);
      if (face->isBaseFace()) {
        TeamColor color = (TeamColor)face->getBaseTeam();
        if (bases.find(color) == bases.end()) {
          bases[color] = TeamBases((TeamColor)color);
        }
        bases[color].addBase(face);
      }
    }
  }

  // clean up
  emptyWorldFileObjectList(list);
  myWorld->finishWorld();

  return myWorld;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
