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

#ifndef __BZFS_STATS_H__
#define __BZFS_STATS_H__

#include "common.h"
#include "bzfs/bzfsAPI.h"
#include <string>

class StatsLink : public bz_EventHandler {
  public:
    StatsLink();
    virtual ~StatsLink();

    void init(void);

    virtual void process(bz_EventData* eventData);
    virtual bool autoDelete(void) { return true; }

  private:
    void buildXMLPlayerList(std::string& params);
    void buildXMLPlayer(std::string& params, int playerID);
    void buildHTMLPlayerList(std::string& params, int skip = -1);
    void buildHTMLPlayer(std::string& params, int playerID, int index);
    bool getPushHeader(std::string& header);
    void buildStateHash(std::string& params);

    std::string url;
    bool sentAdd;

    std::string mapFile;
};

#endif //__BZFS_STATS_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
