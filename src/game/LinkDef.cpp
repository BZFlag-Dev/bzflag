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
#include "LinkDef.h"

// system headers
#include <string>
#include <vector>

// common headers
#include "Pack.h"


//============================================================================//

LinkDef::LinkDef() {
}


LinkDef::LinkDef(const LinkDef& def)
  : srcs(def.srcs)
  , dsts(def.dsts)
  , physics(def.physics) {
}


LinkDef::~LinkDef() {
}


//============================================================================//

void LinkDef::addSrc(const std::string& src) {
  if (!src.empty()) {
    srcs.push_back(src);
  }
}


void LinkDef::addDst(const std::string& dst) {
  if (!dst.empty()) {
    dsts.push_back(dst);
  }
}


//============================================================================//

LinkDef LinkDef::prepend(const std::string& prefix) {
  LinkDef def;
  def.physics = physics;
  for (size_t i = 0; i < srcs.size(); i++) {
    const std::string& src = srcs[i];
    if (!src.empty() && (src[0] != '/')) {
      srcs[i] = prefix + src;
    }
  }
  for (size_t i = 0; i < dsts.size(); i++) {
    const std::string& dst = dsts[i];
    if (!dst.empty() && (dst[0] != '/')) {
      dsts[i] = prefix + dst;
    }
  }
  return def;
}


//============================================================================//

int LinkDef::packSize() const {
  int fullSize = 0;

  fullSize += sizeof(uint16_t); // src count
  for (size_t i = 0; i < srcs.size(); i++) {
    fullSize += nboStdStringPackSize(srcs[i]);
  }

  fullSize += sizeof(uint16_t); // dst count
  for (size_t i = 0; i < dsts.size(); i++) {
    fullSize += nboStdStringPackSize(dsts[i]);
  }

  fullSize += physics.packSize();

  return fullSize;
}


void* LinkDef::pack(void* buf) const {
  uint16_t count;

  count = (uint16_t)srcs.size();
  buf = nboPackUInt16(buf, count);
  for (size_t i = 0; i < srcs.size(); i++) {
    buf = nboPackStdString(buf, srcs[i]);
  }

  count = (uint16_t)dsts.size();
  buf = nboPackUInt16(buf, count);
  for (size_t i = 0; i < dsts.size(); i++) {
    buf = nboPackStdString(buf, dsts[i]);
  }

  buf = physics.pack(buf);

  return buf;
}


void* LinkDef::unpack(void* buf) {
  uint16_t count;

  buf = nboUnpackUInt16(buf, count);
  for (uint16_t i = 0; i < count; i++) {
    std::string src;
    buf = nboUnpackStdString(buf, src);
    addSrc(src);
  }

  buf = nboUnpackUInt16(buf, count);
  for (uint16_t i = 0; i < count; i++) {
    std::string dst;
    buf = nboUnpackStdString(buf, dst);
    addDst(dst);
  }

  buf = physics.unpack(buf);

  return buf;
}


//============================================================================//

void LinkDef::print(std::ostream& out, const std::string& indent) const {
  out << indent << "link" << std::endl;
  for (size_t i = 0; i < srcs.size(); i++) {
    out << indent << "  addSrc " << srcs[i] << std::endl;
  }
  for (size_t i = 0; i < dsts.size(); i++) {
    out << indent << "  addDst " << dsts[i] << std::endl;
  }
  physics.print(out, indent);
  out << indent << "end" << std::endl << std::endl;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
