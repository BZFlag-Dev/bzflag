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

#include "common.h"

/* interface header */
#include "CustomWorldText.h"

/* bzfs headers */
#include "ParseMaterial.h"

/* common headers */
#include "WorldText.h"
#include "ParseColor.h"

/* system headers */
#include <iostream>
#include <sstream>
#include <vector>
#include <string.h>


CustomWorldText::CustomWorldText()
{
  text = new WorldText;
  upright = false;
  return;
}


CustomWorldText::~CustomWorldText()
{
  delete text;
  return;
}


bool CustomWorldText::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (strcasecmp ("data", cmd) == 0) {
    std::string line;
    std::getline(input, line);
    if (line[0] == ' ') {
      line = line.substr(1);
    }
    text->data = line;
    input.putback('\n');
    if (text->data.empty()) {
      std::cout << "World Text: missing data" << std::endl;
    }
  }
  else if (strcasecmp ("font", cmd) == 0) {
    std::string font;
    if (input >> font) {
      text->font = font;
      return true;
    } else {
      std::cout << "missing text font" << std::endl;
      return false;
    }
  }
  else if (strcasecmp ("fontSize", cmd) == 0) {
    if (!(input >> text->fontSize)) {
      return false;
    }
  }
  else if (strcasecmp ("justify", cmd) == 0) {
    if (!(input >> text->justify)) {
      return false;
    }
  }
  else if (strcasecmp ("lineSpace", cmd) == 0) {
    if (!(input >> text->lineSpace)) {
      return false;
    }
  }
  else if (strcasecmp ("fixedWidth", cmd) == 0) {
    if (!(input >> text->fixedWidth)) {
      return false;
    }
  }
  else if ((strcasecmp ("length", cmd) == 0) ||
           (strcasecmp ("lengthPerPixel", cmd) == 0)) {
    if (!(input >> text->lengthPerPixel)) {
      return false;
    }
  }
  else if (strcasecmp ("minDist", cmd) == 0) {
    float minDist;
    if (!(input >> minDist)) {
      return false;
    }
    const float fov    = 60.0f; // defaultFOV
    const float pixels = 1024.0f;

    text->lengthPerPixel =
      minDist * tan(fov * 0.5f * ((float)M_PI / 180.0f)) / (pixels * 0.5f);
  }
  else if (strcasecmp ("bzdb", cmd) == 0) {
    text->useBZDB = true;
  }
  else if (strcasecmp ("billboard", cmd) == 0) {
    text->billboard = true;
  }
  else if (strcasecmp ("upright", cmd) == 0) {
    upright = true;
  }
  else if (strcasecmp ("polyoffset", cmd) == 0) {
    if (!(input >> text->poFactor >> text->poUnits)) {
      return false;
    }
  }
  else if (parseMaterials(cmd, input, &material, 1, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    return WorldFileLocation::read(cmd, input);
  }

  return true;
}


void CustomWorldText::writeToManager() const
{
  MeshTransform xform;

  if ((size[0] != 1.0f) || (size[1] != 1.0f) || (size[2] != 1.0f)) {
    xform.addScale(size);
  }
  if (upright) {
    const fvec3 xAxis(1.0f, 0.0f, 0.0f);
    xform.addSpin(90.0f, xAxis);
  }
  if (rotation != 0.0f) {
    const fvec3 zAxis(0.0f, 0.0f, 1.0f);
    xform.addSpin((float)(rotation * (180.0 / M_PI)), zAxis);
  }
  if ((pos[0] != 0.0f) || (pos[1] != 0.0f) || (pos[2] != 0.0f)) {
    xform.addShift(pos);
  }

  xform.append(transform);
  text->xform = xform;

  text->bzMaterial = MATERIALMGR.addMaterial(&material);

  text->name = name;

  WORLDTEXTMGR.addText(text);
  text = NULL;

  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
