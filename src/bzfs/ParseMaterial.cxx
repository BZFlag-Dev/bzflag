/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ParseMaterial.h"

/* common implementation headers */
#include "MeshMaterial.h"

bool parseMaterial(const char* cmd, std::istream& input,
                   MeshMaterial& material, bool& error)
{
  error = false; // currently unused

  if (strcasecmp(cmd, "resetmat") == 0) {
    material.reset();
  }
  else if (strcasecmp(cmd, "notexture") == 0) {
    material.useTexture = false;
  }
  else if (strcasecmp(cmd, "notexalpha") == 0) {
    material.useTextureAlpha = false;
  }
  else if (strcasecmp(cmd, "notexcolor") == 0) {
    material.useColorOnTexture = false;
  }
  else if (strcasecmp(cmd, "texture") == 0) {
    input >> material.texture;
  }
  else if (strcasecmp(cmd, "texmat") == 0) {
    input >> material.textureMatrix;
  }
  else if (strcasecmp(cmd, "dyncol") == 0) {
    input >> material.dynamicColor;
  }
  else if (strcasecmp(cmd, "ambient") == 0) {
    input >> material.ambient[0] >> material.ambient[1] >>
             material.ambient[2] >> material.ambient[3];
  }
  else if ((strcasecmp(cmd, "diffuse") == 0) || // currently used by bzflag
           (strcasecmp(cmd, "color") == 0)) {
    input >> material.diffuse[0] >> material.diffuse[1] >>
             material.diffuse[2] >> material.diffuse[3];
  }
  else if (strcasecmp(cmd, "specular") == 0) {
    input >> material.specular[0] >> material.specular[1] >>
             material.specular[2] >> material.specular[3];
  }
  else if (strcasecmp(cmd, "emission") == 0) {
    input >> material.emission[0] >> material.emission[1] >>
             material.emission[2] >> material.emission[3];
  }
  else if (strcasecmp(cmd, "shininess") == 0) {
    input >> material.shininess;
  }
  else {
    return false;
  }

  return true;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
