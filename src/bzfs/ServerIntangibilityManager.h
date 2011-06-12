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

#ifndef __SERVERINTANGIBILITYMANAGER_H__
#define __SERVERINTANGIBILITYMANAGER_H__

#include "common.h"

/* system headers */
#include <map>

/* common headers */
#include "Singleton.h"

#include "obstacle/Obstacle.h"


#define _INVALID_TANGIBILITY 255

class ServerIntangibilityManager :   public Singleton<ServerIntangibilityManager> {
  public:
    void setWorldObjectTangibility(uint32_t objectGUID, unsigned char tangible);

    void sendNewPlayerWorldTangibility(int playerID);

    void resetTangibility(void);

    unsigned char getWorldObjectTangibility(const Obstacle* obs);
    unsigned char getWorldObjectTangibility(uint32_t objectGUID);

  protected:
    friend class Singleton<ServerIntangibilityManager>;

  private:
    ServerIntangibilityManager() {};
    ~ServerIntangibilityManager() {};

    // obstacle id to tangilibilty map
    typedef std::map<uint32_t, unsigned char> TangibilityMap;
    TangibilityMap tangibilityMap;
};

#endif  /*__SERVERINTANGIBILITYMANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
