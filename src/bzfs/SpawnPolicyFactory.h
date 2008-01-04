/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SPAWNPOLICYFACTORY_H__
#define __SPAWNPOLICYFACTORY_H__

#include "common.h"

/* system headers */
#include <string>

/* common interface headers */
#include "Factory.h"
#include "Singleton.h"
#include "TextUtils.h"

/* bzfs-specific interface headers */
#include "SpawnPolicy.h"


/** convenience handle on the singleton instance */
#define SPAWNPOLICY (SpawnPolicyFactory::instance())


/** a SpawnPolicyFactory has all of the available spawn policies
 * registered and is used to create instances of the policies.
 */
class SpawnPolicyFactory : public Singleton<SpawnPolicyFactory>,
			   public Factory<SpawnPolicy, std::string>
{

public:
  SpawnPolicy *Policy(std::string s = std::string(""));

  void setDefault(std::string);

protected:
  friend class Singleton<SpawnPolicyFactory>;

private:
  SpawnPolicyFactory();
  ~SpawnPolicyFactory();

  std::string _defaultPolicy;
};


#endif  /*__SPAWNPOLICYFACTORY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
