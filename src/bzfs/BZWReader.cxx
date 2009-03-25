/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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
using std::string;
using std::vector;

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
#include "CustomGate.h"
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
#include "TextUtils.h"
#include "StateDatabase.h"

// bzfs specific headers
#include "bzfs.h"


BZWReader::BZWReader(const string &filename) : cURLManager(),
					     location(filename),
					     input(NULL),
					     fromBlob(false)
{
  static const string httpProtocol("http://");
  static const string ftpProtocol("ftp://");
  static const string fileProtocol("file:/");

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
    errorHandler->warning(string(
      "world file extension is not .bzw, trying to load anyway"), 0);
  }

  if (input->peek() == EOF) {
    errorHandler->fatalError(string("could not find bzflag world file"), 0);
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
    errorHandler->fatalError(string("could not find bzflag world file"), 0);
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
    httpData = string(data, length);
  else
    httpData = "";
}


void BZWReader::readToken(char *buffer, int n)
{
  int c = -1;

  // skip whitespace
  while (input->good() && (c = input->get()) != -1 && isspace(c) && c != '\n')
    ;

  // read up to whitespace or n - 1 characters into buffer
  int i = 0;
  if (c != -1 && c != '\n') {
    buffer[i++] = c;
    while (input->good() && i < n - 1 && (c = input->get()) != -1 && !isspace(c))
      buffer[i++] = (char)c;
  }

  // terminate string
  buffer[i] = 0;

  // put back last character we didn't use
  if (c != -1 && isspace(c))
    input->putback(c);
}


static bool parseNormalObject(const char* token, WorldFileObject** object)
{
  WorldFileObject* tmpObj = NULL;

  if (strcasecmp(token, "box") == 0) {
    tmpObj = new CustomBox;
  } else if (strcasecmp(token, "pyramid") == 0) {
    tmpObj = new CustomPyramid();
  } else if (strcasecmp(token, "base") == 0) {
    tmpObj = new CustomBase;
  } else if (strcasecmp(token, "link") == 0) {
    tmpObj = new CustomLink();
  } else if (strcasecmp(token, "mesh") == 0) {
    tmpObj = new CustomMesh;
  } else if (strcasecmp(token, "arc") == 0) {
    tmpObj = new CustomArc(false);
  } else if (strcasecmp(token, "meshbox") == 0) {
    tmpObj = new CustomArc(true);
  } else if (strcasecmp(token, "cone") == 0) {
    tmpObj = new CustomCone(false);
  } else if (strcasecmp(token, "meshpyr") == 0) {
    tmpObj = new CustomCone(true);
  } else if (strcasecmp(token, "sphere") == 0) {
    tmpObj = new CustomSphere;
  } else if (strcasecmp(token, "tetra") == 0) {
    tmpObj = new CustomTetra();
  } else if (strcasecmp(token, "weapon") == 0) {
    tmpObj = new CustomWeapon;
  } else if (strcasecmp(token, "zone") == 0) {
    tmpObj = new CustomZone;
  } else if (strcasecmp(token, "waterLevel") == 0) {
    tmpObj = new CustomWaterLevel;
  } else if (strcasecmp(token, "dynamicColor") == 0) {
    tmpObj = new CustomDynamicColor;
  } else if (strcasecmp(token, "textureMatrix") == 0) {
    tmpObj = new CustomTextureMatrix;
  } else if (strcasecmp(token, "material") == 0) {
    tmpObj = new CustomMaterial;
  } else if (strcasecmp(token, "physics") == 0) {
    tmpObj = new CustomPhysicsDriver;
  } else if (strcasecmp(token, "transform") == 0) {
    tmpObj = new CustomMeshTransform;
  } else if (strcasecmp(token, "text") == 0) {
    tmpObj = new CustomWorldText;
  }

  if (tmpObj != NULL) {
    *object = tmpObj;
    return true;
  } else {
    return false;
  }
}


bool BZWReader::readRawLines(string& args, vector<string>& lines,
                             const string& endToken, int& lineNum)
{
  std::getline(*input, args);

  while (!input->eof() && !input->fail() && input->good()) {
    string line;
    std::getline(*input, line);
    lineNum++;

    if (!line.empty()) {
      logDebugMessage(4, "reading worldfile raw line: %s\n", line.c_str());
    }

    const char* start = TextUtils::skipWhitespace(line.c_str());
    const char* end   = TextUtils::skipNonWhitespace(start);
    const string token(start, end - start);
    if (strcasecmp(token.c_str(), endToken.c_str()) == 0) {
      input->putback('\n');
      return true;
    }

    lines.push_back(line);
  }
  return false;
}


bool BZWReader::readWorldStream(vector<WorldFileObject*>& wlist,
				GroupDefinition* groupDef,
				bool& gotWorld)
{
  // make sure input is valid
  if (input->peek() == EOF) {
    errorHandler->fatalError(string("unexpected EOF"), 0);
    return false;
  }

  int lineNum = 1;
  char buffer[4096];
  WorldFileObject* object = NULL;
  WorldFileObject* newObject = NULL;
  GroupDefinition* const worldDef = (GroupDefinition*)OBSTACLEMGR.getWorld();
  GroupDefinition* const startDef = groupDef;

  while (!input->eof() && !input->fail() && input->good()) {
    // watch out for starting a new object when one is already in progress
    if (newObject) {
      if (object) {
	errorHandler->warning(
	  string("discarding incomplete object"), lineNum);
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
    else if (strcasecmp(buffer, "end") == 0) {
      if (!object) {
	errorHandler->fatalError(
	  string("unexpected \"end\" token"), lineNum);
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
    else if (parseNormalObject(buffer, &newObject)) {
      // newObject has already been assigned
    }
    else if (strcasecmp(buffer, "define") == 0) {
      if (groupDef != worldDef) {
	errorHandler->warning(
	  string("group definitions can not be nested \"") +
	  string(buffer) + string("\" - skipping"), lineNum);
      } else {
	readToken(buffer, sizeof(buffer));
	if (strlen(buffer) > 0) {
	  if (OBSTACLEMGR.findGroupDef(buffer) != NULL) {
	    errorHandler->warning(
	      string("duplicate group definition \"") +
	      string(buffer) + string("\" - using newest"), lineNum);
	  }
	  groupDef = new GroupDefinition(buffer);
	} else {
	  errorHandler->warning(
	    string("missing group definition name"), lineNum);
	}
      }
    }
    else if (strcasecmp(buffer, "enddef") == 0) {
      if (groupDef == worldDef) {
	errorHandler->warning(
	  string("enddef without define - skipping"), lineNum);
      } else {
	OBSTACLEMGR.addGroupDef(groupDef);
	groupDef = worldDef;
      }
    }
    else if (strcasecmp(buffer, "group") == 0) {
      readToken(buffer, sizeof(buffer));
      if (strlen(buffer) <= 0) {
	errorHandler->warning(
	  string("missing group definition reference"), lineNum);
      }
      newObject = new CustomGroup(buffer);
    }
    else if (strcasecmp(buffer, "teleporter") == 0) {
      readToken(buffer, sizeof(buffer));
      newObject = new CustomGate(buffer);
    }
    else if (strcasecmp(buffer, "include") == 0) {
      // NOTE: intentionally undocumented  (at the moment)
      readToken(buffer, sizeof(buffer));
      string incName = buffer;
      if (object == NULL) {
	// FIXME - check for recursion
	//       - better filename handling ("", spaces, and / vs. \\)
	//       - make relative names work from the base file location
	logDebugMessage(1,"%s: (line %i): including \"%s\"\n",
		location.c_str(), lineNum, incName.c_str());
	BZWReader incFile(incName);
	vector<WorldFileObject*> incWlist;
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
    else if (strcasecmp(buffer, "world") == 0) {
      if (!gotWorld) {
	newObject = new CustomWorld();
	gotWorld = true;
      } else {
	errorHandler->warning(
	  string("multiple \"world\" sections found"), lineNum);
      }
    }
    else if (strcasecmp(buffer, "options") == 0) {
      string args;
      vector<string> optionLines;
      if (!readRawLines(args, optionLines, "end", lineNum)) {
        errorHandler->fatalError("missing \"end\" for \"options\"", lineNum);
        return false;
      }
    }
    else if (strcasecmp(buffer, "info") == 0) {
      string args;
      if (!readRawLines(args, mapInfoLines, "end", lineNum)) {
        errorHandler->fatalError("missing \"end\" for \"info\"", lineNum);
        return false;
      }
    }
    else if (object) { // filling the current object
      if (!object->read(buffer, *input)) {
        // unknown token
        errorHandler->warning(
          string("unknown object parameter \"") +
          string(buffer) + string("\" - skipping"), lineNum);
        // delete object;
        // return false;
      }
    }
    else {
      // handle custom objects
      const string upperToken = TextUtils::toupper(string(buffer));
      CustomObjectMap::iterator it = customObjectMap.find(upperToken);
      if (it != customObjectMap.end()) {
	string endToken = it->second.endToken;
	if (endToken.empty()) {
	  endToken = "end";
        }
        string args;
        vector<string> customLines;
        if (!readRawLines(args, customLines, endToken, lineNum)) {
          string msg;
          msg += "missing \"" + TextUtils::tolower(endToken) + "\"";
          msg += " for \"" + TextUtils::tolower(upperToken) + "\"";
          errorHandler->fatalError(msg, lineNum);
          return false;
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
            vector<WorldFileObject*> incWlist;
            if (incStream.readWorldStream(incWlist, groupDef, gotWorld)) {
              // add the included objects
              for (unsigned int i = 0; i < incWlist.size(); i++) {
                wlist.push_back(incWlist[i]);
              }
            }
            else {
              // empty the failed list
              emptyWorldFileObjectList(incWlist);
              errorHandler->fatalError(
                TextUtils::format("embedding \"%s\"", data.name.c_str()), lineNum);
              return false;
            }
          }
	}
      }
      else {
        errorHandler->warning(
          string("invalid object type \"") +
          string(buffer) + string("\" - skipping"), lineNum);
        // return false;
      }
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
    errorHandler->fatalError(string("missing \"end\" parameter"), lineNum);
    delete object;
    retval = false;
  }

  if (groupDef != startDef) {
    errorHandler->fatalError(string("missing \"enddef\" parameter"), lineNum);
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
    errorHandler->fatalError(string("WorldInfo failed to initialize"), 0);
    return NULL;
  }

  // read file
  bool gotWorld = false;
  vector<WorldFileObject*> list;
  GroupDefinition* worldDef = (GroupDefinition*)OBSTACLEMGR.getWorld();
  if (!readWorldStream(list, worldDef, gotWorld)) {
    emptyWorldFileObjectList(list);
    errorHandler->fatalError(string("world file failed to load."), 0);
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

  // generate group instances
  OBSTACLEMGR.makeWorld();

  // make local bases
  unsigned int i;
  const ObstacleList& baseList = OBSTACLEMGR.getBases();
  for (i = 0; i < baseList.size(); i++) {
    const BaseBuilding* base = (const BaseBuilding*) baseList[i];
    TeamColor color = (TeamColor)base->getTeam();
    if (bases.find(color) == bases.end()) {
      bases[color] = TeamBases((TeamColor)color);
    }
    bases[color].addBase(base->getPosition(),
                         base->getSize(),
                         base->getRotation());
  }

  // add objects
  const unsigned int n = list.size();
  for (i = 0; i < n; ++i) {
    list[i]->writeToWorld(myWorld);
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
