/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

#include "BzMaterial.h"
#include "TextureMatrix.h"
#include "DynamicColor.h"
#include "Pack.h"


/****************************************************************************/
//
// BzMaterialManager
//

BzMaterialManager MATERIALMGR;


BzMaterialManager::BzMaterialManager()
{
  return;
}


BzMaterialManager::~BzMaterialManager()
{
  clear();
  return;
}


void BzMaterialManager::clear()
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    delete materials[i];
  }
  materials.clear();
  return;
}


const BzMaterial* BzMaterialManager::addMaterial(const BzMaterial* material)
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    if (*material == *(materials[i])) {
      return materials[i];
    }
  }
  BzMaterial* newMat = new BzMaterial(*material);
  if (findMaterial(newMat->getName()) != NULL) {
    newMat->setName("");
  }
  materials.push_back(newMat);
  return newMat;
}


const BzMaterial* BzMaterialManager::findMaterial(const std::string& target) const
{
  if (target.size() <= 0) {
    return NULL;
  }
  else if ((target[0] >= '0') && (target[0] <= '9')) {
    int index = atoi (target.c_str());
    if ((index < 0) || (index >= (int)materials.size())) {
      return NULL;
    } else {
      return materials[index];
    }
  }
  else {
    for (unsigned int i = 0; i < materials.size(); i++) {
      if (target == materials[i]->getName()) {
	return materials[i];
      }
    }
    return NULL;
  }
}


const BzMaterial* BzMaterialManager::getMaterial(int id) const
{
  if ((id < 0) || (id >= (int)materials.size())) {
    return BzMaterial::getDefault();
  }
  return materials[id];
}


int BzMaterialManager::getIndex(const BzMaterial* material) const
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    if (material == materials[i]) {
      return i;
    }
  }
  return -1;
}


void* BzMaterialManager::pack(void* buf)
{
  buf = nboPackUInt(buf, (unsigned int)materials.size());
  for (unsigned int i = 0; i < materials.size(); i++) {
    buf = materials[i]->pack(buf);
  }

  return buf;
}


void* BzMaterialManager::unpack(void* buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    BzMaterial* mat = new BzMaterial;
    buf = mat->unpack(buf);
    materials.push_back(mat);
  }
  return buf;
}


int BzMaterialManager::packSize()
{
  int fullSize = sizeof (uint32_t);
  for (unsigned int i = 0; i < materials.size(); i++) {
    fullSize += materials[i]->packSize();
  }
  return fullSize;
}


void BzMaterialManager::print(std::ostream& out, const std::string& indent) const
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    materials[i]->print(out, indent);
  }
  return;
}


void BzMaterialManager::printReference(std::ostream& out,
				       const BzMaterial* mat) const
{
  if (mat == NULL) {
    out << "-1";
    return;
  }
  int index = getIndex(mat);
  if (index == -1) {
    out << "-1";
    return;
  }
  if (mat->getName().size() > 0) {
    out << mat->getName();
    return;
  } else {
    out << index;
    return;
  }
}


void BzMaterialManager::makeTextureList(TextureSet& set) const
{
  set.clear();
  for (unsigned int i = 0; i < materials.size(); i++) {
    const BzMaterial* mat = materials[i];
    for (int j = 0; j < mat->getTextureCount(); j++) {
      set.insert(mat->getTexture(j));
    }
  }
  return;
}


void BzMaterialManager::setTextureLocal(const std::string& url,
                                        const std::string& local)
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    BzMaterial* mat = materials[i];
    for (int j = 0; j < mat->getTextureCount(); j++) {
      if (mat->getTexture(j) == url) {
        mat->setTextureLocal(j, local);
      }
    }
  }
  return;
}


/****************************************************************************/
//
// BzMaterial
//

BzMaterial BzMaterial::defaultMaterial;
std::string BzMaterial::nullString = "";


void BzMaterial::reset()
{
  dynamicColor = -1;
  const float defAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  const float defDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  const float defSpecular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  const float defEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  memcpy (ambient, defAmbient, sizeof(ambient));
  memcpy (diffuse, defDiffuse, sizeof(diffuse));
  memcpy (specular, defSpecular, sizeof(specular));
  memcpy (emission, defEmission, sizeof(emission));
  shininess = 0.0f;
  alphaThreshold = 0.0f;
  noCulling = false;
  noSorting = false;

  delete[] textures;
  textures = NULL;
  textureCount = 0;

  delete[] shaders;
  shaders = NULL;
  shaderCount = 0;

  return;
}


BzMaterial::BzMaterial()
{
  textures = NULL;
  shaders = NULL;
  reset();
  return;
}


BzMaterial::~BzMaterial()
{
  delete[] textures;
  delete[] shaders;
  return;
}


BzMaterial::BzMaterial(const BzMaterial& m)
{
  textures = NULL;
  shaders = NULL;
  *this = m;
  return;
}


BzMaterial& BzMaterial::operator=(const BzMaterial& m)
{
  int i;

  name = m.name;

  dynamicColor = m.dynamicColor;
  memcpy (ambient, m.ambient, sizeof(ambient));
  memcpy (diffuse, m.diffuse, sizeof(diffuse));
  memcpy (specular, m.specular, sizeof(specular));
  memcpy (emission, m.emission, sizeof(emission));
  shininess = m.shininess;
  alphaThreshold = m.alphaThreshold;
  noCulling = m.noCulling;
  noSorting = m.noSorting;

  delete[] textures;
  textureCount = m.textureCount;
  if (textureCount > 0) {
    textures = new TextureInfo[textureCount];
  } else {
    textures = NULL;
  }
  for (i = 0; i < textureCount; i++) {
    textures[i] = m.textures[i];
  }

  delete[] shaders;
  shaderCount = m.shaderCount;
  if (shaderCount > 0) {
    shaders = new ShaderInfo[shaderCount];
  } else {
    shaders = NULL;
  }
  for (i = 0; i < shaderCount; i++) {
    shaders[i] = m.shaders[i];
  }

  return *this;
}


bool BzMaterial::operator==(const BzMaterial& m) const
{
  int i;

  if ((dynamicColor != m.dynamicColor) ||
      (memcmp (ambient, m.ambient, sizeof(float[4])) != 0) ||
      (memcmp (diffuse, m.diffuse, sizeof(float[4])) != 0) ||
      (memcmp (specular, m.specular, sizeof(float[4])) != 0) ||
      (memcmp (emission, m.emission, sizeof(float[4])) != 0) ||
      (shininess != m.shininess) ||
      (alphaThreshold != m.alphaThreshold) ||
      (noCulling != m.noCulling) || (noSorting != m.noSorting)) {
    return false;
  }

  if (textureCount != m.textureCount) {
    return false;
  }
  for (i = 0; i < textureCount; i++) {
    if ((textures[i].name != m.textures[i].name) ||
	(textures[i].matrix != m.textures[i].matrix) ||
	(textures[i].combineMode != m.textures[i].combineMode) ||
	(textures[i].useAlpha != m.textures[i].useAlpha) ||
	(textures[i].useColor != m.textures[i].useColor) ||
	(textures[i].useSphereMap != m.textures[i].useSphereMap)) {
      return false;
    }
  }

  if (shaderCount != m.shaderCount) {
    return false;
  }
  for (i = 0; i < shaderCount; i++) {
    if (shaders[i].name != m.shaders[i].name) {
      return false;
    }
  }

  return true;
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


void* BzMaterial::pack(void* buf) const
{
  int i;

  buf = nboPackStdString(buf, name);
  
  uint8_t modeByte = 0;
  if (noCulling) modeByte |= (1 << 0);
  if (noSorting) modeByte |= (1 << 1);
  buf = nboPackUByte(buf, modeByte);

  buf = nboPackInt(buf, dynamicColor);
  buf = pack4Float(buf, ambient);
  buf = pack4Float(buf, diffuse);
  buf = pack4Float(buf, specular);
  buf = pack4Float(buf, emission);
  buf = nboPackFloat(buf, shininess);
  buf = nboPackFloat(buf, alphaThreshold);

  buf = nboPackUByte(buf, textureCount);
  for (i = 0; i < textureCount; i++) {
    const TextureInfo* texinfo = &textures[i];

    buf = nboPackStdString(buf, texinfo->name);
    buf = nboPackInt(buf, texinfo->matrix);
    buf = nboPackInt(buf, texinfo->combineMode);
    unsigned char stateByte = 0;
    if (texinfo->useAlpha) {
      stateByte = stateByte | (1 << 0);
    }
    if (texinfo->useColor) {
      stateByte = stateByte | (1 << 1);
    }
    if (texinfo->useSphereMap) {
      stateByte = stateByte | (1 << 2);
    }
    buf = nboPackUByte(buf, stateByte);
  }

  buf = nboPackUByte(buf, shaderCount);
  for (i = 0; i < shaderCount; i++) {
    buf = nboPackStdString(buf, shaders[i].name);
  }

  return buf;
}


void* BzMaterial::unpack(void* buf)
{
  int i;
  int32_t inTmp;

  buf = nboUnpackStdString(buf, name);

  uint8_t modeByte;
  buf = nboUnpackUByte(buf, modeByte);
  noCulling = (modeByte & (1 << 0)) != 0;
  noSorting = (modeByte & (1 << 1)) != 0;

  buf = nboUnpackInt(buf, inTmp);
  dynamicColor = int(inTmp);
  buf = unpack4Float(buf, ambient);
  buf = unpack4Float(buf, diffuse);
  buf = unpack4Float(buf, specular);
  buf = unpack4Float(buf, emission);
  buf = nboUnpackFloat(buf, shininess);
  buf = nboUnpackFloat(buf, alphaThreshold);

  unsigned char tCount;
  buf = nboUnpackUByte(buf, tCount);
  textureCount = tCount;
  textures = new TextureInfo[textureCount];
  for (i = 0; i < textureCount; i++) {
    TextureInfo* texinfo = &textures[i];

    buf = nboUnpackStdString(buf, texinfo->name);
    buf = nboUnpackInt(buf, inTmp);
    texinfo->matrix = int(inTmp);
    buf = nboUnpackInt(buf, inTmp);
    texinfo->combineMode = int(inTmp);
    texinfo->useAlpha = false;
    texinfo->useColor = false;
    texinfo->useSphereMap = false;
    unsigned char stateByte;
    buf = nboUnpackUByte(buf, stateByte);
    if (stateByte & (1 << 0)) {
      texinfo->useAlpha = true;
    }
    if (stateByte & (1 << 1)) {
      texinfo->useColor = true;
    }
    if (stateByte & (1 << 2)) {
      texinfo->useSphereMap = true;
    }
  }

  unsigned char sCount;
  buf = nboUnpackUByte(buf, sCount);
  shaderCount = sCount;
  shaders = new ShaderInfo[shaderCount];
  for (i = 0; i < shaderCount; i++) {
    buf = nboUnpackStdString(buf, shaders[i].name);
  }

  return buf;
}


int BzMaterial::packSize() const
{
  int i;
  
  const int modeSize = sizeof(uint8_t);

  const int colorSize = sizeof(int32_t) + (4 * sizeof(float[4])) +
                        sizeof(float) + sizeof(float);
  
  int textureSize = sizeof(unsigned char);
  for (i = 0; i < textureCount; i++) {
    textureSize += nboStdStringPackSize(textures[i].name);
    textureSize += sizeof(int32_t);
    textureSize += sizeof(int32_t);
    textureSize += sizeof(unsigned char);
  }

  int shaderSize = sizeof(unsigned char);
  for (i = 0; i < shaderCount; i++) {
    shaderSize += nboStdStringPackSize(shaders[i].name);
  }

  return nboStdStringPackSize(name) + modeSize + colorSize + textureSize + shaderSize;
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


void BzMaterial::print(std::ostream& out, const std::string& /*indent*/) const
{
  int i;

  out << "material" << std::endl;

  if (name.size() > 0) {
    out << "  name " << name << std::endl;
  }

  if (dynamicColor != defaultMaterial.dynamicColor) {
    out << "  dyncol ";
    const DynamicColor* dyncol = DYNCOLORMGR.getColor(dynamicColor);
    if ((dyncol != NULL) && (dyncol->getName().size() > 0)) {
      out << dyncol->getName();
    } else {
      out << dynamicColor;
    }
    out << std::endl;
  }
  printColor(out, "  ambient ",  ambient,  defaultMaterial.ambient);
  printColor(out, "  diffuse ",  diffuse,  defaultMaterial.diffuse);
  printColor(out, "  specular ", specular, defaultMaterial.specular);
  printColor(out, "  emission ", emission, defaultMaterial.emission);
  if (shininess != defaultMaterial.shininess) {
    out << "  shininess " << shininess << std::endl;
  }
  if (alphaThreshold != defaultMaterial.alphaThreshold) {
    out << "  alphathresh " << alphaThreshold << std::endl;
  }
  if (noCulling) {
    out << "  noculling" << std::endl;
  }
  if (noSorting) {
    out << "  nosorting" << std::endl;
  }

  for (i = 0; i < textureCount; i++) {
    const TextureInfo* texinfo = &textures[i];
    out << "  addtexture " << texinfo->name << std::endl;
    if (texinfo->matrix != -1) {
      out << "    texmat ";
      const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texinfo->matrix);
      if ((texmat != NULL) && (texmat->getName().size() > 0)) {
	out << texmat->getName();
      } else {
	out << texinfo->matrix;
      }
      out << std::endl;
    }

    if (!texinfo->useAlpha) {
      out << "    notexalpha" << std::endl;
    }
    if (!texinfo->useColor) {
      out << "    notexcolor" << std::endl;
    }
    if (texinfo->useSphereMap) {
      out << "    spheremap" << std::endl;
    }
  }

  for (i = 0; i < shaderCount; i++) {
    const ShaderInfo* shdinfo = &shaders[i];
    out << "  addshader " << shdinfo->name << std::endl;
  }

  out << "end" << std::endl << std::endl;

  return;
}


/****************************************************************************/
//
// Parameter setting
//

bool BzMaterial::setName(const std::string& matname)
{
  if (matname.size() <= 0) {
    name = "";
    return false;
  }
  else if ((matname[0] >= '0') && (matname[0] <= '9')) {
    name = "";
    return false;
  }
  else {
    name = matname;
  }
  return true;
}

void BzMaterial::setDynamicColor(int dyncol)
{
  dynamicColor = dyncol;
  return;
}

void BzMaterial::setAmbient(const float color[4])
{
  memcpy (ambient, color, sizeof(float[4]));
  return;
}

void BzMaterial::setDiffuse(const float color[4])
{
  memcpy (diffuse, color, sizeof(float[4]));
  return;
}

void BzMaterial::setSpecular(const float color[4])
{
  memcpy (specular, color, sizeof(float[4]));
  return;
}

void BzMaterial::setEmission(const float color[4])
{
  memcpy (emission, color, sizeof(float[4]));
  return;
}

void BzMaterial::setShininess(const float shine)
{
  shininess = shine;
  return;
}

void BzMaterial::setAlphaThreshold(const float thresh)
{
  alphaThreshold = thresh;
  return;
}

void BzMaterial::setNoCulling(bool value)
{
  noCulling = value;
  return;
}

void BzMaterial::setNoSorting(bool value)
{
  noSorting = value;
  return;
}


void BzMaterial::addTexture(const std::string& texname)
{
  textureCount++;
  TextureInfo* tmpinfo = new TextureInfo[textureCount];
  for (int i = 0; i < (textureCount - 1); i++) {
    tmpinfo[i] = textures[i];
  }
  delete[] textures;
  textures = tmpinfo;

  TextureInfo* texinfo = &textures[textureCount - 1];
  texinfo->name = texname;
  texinfo->localname = texname;
  texinfo->matrix = -1;
  texinfo->combineMode = decal;
  texinfo->useAlpha = true;
  texinfo->useColor = true;
  texinfo->useSphereMap = false;

  return;
}

void BzMaterial::setTexture(const std::string& texname)
{
  if (textureCount <= 0) {
    addTexture(texname);
  } else {
    textures[textureCount - 1].name = texname;
  }

  return;
}

void BzMaterial::setTextureLocal(int texid, const std::string& localname)
{
  if ((texid >= 0) && (texid < textureCount)) {
    textures[texid].localname = localname;
  }
  return;
}

void BzMaterial::setTextureMatrix(int matrix)
{
  if (textureCount > 0) {
    textures[textureCount - 1].matrix = matrix;
  }
  return;
}

void BzMaterial::setCombineMode(int mode)
{
  if (textureCount > 0) {
    textures[textureCount - 1].combineMode = mode;
  }
  return;
}

void BzMaterial::setUseTextureAlpha(bool value)
{
  if (textureCount > 0) {
    textures[textureCount - 1].useAlpha = value;
  }
  return;
}

void BzMaterial::setUseColorOnTexture(bool value)
{
  if (textureCount > 0) {
    textures[textureCount - 1].useColor = value;
  }
  return;
}

void BzMaterial::setUseSphereMap(bool value)
{
  if (textureCount > 0) {
    textures[textureCount - 1].useSphereMap = value;
  }
  return;
}


void BzMaterial::clearTextures()
{
  delete[] textures;
  textures = NULL;
  textureCount = 0;
  return;
}


void BzMaterial::setShader(const std::string& shadername)
{
  if (shaderCount <= 0) {
    addShader(shadername);
  } else {
    shaders[shaderCount - 1].name = shadername;
  }

  return;
}

void BzMaterial::addShader(const std::string& shaderName)
{
  shaderCount++;
  ShaderInfo* tmpinfo = new ShaderInfo[shaderCount];
  for (int i = 0; i < (shaderCount - 1); i++) {
    tmpinfo[i] = shaders[i];
  }
  delete[] shaders;
  shaders = tmpinfo;
  shaders[shaderCount - 1].name = shaderName;
  return;
}


void BzMaterial::clearShaders()
{
  delete[] shaders;
  shaders = NULL;
  shaderCount = 0;
  return;
}


/****************************************************************************/
//
// Parameter retrieval
//

const std::string& BzMaterial::getName() const
{
  return name;
}

int BzMaterial::getDynamicColor() const
{
  return dynamicColor;
}

const float* BzMaterial::getAmbient() const
{
  return ambient;
}

const float* BzMaterial::getDiffuse() const
{
  return diffuse;
}

const float* BzMaterial::getSpecular() const
{
  return specular;
}

const float* BzMaterial::getEmission() const
{
  return emission;
}

float BzMaterial::getShininess() const
{
  return shininess;
}

float BzMaterial::getAlphaThreshold() const
{
  return alphaThreshold;
}

bool BzMaterial::getNoCulling() const
{
  return noCulling;
}

bool BzMaterial::getNoSorting() const
{
  return noSorting;
}


int BzMaterial::getTextureCount() const
{
  return textureCount;
}

const std::string& BzMaterial::getTexture(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].name;
  } else {
    return nullString;
  }
}

const std::string& BzMaterial::getTextureLocal(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].localname;
  } else {
    return nullString;
  }
}

int BzMaterial::getTextureMatrix(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].matrix;
  } else {
    return -1;
  }
}

int BzMaterial::getCombineMode(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].combineMode;
  } else {
    return -1;
  }
}

bool BzMaterial::getUseTextureAlpha(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].useAlpha;
  } else {
    return false;
  }
}

bool BzMaterial::getUseColorOnTexture(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].useColor;
  } else {
    return false;
  }
}

bool BzMaterial::getUseSphereMap(int texid) const
{
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].useSphereMap;
  } else {
    return false;
  }
}


int BzMaterial::getShaderCount() const
{
  return shaderCount;
}

const std::string& BzMaterial::getShader(int shdid) const
{
  if ((shdid >= 0) && (shdid < shaderCount)) {
    return shaders[shdid].name;
  } else {
    return nullString;
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

