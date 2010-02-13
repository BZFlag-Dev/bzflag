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

#include "common.h"

// implementation header
#include "BzMaterial.h"

// system headers
#include <string.h>
#include <string>
#include <map>

// common headers
#include "bzfio.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "BZDBCache.h"
#include "CacheManager.h"
#include "TextUtils.h"
#include "Pack.h"


//============================================================================//
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
  nameMap.clear();
  return;
}


const BzMaterial* BzMaterialManager::addMaterial(const BzMaterial* material)
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    if (*material == *(materials[i])) {
      const std::string& name = material->getName();
      if (name.size() > 0) {
	materials[i]->addAlias(name);
      }
      return materials[i];
    }
  }
  BzMaterial* newMat = new BzMaterial(*material);
  if (findMaterial(newMat->getName()) != NULL) {
    newMat->setName("");
  }
  newMat->setID((int)materials.size());
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
//FIXME    NameMap::const_iterator it = nameMap.find(target);
//    if (it != nameMap.end()) {
//      return it->second;
//    }
    for (unsigned int i = 0; i < materials.size(); i++) {
      const BzMaterial* mat = materials[i];
      // check the base name
      if (target == mat->getName()) {
	return mat;
      }
      // check the aliases
      const std::vector<std::string>& aliases = mat->getAliases();
      for (unsigned int j = 0; j < aliases.size(); j++) {
	if (target == aliases[j]) {
	  return mat;
	}
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
  buf = nboPackUInt32(buf, (unsigned int)materials.size());
  for (unsigned int i = 0; i < materials.size(); i++) {
    buf = materials[i]->pack(buf);
  }

  return buf;
}


void* BzMaterialManager::unpack(void* buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt32 (buf, count);
  for (i = 0; i < count; i++) {
    BzMaterial* mat = new BzMaterial;
    buf = mat->unpack(buf);
    materials.push_back(mat);
    mat->setID(i);
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


void BzMaterialManager::printMTL(std::ostream& out, const std::string& indent) const
{
  for (unsigned int i = 0; i < materials.size(); i++) {
    materials[i]->printMTL(out, indent);
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


void BzMaterialManager::makeTextureList(TextureSet& set, bool referenced) const
{
  set.clear();
  for (unsigned int i = 0; i < materials.size(); i++) {
    const BzMaterial* mat = materials[i];
    for (int j = 0; j < mat->getTextureCount(); j++) {
      if (mat->getReference() || !referenced) {
	set.insert(mat->getTexture(j));
      }
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


//============================================================================//
//
// BzMaterial
//

BzMaterial BzMaterial::defaultMaterial;
const std::string BzMaterial::nullString = "";


std::string BzMaterial::convertTexture(const std::string& oldTex)
{
  static std::map<std::string, std::string> convMap;

  if (convMap.empty()) {
    const char* colors[] = {
      "rogue", "red", "green", "blue", "purple", "rabbit", "hunter", "observer"
    };
    const char* types[] = {
      "tank", "icon", "bolt", "super_bolt", "laser", "basewall", "basetop"
    };
    const int colorCount = countof(colors);
    const int typeCount  = countof(types);
    for (int c = 0; c < colorCount; c++) {
      for (int t = 0; t < typeCount; t++) {
        std::string oldName, newName;

        oldName += colors[c];
        oldName += "_";
        oldName += types[t];

        newName += "skins/";
        newName += colors[c];
        newName += "/";
        newName += types[t];
        newName += ".png";

        convMap[oldName] = newName;
        logDebugMessage(6, "TEXTURE MAP:  %-23s  =>  %s\n",
                           oldName.c_str(), newName.c_str());
        oldName += ".png";
        convMap[oldName] = newName;
        logDebugMessage(6, "TEXTURE MAP:  %-23s  =>  %s\n",
                           oldName.c_str(), newName.c_str());
      }
    }
  }

  std::map<std::string, std::string>::const_iterator it = convMap.find(oldTex);
  if (it != convMap.end()) {
    logDebugMessage(0, "WARNING: converted texture '%s' to '%s'\n",
                       oldTex.c_str(), it->second.c_str());
    return it->second;
  }

  return oldTex;
}


void BzMaterial::reset()
{
  order = 0;
  dynamicColor = -1;

  ambient  = fvec4(0.2f, 0.2f, 0.2f, 1.0f);
  diffuse  = fvec4(1.0f, 1.0f, 1.0f, 1.0f);
  specular = fvec4(0.0f, 0.0f, 0.0f, 1.0f);
  emission = fvec4(0.0f, 0.0f, 0.0f, 1.0f);
  shininess = 0.0f;

  alphaThreshold = 0.0f;

  poFactor = 0.0f;
  poUnits  = 0.0f;

  occluder       = false;
  groupAlpha     = false;
  noRadar        = false;
  noRadarOutline = false;
  noShadowCast   = false;
  noShadowRecv   = false;
  texShadow      = false;
  noCulling      = false;
  noSorting      = false;
  noBlending     = false;
  noLighting     = false;
  radarSpecial   = false;
  delete[] textures;
  textures = NULL;
  textureCount = 0;

  delete[] shaders;
  shaders = NULL;
  shaderCount = 0;

  referenced = false;

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

  referenced = false;

  name = m.name;
  aliases = m.aliases;

  order = m.order;
  dynamicColor = m.dynamicColor;
  memcpy (ambient, m.ambient, sizeof(ambient));
  memcpy (diffuse, m.diffuse, sizeof(diffuse));
  memcpy (specular, m.specular, sizeof(specular));
  memcpy (emission, m.emission, sizeof(emission));
  shininess = m.shininess;
  alphaThreshold = m.alphaThreshold;
  poFactor = m.poFactor;
  poUnits  = m.poUnits;
  occluder = m.occluder;
  groupAlpha = m.groupAlpha;
  noRadar = m.noRadar;
  noRadarOutline = m.noRadarOutline;
  noShadowCast = m.noShadowCast;
  noShadowRecv = m.noShadowRecv;
  texShadow = m.texShadow;
  noCulling = m.noCulling;
  noSorting = m.noSorting;
  noBlending = m.noBlending;
  noLighting = m.noLighting;
  radarSpecial = m.radarSpecial;

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

  if ((order != m.order) ||
      (dynamicColor != m.dynamicColor) ||
      (ambient  != m.ambient)  || (diffuse  != m.diffuse)  ||
      (specular != m.specular) || (emission != m.emission) ||
      (shininess != m.shininess) || (alphaThreshold != m.alphaThreshold) ||
      (poFactor != m.poFactor) || (poUnits != m.poUnits) ||
      (occluder != m.occluder) || (groupAlpha != m.groupAlpha) ||
      (noRadar != m.noRadar) || (noRadarOutline != m.noRadarOutline) ||
      (noShadowCast != m.noShadowCast) || (noShadowRecv != m.noShadowRecv) ||
      (texShadow != m.texShadow) ||
      (noCulling != m.noCulling) || (noSorting != m.noSorting) ||
      (noBlending != m.noBlending) || (noLighting != m.noLighting) ||
      (radarSpecial != m.radarSpecial)) {
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
	(textures[i].useSphereMap != m.textures[i].useSphereMap) ||
	(textures[i].autoScale != m.textures[i].autoScale)) {
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


void* BzMaterial::pack(void* buf) const
{
  int i;

  buf = nboPackStdString(buf, name);
  buf = nboPackInt32(buf, (int32_t)aliases.size());
  for (size_t a = 0; a < aliases.size(); a++) {
    buf = nboPackStdString(buf, aliases[a]);
  }

  uint16_t modeBytes = 0;
  if (noCulling)      { modeBytes |= (1 << 0); }
  if (noSorting)      { modeBytes |= (1 << 1); }
  if (noRadar)        { modeBytes |= (1 << 2); }
  if (noRadarOutline) { modeBytes |= (1 << 3); }
  if (noShadowCast)   { modeBytes |= (1 << 4); }
  if (noShadowRecv)   { modeBytes |= (1 << 5); }
  if (texShadow)      { modeBytes |= (1 << 6); }
  if (occluder)       { modeBytes |= (1 << 7); }
  if (groupAlpha)     { modeBytes |= (1 << 8); }
  if (noLighting)     { modeBytes |= (1 << 9); }
  if (noBlending)     { modeBytes |= (1 << 10); }
  if (radarSpecial)   { modeBytes |= (1 << 11); }
  buf = nboPackUInt16(buf, modeBytes);

  buf = nboPackInt32(buf, order);
  buf = nboPackInt32(buf, dynamicColor);
  buf = nboPackFVec4(buf, ambient);
  buf = nboPackFVec4(buf, diffuse);
  buf = nboPackFVec4(buf, specular);
  buf = nboPackFVec4(buf, emission);
  buf = nboPackFloat(buf, shininess);
  buf = nboPackFloat(buf, alphaThreshold);
  buf = nboPackFloat(buf, poFactor);
  buf = nboPackFloat(buf, poUnits);

  buf = nboPackUInt8(buf, textureCount);
  for (i = 0; i < textureCount; i++) {
    const TextureInfo* texinfo = &textures[i];

    buf = nboPackStdString(buf, texinfo->name);
    buf = nboPackInt32(buf, texinfo->matrix);
    buf = nboPackInt32(buf, texinfo->combineMode);
    buf = nboPackFVec2(buf, texinfo->autoScale);
    unsigned char stateByte = 0;
    if (texinfo->useAlpha)     { stateByte = stateByte | (1 << 0); }
    if (texinfo->useColor)     { stateByte = stateByte | (1 << 1); }
    if (texinfo->useSphereMap) { stateByte = stateByte | (1 << 2); }
    buf = nboPackUInt8(buf, stateByte);
  }

  buf = nboPackUInt8(buf, shaderCount);
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
  buf = nboUnpackInt32(buf, inTmp);
  for (i = 0; i < inTmp; i++) {
    std::string alias;
    buf = nboUnpackStdString(buf, alias);
    aliases.push_back(alias);
  }

  uint16_t modeBytes;
  buf = nboUnpackUInt16(buf, modeBytes);
  noCulling      = (modeBytes & (1 << 0)) != 0;
  noSorting      = (modeBytes & (1 << 1)) != 0;
  noRadar        = (modeBytes & (1 << 2)) != 0;
  noRadarOutline = (modeBytes & (1 << 3)) != 0;
  noShadowCast   = (modeBytes & (1 << 4)) != 0;
  noShadowRecv   = (modeBytes & (1 << 5)) != 0;
  texShadow      = (modeBytes & (1 << 6)) != 0;
  occluder       = (modeBytes & (1 << 7)) != 0;
  groupAlpha     = (modeBytes & (1 << 8)) != 0;
  noLighting     = (modeBytes & (1 << 9)) != 0;
  noBlending     = (modeBytes & (1 << 10)) != 0;
  radarSpecial   = (modeBytes & (1 << 11)) != 0;

  buf = nboUnpackInt32(buf, order);
  buf = nboUnpackInt32(buf, inTmp); dynamicColor = int(inTmp);
  buf = nboUnpackFVec4(buf, ambient);
  buf = nboUnpackFVec4(buf, diffuse);
  buf = nboUnpackFVec4(buf, specular);
  buf = nboUnpackFVec4(buf, emission);
  buf = nboUnpackFloat(buf, shininess);
  buf = nboUnpackFloat(buf, alphaThreshold);
  buf = nboUnpackFloat(buf, poFactor);
  buf = nboUnpackFloat(buf, poUnits);

  unsigned char tCount;
  buf = nboUnpackUInt8(buf, tCount);
  textureCount = tCount;
  textures = new TextureInfo[textureCount];
  for (i = 0; i < textureCount; i++) {
    TextureInfo* texinfo = &textures[i];
    buf = nboUnpackStdString(buf, texinfo->name);
    texinfo->localname = texinfo->name;
    buf = nboUnpackInt32(buf, inTmp);
    texinfo->matrix = int(inTmp);
    buf = nboUnpackInt32(buf, inTmp);
    texinfo->combineMode = int(inTmp);
    buf = nboUnpackFVec2(buf, texinfo->autoScale);
    texinfo->useAlpha = false;
    texinfo->useColor = false;
    texinfo->useSphereMap = false;
    unsigned char stateByte;
    buf = nboUnpackUInt8(buf, stateByte);
    if (stateByte & (1 << 0)) { texinfo->useAlpha     = true; }
    if (stateByte & (1 << 1)) { texinfo->useColor     = true; }
    if (stateByte & (1 << 2)) { texinfo->useSphereMap = true; }
  }

  unsigned char sCount;
  buf = nboUnpackUInt8(buf, sCount);
  shaderCount = sCount;
  shaders = new ShaderInfo[shaderCount];
  for (i = 0; i < shaderCount; i++) {
    buf = nboUnpackStdString(buf, shaders[i].name);
  }

  return buf;
}


int BzMaterial::packSize() const
{
  int fullSize = 0;

  fullSize += nboStdStringPackSize(name); // name
  fullSize += sizeof(int32_t);            // aliases count
  for (size_t a = 0; a < aliases.size(); a++) {
    fullSize += nboStdStringPackSize(aliases[a]);
  }

  fullSize += sizeof(uint16_t); // modeBytes

  fullSize += sizeof(int32_t); // order
  fullSize += sizeof(int32_t); // dynamicColor
  fullSize += sizeof(fvec4);   // ambient
  fullSize += sizeof(fvec4);   // diffuse
  fullSize += sizeof(fvec4);   // specular
  fullSize += sizeof(fvec4);   // emission
  fullSize += sizeof(float);   // shininess
  fullSize += sizeof(float);   // alphaThreshold
  fullSize += sizeof(float);   // poFactor
  fullSize += sizeof(float);   // poUnits
  
  fullSize += sizeof(uint8_t); // texture count
  for (int i = 0; i < textureCount; i++) {
    fullSize += nboStdStringPackSize(textures[i].name);
    fullSize += sizeof(int32_t); // matrix
    fullSize += sizeof(int32_t); // combineMode
    fullSize += sizeof(fvec2);   // autoScale
    fullSize += sizeof(uint8_t); // stateByte
  }

  fullSize += sizeof(uint8_t); // shader count
  for (int i = 0; i < shaderCount; i++) {
    fullSize += nboStdStringPackSize(shaders[i].name);
  }

  return fullSize;
}


static void printColor(std::ostream& out, const char *name,
		       const fvec4& color, const fvec4& reference)
{
  if (color != reference) {
    out << name << color.r << " " << color.g << " "
		<< color.b << " " << color.a << std::endl;
  }
  return;
}


void BzMaterial::print(std::ostream& out, const std::string& indent) const
{
  int i;

  out << indent << "material # " << id << std::endl;

  if (!name.empty()) {
    out << indent << "  name " << name << std::endl;
  }
  for (size_t a = 0; a < aliases.size(); a++) {
    if (!aliases[a].empty()) {
      out << indent << "  # alias " << aliases[a] << std::endl;
    }
  }

  if (order != defaultMaterial.order) {
    out << indent << "  order " << order << std::endl;
  }

  if (dynamicColor != defaultMaterial.dynamicColor) {
    out << indent << "  dyncol ";
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
    out << indent << "  shininess " << shininess << std::endl;
  }
  if (alphaThreshold != defaultMaterial.alphaThreshold) {
    out << indent << "  alphathresh " << alphaThreshold << std::endl;
  }
  if ((poFactor != 0.0f) || (poUnits != 0.0f)) {
    out << indent << "  depthoffset " << poFactor << " "
                                      << poUnits << std::endl;
  }
  if (occluder) {
    out << indent << "  occluder" << std::endl;
  }
  if (groupAlpha) {
    out << indent << "  groupAlpha" << std::endl;
  }
  if (noRadar) {
    out << indent << "  noradar" << std::endl;
  }
  if (noRadarOutline) {
    out << indent << "  noRadarOutline" << std::endl;
  }
  if (noShadowCast) {
    out << indent << "  noshadowcast" << std::endl;
  }
  if (noShadowRecv) {
    out << indent << "  noshadowrecv" << std::endl;
  }
  if (texShadow) {
    out << indent << "  texshadow" << std::endl;
  }
  if (noCulling) {
    out << indent << "  noculling" << std::endl;
  }
  if (noSorting) {
    out << indent << "  nosorting" << std::endl;
  }
  if (noBlending) {
    out << indent << "  noblending" << std::endl;
  }
  if (noLighting) {
    out << indent << "  nolighting" << std::endl;
  }
  if (radarSpecial) {
    out << indent << "  radarSpecial" << std::endl;
  }

  for (i = 0; i < textureCount; i++) {
    const TextureInfo* texinfo = &textures[i];
    out << indent << "  addtexture " << texinfo->name << std::endl;
    if (texinfo->matrix != -1) {
      out << indent << "    texmat ";
      const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texinfo->matrix);
      if ((texmat != NULL) && (texmat->getName().size() > 0)) {
	out << texmat->getName();
      } else {
	out << texinfo->matrix;
      }
      out << std::endl;
    }

    if (!texinfo->useAlpha) {
      out << indent << "    notexalpha" << std::endl;
    }
    if (!texinfo->useColor) {
      out << indent << "    notexcolor" << std::endl;
    }
    if (texinfo->useSphereMap) {
      out << indent << "    spheremap" << std::endl;
    }
    if ((texinfo->autoScale.x != 0.0f) || (texinfo->autoScale.y != 0.0f)) {
      out << indent << "    texautoscale " << texinfo->autoScale << std::endl;
    }
  }

  for (i = 0; i < shaderCount; i++) {
    const ShaderInfo* shdinfo = &shaders[i];
    out << indent << "  addshader " << shdinfo->name << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


void BzMaterial::printMTL(std::ostream& out, const std::string& /*indent*/) const
{
  out << "newmtl ";
  if (name.size() > 0) {
    out << name << std::endl;
  } else {
    out << MATERIALMGR.getIndex(this) << std::endl;
  }
  if (noLighting) {
    out << "illum 0" << std::endl;
  } else {
    out << "illum 2" << std::endl;
  }
  out << "d "   << diffuse.a << std::endl;
  out << "#Ka " << ambient.rgb().tostring()  << std::endl; // not really used
  out << "Kd "  << diffuse.rgb().tostring()  << std::endl;
  out << "Ke "  << emission.rgb().tostring() << std::endl;
  out << "Ks "  << specular.rgb().tostring() << std::endl;
  out << "Ns "  << (1000.0f * (shininess / 128.0f)) << std::endl;
  if (textureCount > 0) {
    std::string texname = textures[0].name;
    const std::string pngExt = ".png";
    if (!texname.empty() && !CacheManager::isCacheFileType(texname)) {
      if (texname.size() < pngExt.size()) {
        texname += pngExt;
      }
      else {
        std::string ext = texname.substr(texname.size() - pngExt.size());
        ext = TextUtils::tolower(ext);
        if (ext != pngExt) {
          texname += pngExt;
        }
      }
      static BZDB_string prefix("objTexturePrefix");
      out << "map_Kd " << ((std::string)prefix + texname) << std::endl;
    }
  }
  out << std::endl;
  return;
}


//============================================================================//
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

bool BzMaterial::addAlias(const std::string& alias)
{
  if (alias.size() <= 0) {
    name = "";
    return false;
  }
  else if ((alias[0] >= '0') && (alias[0] <= '9')) {
    name = "";
    return false;
  }
  else {
    for ( unsigned int i = 0; i < (unsigned int)aliases.size(); i++)
    {
      if ( aliases[i] == alias )
	return true;
    }
    aliases.push_back(alias); // only add it if it's new
  }
  return true;
}

void BzMaterial::setOrder(int value)
{
  order = value;
  return;
}

void BzMaterial::setDynamicColor(int dyncol)
{
  dynamicColor = dyncol;
  return;
}

void BzMaterial::setAmbient(const fvec4& color)
{
  ambient = color;
  return;
}

void BzMaterial::setDiffuse(const fvec4& color)
{
  diffuse = color;
  return;
}

void BzMaterial::setSpecular(const fvec4& color)
{
  specular = color;
  return;
}

void BzMaterial::setEmission(const fvec4& color)
{
  emission = color;
  return;
}

void BzMaterial::setShininess(float shine)
{
  shininess = shine;
  return;
}

void BzMaterial::setAlphaThreshold(float thresh)
{
  alphaThreshold = thresh;
  return;
}

void BzMaterial::setPolygonOffset(float factor, float units)
{
  poFactor = factor;
  poUnits  = units;
  return;
}

void BzMaterial::setRadarSpecial(bool value)
{
  radarSpecial = value;
  return;
}

void BzMaterial::setOccluder(bool value)
{
  occluder = value;
  return;
}

void BzMaterial::setNoRadarOutline(bool value)
{
  noRadarOutline = value;
  return;
}

void BzMaterial::setGroupAlpha(bool value)
{
  groupAlpha = value;
  return;
}

void BzMaterial::setNoRadar(bool value)
{
  noRadar = value;
  return;
}

void BzMaterial::setNoShadowCast(bool value)
{
  noShadowCast = value;
  return;
}

void BzMaterial::setNoShadowRecv(bool value)
{
  noShadowRecv = value;
  return;
}

void BzMaterial::setTextureShadow(bool value)
{
  texShadow = value;
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

void BzMaterial::setNoBlending(bool value)
{
  noBlending = value;
  return;
}

void BzMaterial::setNoLighting(bool value)
{
  noLighting = value;
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
  const std::string newName = convertTexture(texname);
  texinfo->name = newName;
  texinfo->localname = newName;
  texinfo->matrix = -1;
  texinfo->combineMode = decal;
  texinfo->useAlpha = true;
  texinfo->useColor = true;
  texinfo->useSphereMap = false;
  texinfo->autoScale = fvec2(0.0f, 0.0f);

  return;
}

void BzMaterial::setTexture(const std::string& texname)
{
  if (textureCount <= 0) {
    addTexture(texname);
  } else {
    textures[textureCount - 1].name = convertTexture(texname);
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

void BzMaterial::setTextureAutoScale(const fvec2& scale)
{
  if (textureCount > 0) {
    textures[textureCount - 1].autoScale = scale;
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


//============================================================================//
//
// Parameter retrieval
//

const std::string& BzMaterial::getName() const
{
  return name;
}

const std::vector<std::string>& BzMaterial::getAliases() const
{
  return aliases;
}

int BzMaterial::getOrder() const
{
  return order;
}

int BzMaterial::getDynamicColor() const
{
  return dynamicColor;
}

const fvec4& BzMaterial::getAmbient() const
{
  return ambient;
}

const fvec4& BzMaterial::getDiffuse() const
{
  return diffuse;
}

const fvec4& BzMaterial::getSpecular() const
{
  return specular;
}

const fvec4& BzMaterial::getEmission() const
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

bool BzMaterial::getPolygonOffset(float& factor, float& units) const
{
  factor = poFactor;
  units  = poUnits;
  return (poFactor != 0.0f) || (poUnits != 0.0f);
}

bool BzMaterial::getRadarSpecial() const
{
  return radarSpecial;
}

bool BzMaterial::getOccluder() const
{
  return occluder;
}

bool BzMaterial::getNoRadarOutline() const
{
  return noRadarOutline;
}

bool BzMaterial::getGroupAlpha() const
{
  return groupAlpha;
}

bool BzMaterial::getNoRadar() const
{
  return noRadar;
}

bool BzMaterial::getNoShadowCast() const
{
  return noShadowCast;
}

bool BzMaterial::getNoShadowRecv() const
{
  return noShadowRecv;
}

bool BzMaterial::getTextureShadow() const
{
  return texShadow;
}

bool BzMaterial::getNoCulling() const
{
  return noCulling;
}

bool BzMaterial::getNoSorting() const
{
  return noSorting;
}

bool BzMaterial::getNoBlending() const
{
  return noBlending;
}

bool BzMaterial::getNoLighting() const
{
  return noLighting;
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

const fvec2& BzMaterial::getTextureAutoScale(int texid) const
{
  static const fvec2 defScale(0.0f, 0.0f);
  if ((texid >= 0) && (texid < textureCount)) {
    return textures[texid].autoScale;
  } else {
    return defScale;
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


bool BzMaterial::isInvisible() const
{
  const DynamicColor* dyncol = DYNCOLORMGR.getColor(dynamicColor);
  if ((diffuse.a == 0.0f) && (dyncol == NULL) &&
      !((textureCount > 0) && !textures[0].useColor)) {
    return true;
  }
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
