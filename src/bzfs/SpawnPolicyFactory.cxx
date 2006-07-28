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

/* interface header */
#include "SpawnPolicyFactory.h"

/* bzfs-specific implementation headers */
#include "DefaultSpawnPolicy.h"
#include "RandomSpawnPolicy.h"
#include "DangerousSpawnPolicy.h"


// initialize to an "unset" policy
std::string SpawnPolicyFactory::_defaultPolicy = std::string("");

// recognized policies
SpawnPolicyFactory::PolicyRegister SpawnPolicyFactory::_policies;

// register the policies
static bool _policiesInitialized = SpawnPolicyFactory::Init();


/* public */

SpawnPolicy *
SpawnPolicyFactory::DefaultPolicy() {
  SpawnPolicy *policy = SpawnPolicyFactory::Policy(SpawnPolicyFactory::_defaultPolicy);

  /* failsafe, just so we don't ever return NULL on this call */
  if (!policy) {
    return new DefaultSpawnPolicy();
  }
  return policy;
}

SpawnPolicy *
SpawnPolicyFactory::Policy(std::string policy) {
  std::string lcPolicy = TextUtils::tolower(policy);
  if (lcPolicy == "") {
    lcPolicy = "default";
  }
  PolicyRegister::const_iterator policyEntry = _policies.find(lcPolicy);
  if (policyEntry != _policies.end()) {
    return policyEntry->second;
  }
  
  /* may return NULL if policy isn't something recognized */
  return (SpawnPolicy *)NULL;
}

void
SpawnPolicyFactory::SetDefault(std::string policy) {
  _defaultPolicy = policy;
}

bool
SpawnPolicyFactory::IsValid(std::string policy) {
  std::string lcPolicy = TextUtils::tolower(policy);
  if (lcPolicy == "") {
    lcPolicy = "default";
  }
  PolicyRegister::const_iterator policyEntry = _policies.find(lcPolicy);
  if (policyEntry != _policies.end()) {
    // found it
    return true;
  }
  return false;
}

void
SpawnPolicyFactory::PrintList(std::ostream &stream) {
  PolicyRegister::const_iterator policyEntry = _policies.begin();
  while (policyEntry != _policies.end()) {
    stream << policyEntry->second->Name() << std::endl;
    policyEntry++;
  }
}


/* private */

SpawnPolicyFactory::SpawnPolicyFactory()
{
}

SpawnPolicyFactory::~SpawnPolicyFactory()
{
}

bool
SpawnPolicyFactory::Init()
{
  static bool _initialized = false;
  if (!_initialized) {
    RegisterPolicy<DefaultSpawnPolicy>();
    RegisterPolicy<RandomSpawnPolicy>();
    RegisterPolicy<DangerousSpawnPolicy>();
    _initialized = true;
  }
  return _initialized;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
