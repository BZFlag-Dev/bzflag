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

#include <string.h>

#include "MeshMaterial.h"
#include "Pack.h"


MeshMaterial MeshMaterial::defaultMaterial;


void MeshMaterial::reset()
{
  texture = "";
  textureMatrix = -1;
  dynamicColor = -1;
  const float defAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  const float defDiffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
  const float defSpecular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  const float defEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  memcpy (ambient, defAmbient, sizeof(ambient));
  memcpy (diffuse, defDiffuse, sizeof(diffuse));
  memcpy (specular, defSpecular, sizeof(specular));
  memcpy (emission, defEmission, sizeof(emission));
  shininess = 0.0f;
  useTexture = true;
  useTextureAlpha = true;
  useColorOnTexture = true;
  return;
}


MeshMaterial::MeshMaterial()
{
  reset();
  return;
}


MeshMaterial::MeshMaterial(const MeshMaterial& m)
{
  texture = m.texture;
  textureMatrix = m.textureMatrix;
  dynamicColor = m.dynamicColor;
  memcpy (ambient, m.ambient, sizeof(ambient));
  memcpy (diffuse, m.diffuse, sizeof(diffuse));
  memcpy (specular, m.specular, sizeof(specular));
  memcpy (emission, m.emission, sizeof(emission));
  shininess = m.shininess;
  useTexture = m.useTexture;
  useTextureAlpha = m.useTextureAlpha;
  useColorOnTexture = m.useColorOnTexture;
  return;
}


MeshMaterial& MeshMaterial::operator=(const MeshMaterial& m)
{
  texture = m.texture;
  textureMatrix = m.textureMatrix;
  dynamicColor = m.dynamicColor;
  memcpy (ambient, m.ambient, sizeof(ambient));
  memcpy (diffuse, m.diffuse, sizeof(diffuse));
  memcpy (specular, m.specular, sizeof(specular));
  memcpy (emission, m.emission, sizeof(emission));
  shininess = m.shininess;
  useTexture = m.useTexture;
  useTextureAlpha = m.useTextureAlpha;
  useColorOnTexture = m.useColorOnTexture;
  return *this;
}


bool MeshMaterial::operator==(const MeshMaterial& m)
{
  if ((texture != m.texture) ||
      (textureMatrix != m.textureMatrix) ||
      (dynamicColor != m.dynamicColor) ||
      (shininess != m.shininess) ||
      (memcmp (ambient, m.ambient, sizeof(float[4])) != 0) ||
      (memcmp (diffuse, m.diffuse, sizeof(float[4])) != 0) ||
      (memcmp (specular, m.specular, sizeof(float[4])) != 0) ||
      (memcmp (emission, m.emission, sizeof(float[4])) != 0) ||
      (useTexture != m.useTexture) ||
      (useTextureAlpha != m.useTextureAlpha) ||
      (useColorOnTexture != m.useColorOnTexture)) {
    return false;
  }
  return true;
}


bool MeshMaterial::copyDiffs(const MeshMaterial& moded,
                             const MeshMaterial& orig)
{
  bool changed = false;

  if (orig.texture != moded.texture) {
    texture = moded.texture;
    changed = true;
  }
  if (orig.textureMatrix != moded.textureMatrix) {
    textureMatrix = moded.textureMatrix;
    changed = true;
  }
  if (orig.dynamicColor != moded.dynamicColor) {
    dynamicColor = moded.dynamicColor;
    changed = true;
  }
  if (orig.shininess != moded.shininess) {
    shininess = moded.shininess;
    changed = true;
  }
  if (memcmp (orig.ambient, moded.ambient, sizeof(float[4])) != 0) {
    memcpy (ambient, moded.ambient, sizeof(float[4]));
    changed = true;
  }
  if (memcmp (orig.diffuse, moded.diffuse, sizeof(float[4])) != 0) {
    memcpy (diffuse, moded.diffuse, sizeof(float[4]));
    changed = true;
  }
  if (memcmp (orig.specular, moded.specular, sizeof(float[4])) != 0) {
    memcpy (specular, moded.specular, sizeof(float[4]));
    changed = true;
  }
  if (memcmp (orig.emission, moded.emission, sizeof(float[4])) != 0) {
    memcpy (emission, moded.emission, sizeof(float[4]));
    changed = true;
  }
  if (orig.useTexture != moded.useTexture) {
    useTexture = moded.useTexture;
    changed = true;
  }
  if (orig.useTextureAlpha != moded.useTextureAlpha) {
    useTextureAlpha = moded.useTextureAlpha;
    changed = true;
  }
  if (orig.useColorOnTexture != moded.useColorOnTexture) {
    useColorOnTexture = moded.useColorOnTexture;
    changed = true;
  }

  return changed;
}


static void* pack4Float(void *buf, const float values[4])
{
  int i;
  for (i = 0; i < 4; i++) {
    buf = nboPackFloat(buf, values[i]);
  }
  return buf;
}


static void* unpack4Float(void *buf, float values[4])
{
  int i;
  for (i = 0; i < 4; i++) {
    buf = nboUnpackFloat(buf, values[i]);
  }
  return buf;
}


void* MeshMaterial::pack(void* buf)
{
  unsigned char stateByte = 0;
  if (useTexture) {
    stateByte = stateByte | (1 << 0);
  }
  if (useTextureAlpha) {
    stateByte = stateByte | (1 << 1);
  }
  if (useColorOnTexture) {
    stateByte = stateByte | (1 << 2);
  }
  buf = nboPackUByte(buf, stateByte);
  unsigned char len = (unsigned char)texture.size();
  buf = nboPackUByte(buf, len);
  buf = nboPackString(buf, texture.c_str(), len);
  buf = nboPackInt(buf, textureMatrix);
  buf = nboPackInt(buf, dynamicColor);
  buf = pack4Float(buf, ambient);
  buf = pack4Float(buf, diffuse);
  buf = pack4Float(buf, specular);
  buf = pack4Float(buf, emission);
  buf = nboPackFloat(buf, shininess);
  return buf;
}


void* MeshMaterial::unpack(void* buf)
{
  unsigned char stateByte;
  buf = nboUnpackUByte(buf, stateByte);
  useTexture = useTextureAlpha = useColorOnTexture = false;
  if (stateByte & (1 << 0)) {
    useTexture = true;
  }
  if (stateByte & (1 << 1)) {
    useTextureAlpha = true;
  }
  if (stateByte & (1 << 2)) {
    useColorOnTexture = true;
  }
  char textureStr[256];
  unsigned char len;
  buf = nboUnpackUByte(buf, len);
  buf = nboUnpackString(buf, textureStr, len);
  textureStr[len] = '\0';
  texture = textureStr;
  buf = nboUnpackInt(buf, textureMatrix);
  buf = nboUnpackInt(buf, dynamicColor);
  buf = unpack4Float(buf, ambient);
  buf = unpack4Float(buf, diffuse);
  buf = unpack4Float(buf, specular);
  buf = unpack4Float(buf, emission);
  buf = nboUnpackFloat(buf, shininess);
  return buf;
}


int MeshMaterial::packSize()
{
  const int basicSize = (2 * sizeof(unsigned char)) +
                        sizeof(int) + sizeof(int) +
                        (4 * sizeof(float[4])) + sizeof(float);
  unsigned char len = (unsigned char)texture.size();
  return basicSize + len;
}


static void printColor(std::ostream& out, const char *name,
                       const float color[4], const float reference[4])
{
  if (memcmp(color, reference, sizeof(float[4])) != 0) {
    out << name << color[0] << " " << color[1] << " " 
                << color[2] << " " << color[3] << std::endl;
  }
  return;
}
                       

void MeshMaterial::print(std::ostream& out, int /*level*/)
{
  if (texture.size() > 0) {
    out << "    texture " << texture << std::endl;
  }
  if (textureMatrix != getDefault().textureMatrix) {
    out << "    texmat " << textureMatrix << std::endl;
  }
  if (!useTexture) {
    out << "    notexture" << std::endl;
  }
  if (!useTextureAlpha) {
    out << "    notexalpha" << std::endl;
  }
  if (!useColorOnTexture) {
    out << "    notexcolor" << std::endl;
  }
  if (dynamicColor != getDefault().dynamicColor) {
    out << "    dyncol " << dynamicColor << std::endl;
  }

  printColor(out, "    ambient ",  ambient,  getDefault().ambient);
  printColor(out, "    diffuse ",  diffuse,  getDefault().diffuse);
  printColor(out, "    specular ", specular, getDefault().specular);
  printColor(out, "    emission ", emission, getDefault().emission);

  if (shininess != getDefault().shininess) {
    out << "    shininess " << shininess << std::endl;
  }
  
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

