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
#include "WorldText.h"

/* system interface headers */
#include <string>
#include <vector>
#include <string.h>

/* common interface headers */
#include "Pack.h"
#include "BzMaterial.h"
#include "MeshTransform.h"
#include "CacheManager.h"


WorldTextManager WORLDTEXTMGR;


static const float defFontSize = 18.0f;
static const float defLineSpace = 1.1f;


//============================================================================//
//============================================================================//

WorldTextManager::WorldTextManager()
{
}


WorldTextManager::~WorldTextManager()
{
  clear();
}


void WorldTextManager::clear()
{
  for (size_t i = 0; i < texts.size(); i++) {
    delete texts[i];
  }
  texts.clear();
}


int WorldTextManager::addText(WorldText* text)
{
  texts.push_back(text);
  return ((int)texts.size() - 1);
}


void WorldTextManager::getFontURLs(std::set<std::string>& fontURLs) const
{
  for (size_t i = 0; i < texts.size(); i++) {
    const WorldText* text = texts[i];
    if (CacheManager::isCacheFileType(text->font)) {
      fontURLs.insert(text->font);
    }
  }
}


int WorldTextManager::packSize() const
{
  int fullSize = sizeof(uint32_t);
  for (size_t i = 0; i < texts.size(); i++) {
    fullSize += texts[i]->packSize();
  }
  return fullSize;
}


void* WorldTextManager::pack(void* buf) const
{
  buf = nboPackUInt(buf, (uint32_t)texts.size());
  for (size_t i = 0; i < texts.size(); i++) {
    buf = texts[i]->pack(buf);
  }
  return buf;
}


void* WorldTextManager::unpack(void* buf)
{
  uint32_t count;
  buf = nboUnpackUInt(buf, count);
  for (uint32_t i = 0; i < count; i++) {
    WorldText* text = new WorldText;
    buf = text->unpack(buf);
    texts.push_back(text);
  }
  return buf;
}


void WorldTextManager::print(std::ostream& out, const std::string& indent) const
{
  for (size_t i = 0; i < texts.size(); i++) {
    texts[i]->print(out, indent);
  }
}


//============================================================================//
//============================================================================//

WorldText::WorldText()
: name("")
, data("")
, font("")
, fontSize(defFontSize)
, justify(0.0f)
, lineSpace(defLineSpace)
, fixedWidth(0.0f)
, lengthPerPixel(0.0f)
, useBZDB(false)
, billboard(false)
, bzMaterial(NULL)
, poFactor(0.0f)
, poUnits(0.0f)
{
}


WorldText::WorldText(const WorldText& wt)
: name(wt.name)
, data(wt.data)
, font(wt.font)
, fontSize(wt.fontSize)
, justify(wt.justify)
, lineSpace(wt.lineSpace)
, fixedWidth(wt.fixedWidth)
, lengthPerPixel(wt.lengthPerPixel)
, useBZDB(wt.useBZDB)
, billboard(wt.billboard)
, bzMaterial(wt.bzMaterial)
, poFactor(wt.poFactor)
, poUnits(wt.poUnits)
{
  xform = wt.xform;
}


WorldText::~WorldText()
{
}


bool WorldText::isValid() const
{
  if (data.empty()) {
    return false;
  }
  return true;
}


int WorldText::packSize() const
{
  int fullSize = 0;

  fullSize += nboStdStringPackSize(name);
  fullSize += nboStdStringPackSize(font);
  fullSize += nboStdStringPackSize(data);
  fullSize += sizeof(float); // fontSize
  fullSize += sizeof(float); // justify
  fullSize += sizeof(float); // lineSpace
  fullSize += sizeof(float); // fixedWidth
  fullSize += sizeof(float); // lengthPerPixel
  fullSize += sizeof(float); // poFactor
  fullSize += sizeof(float); // poUnits

  fullSize += xform.packSize();

  fullSize += sizeof(int32_t); // bzMaterial

  fullSize += sizeof(uint8_t);   // status

  return fullSize;
}


void* WorldText::pack(void* buf) const
{
  buf = nboPackStdString(buf, name);
  buf = nboPackStdString(buf, font);
  buf = nboPackStdString(buf, data);
  buf = nboPackFloat(buf, fontSize);
  buf = nboPackFloat(buf, justify);
  buf = nboPackFloat(buf, lineSpace);
  buf = nboPackFloat(buf, fixedWidth);
  buf = nboPackFloat(buf, lengthPerPixel);
  buf = nboPackFloat(buf, poFactor);
  buf = nboPackFloat(buf, poUnits);

  buf = xform.pack(buf);

  // material
  int32_t matindex = MATERIALMGR.getIndex(bzMaterial);
  buf = nboPackInt(buf, matindex);

  uint8_t status = 0;
  status |= useBZDB   ? (1 << 0) : 0;
  status |= billboard ? (1 << 1) : 0;
  buf = nboPackUByte(buf, status);

  return buf;
}


void* WorldText::unpack(void* buf)
{
  buf = nboUnpackStdString(buf, name);
  buf = nboUnpackStdString(buf, font);
  buf = nboUnpackStdString(buf, data);
  buf = nboUnpackFloat(buf, fontSize);
  buf = nboUnpackFloat(buf, justify);
  buf = nboUnpackFloat(buf, lineSpace);
  buf = nboUnpackFloat(buf, fixedWidth);
  buf = nboUnpackFloat(buf, lengthPerPixel);
  buf = nboUnpackFloat(buf, poFactor);
  buf = nboUnpackFloat(buf, poUnits);

  buf = xform.unpack(buf);

  // material
  int32_t matindex;
  buf = nboUnpackInt(buf, matindex);
  bzMaterial = MATERIALMGR.getMaterial(matindex);

  uint8_t status;
  buf = nboUnpackUByte(buf, status);
  useBZDB   = (status & (1 << 0)) != 0;
  billboard = (status & (1 << 1)) != 0;

  if (fontSize < 1.0f) {
    fontSize = 1.0f;
  }

  return buf;
}


void WorldText::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "text" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }
  if (data.size() > 0) {
    out << indent << "  data " << data << std::endl;
  }
  if (useBZDB) {
    out << indent << "  bzdb" << std::endl;
  }
  if (font.size() > 0) {
    out << indent << "  font " << font << std::endl;
  }
  if (fontSize != defFontSize) {
    out << indent << "  fontsize " << fontSize << std::endl;
  }
  if (justify != 0.0f) {
    out << indent << "  justify " << justify << std::endl;
  }
  if (lineSpace != defLineSpace) {
    out << indent << "  linespace " << lineSpace << std::endl;
  }
  if (fixedWidth != 0.0f) {
    out << indent << "  fixedwidth " << fixedWidth << std::endl;
  }
  if (lengthPerPixel != 0.0f) {
    out << indent << "  lengthPerPixel " << lengthPerPixel << std::endl;
  }

  if (billboard) {
    out << indent << "  billboard" << std::endl;
  }

  xform.printTransforms(out, indent);

  out << indent << "  matref ";
  MATERIALMGR.printReference(out, bzMaterial);
  out << std::endl;

  if ((poFactor != 0.0f) || (poUnits != 0.0f)) {
    out << indent << "  polyoffset " << poFactor << " " << poUnits << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;
}


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
