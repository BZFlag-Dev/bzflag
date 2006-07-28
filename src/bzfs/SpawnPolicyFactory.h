/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include <iostream>
#include <string>
#include <map>

/* common interface headers */
#include "TextUtils.h"

/* bzfs-specific interface headers */
#include "SpawnPolicy.h"


/** a SpawnPolicyFactory has all of the available spawn policies
 * registered and is used to create an instance of one of the
 * policies.
 */
class SpawnPolicyFactory
{

public:
  typedef std::map<std::string, SpawnPolicy *> PolicyRegister;

  static SpawnPolicy *DefaultPolicy();
  static SpawnPolicy *Policy(std::string);
  static void SetDefault(std::string);

  static bool IsValid(std::string policy);
  static void PrintList(std::ostream &stream);

  template <class _PolicyType>
  static bool RegisterPolicy();

  /** here is where all of the policies are actually registered */
  static bool Init();

private:
  SpawnPolicyFactory();
  ~SpawnPolicyFactory();

  static std::string _defaultPolicy;

  /** registration map of available policy types */
  static PolicyRegister _policies;

};


template <class _PolicyType>
bool SpawnPolicyFactory::RegisterPolicy() {
  _PolicyType *t = new _PolicyType();
  std::string name = TextUtils::tolower(t->Name());
  PolicyRegister::const_iterator policyEntry = _policies.find(name);
  if (policyEntry != _policies.end()) {
    std::cerr << "ERROR: The [" << t->Name() << "] spawn policy is already registered" << std::endl;
    return false;
  }
  _policies[name] = t;
  return true;
}
  

#endif  /*__SPAWNPOLICYFACTORY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
