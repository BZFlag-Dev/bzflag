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

/* MapInfo:
 *   Information about the map
 */

#ifndef BZF_MAP_INFO_H
#define BZF_MAP_INFO_H

#include "common.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>


class MapInfo {
  public:
    typedef std::vector<std::string>                         InfoVec;
    typedef std::map<std::string, std::vector<std::string> > InfoMap;

  public:
    MapInfo();
    MapInfo(const InfoVec& lines);
    ~MapInfo();

    void clear();
    void setLines(const InfoVec& lines);
    void finalize();

    const InfoVec& getVec() const { return infoVec; }
    const InfoMap& getMap() const { return infoMap; }

    const std::vector<std::string>* getValue(const std::string& key) const;

    void print(std::ostream& out, const std::string& indent) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

  private:
    InfoVec infoVec;
    InfoMap infoMap;
};


#endif // BZF_MAP_INFO_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
