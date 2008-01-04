/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WorldInfo:
 *  Human readable information about the map
 */

#ifndef	BZF_WORLD_INFO_H
#define	BZF_WORLD_INFO_H

#include "common.h"
#include "BufferedNetworkMessage.h"

#include <vector>
#include <string>

class WorldInfo
{
public:
  
  class WorldInfoItem
  {
  public:
    std::string key;
    std::string value;
  };

  std::vector<WorldInfoItem> items;

  void clear ( void );

  void send ( NetHandler *handler );
  void receive ( BufferedNetworkMessage *msg );
};


#endif // BZF_WORLD_INFO_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
