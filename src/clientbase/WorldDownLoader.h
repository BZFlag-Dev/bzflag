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

#ifndef	BZF_WORLDDOWNLOADER_H
#define	BZF_WORLDDOWNLOADER_H

#include "common.h"

// system includes
#include <string>
#include <iostream>

/* common headers */
#include "cURLManager.h"

class WorldDownLoader : private cURLManager {
  public:
    WorldDownLoader();
    ~WorldDownLoader();

    void start(char *hexDigest);
    void stop();
    void setCacheURL(char *cacheURL);
    void setCacheTemp(bool cacheTemp);
    uint32_t processChunk(void *buf, uint16_t len, int bytesLeft);
    void cleanCache();

  private:
    void askToBZFS();
    bool isCached(char *hexDigest);
    void loadCached();
    void markOld(std::string &fileName);
    virtual void finalization(char *data, unsigned int length, bool good);

    std::string worldUrl;
    std::string worldHash;
    std::string worldCachePath;
    std::string md5Digest;
    uint32_t worldPtr;
    char *worldDatabase;
    bool isCacheTemp;
    std::ostream *cacheOut;
};


#endif // BZF_WORLDDOWNLOADER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
