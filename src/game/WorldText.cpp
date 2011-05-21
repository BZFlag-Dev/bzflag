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


static const float defFontSize = 18.0f;
static const float defLineSpace = 1.1f;


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
  , fromGroup(false) {
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
  , fromGroup(wt.fromGroup) {
  xform = wt.xform;
}


WorldText::~WorldText() {
}


WorldText* WorldText::copyWithTransform(const MeshTransform& transform) const {
  WorldText* newText = new WorldText(*this);
  newText->xform.append(transform);
  return newText;
}


bool WorldText::isValid() const {
  if (data.empty()) {
    return false;
  }
  return true;
}


int WorldText::packSize() const {
  int fullSize = 0;

  fullSize += nboStdStringPackSize(name);
  fullSize += nboStdStringPackSize(font);
  fullSize += nboStdStringPackSize(data);
  fullSize += sizeof(float); // fontSize
  fullSize += sizeof(float); // justify
  fullSize += sizeof(float); // lineSpace
  fullSize += sizeof(float); // fixedWidth
  fullSize += sizeof(float); // lengthPerPixel

  fullSize += xform.packSize();

  fullSize += sizeof(int32_t); // bzMaterial

  fullSize += sizeof(uint8_t);   // status

  return fullSize;
}


void* WorldText::pack(void* buf) const {
  buf = nboPackStdString(buf, name);
  buf = nboPackStdString(buf, font);
  buf = nboPackStdString(buf, data);
  buf = nboPackFloat(buf, fontSize);
  buf = nboPackFloat(buf, justify);
  buf = nboPackFloat(buf, lineSpace);
  buf = nboPackFloat(buf, fixedWidth);
  buf = nboPackFloat(buf, lengthPerPixel);

  buf = xform.pack(buf);

  // material
  int32_t matindex = MATERIALMGR.getIndex(bzMaterial);
  buf = nboPackInt32(buf, matindex);

  uint8_t status = 0;
  status |= useBZDB   ? (1 << 0) : 0;
  status |= billboard ? (1 << 1) : 0;
  buf = nboPackUInt8(buf, status);

  return buf;
}


void* WorldText::unpack(void* buf) {
  buf = nboUnpackStdString(buf, name);
  buf = nboUnpackStdString(buf, font);
  buf = nboUnpackStdString(buf, data);
  buf = nboUnpackFloat(buf, fontSize);
  buf = nboUnpackFloat(buf, justify);
  buf = nboUnpackFloat(buf, lineSpace);
  buf = nboUnpackFloat(buf, fixedWidth);
  buf = nboUnpackFloat(buf, lengthPerPixel);

  buf = xform.unpack(buf);

  // material
  int32_t matindex;
  buf = nboUnpackInt32(buf, matindex);
  bzMaterial = MATERIALMGR.getMaterial(matindex);

  uint8_t status;
  buf = nboUnpackUInt8(buf, status);
  useBZDB   = (status & (1 << 0)) != 0;
  billboard = (status & (1 << 1)) != 0;

  if (fontSize < 1.0f) {
    fontSize = 1.0f;
  }

  return buf;
}


void WorldText::print(std::ostream& out, const std::string& indent) const {
  out << indent << "text" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  if (!useBZDB) {
    if (data.size() > 0) {
      out << indent << "  string " << data << std::endl;
    }
  }
  else {
    if (data.size() > 0) {
      out << indent << "  varName " << data << std::endl;
    }
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

  out << indent << "end" << std::endl << std::endl;
}


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
