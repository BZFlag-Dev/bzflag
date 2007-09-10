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

#include "common.h"

// implementation header
#include "BZDBLocal.h"

// system headers
#include <stdio.h>
#include <string.h>
#include <string>

// common headers
#include "StateDatabase.h"
#include "ParseColor.h"


// Function Prototypes
// -------------------
static void safeSetInt(const std::string& name, int value);
static void safeSetFloat(const std::string& name, float value);
static void safeSetString(const std::string& name, const std::string&value);


/******************************************************************************/
//
// BZDBbool
//

BZDBbool::BZDBbool(const std::string& _name, bool defVal, bool save)
                   : BZDBLocal(_name, save), data(defVal)
{
  BZDBLocalManager::manager.addEntry(this);
  logDebugMessage(3,"Added BZDBbool(%s) callback\n", name.c_str());
  return;
}


BZDBbool::~BZDBbool()
{
  return;
}


void BZDBbool::callback()
{
  data = BZDB.isTrue(name);
  logDebugMessage(4,"BZDBbool(%s) = %s\n", name.c_str(), data ? "true" : "false");
  return;
}


void BZDBbool::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBbool*)data)->callback();
  return;
}


void BZDBbool::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBbool duplicate \"%s\".\n", name.c_str());
    exit(1);
  }
  BZDB.setBool(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.setPersistent(name, saveOnExit);
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBbool::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBint
//

BZDBint::BZDBint(const std::string& _name, int defVal,
                 int _min, int _max, bool _neverZero, bool save)
                 : BZDBLocal(_name, save), data(defVal),
                   min(_min), max(_max), neverZero(_neverZero)
{
  BZDBLocalManager::manager.addEntry(this);
  logDebugMessage(3,"Added BZDBint(%s) callback\n", name.c_str());
  return;
}


BZDBint::~BZDBint()
{
  return;
}


void BZDBint::callback()
{
  int tmp = BZDB.evalInt(name);

  if (tmp < min) {
    logDebugMessage(3,"BZDBint(%s) min: %f < %f\n", name.c_str(), tmp, min);
    tmp = min; // clamp to min
    safeSetInt(name, tmp);
  }

  if (tmp > max) {
    logDebugMessage(3,"BZDBint(%s) max: %f > %f\n", name.c_str(), tmp, max);
    tmp = max; // clamp to max
    safeSetInt(name, tmp);
  }

  if (neverZero && (tmp == 0)) {
    logDebugMessage(3,"BZDBint(%s) neverZero\n", name.c_str());
    return; // bail out
  }

  data = tmp; // set the new value

  logDebugMessage(4,"BZDBint(%s) = %i\n", name.c_str(), data);

  return;
}


void BZDBint::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBint*)data)->callback();
  return;
}


void BZDBint::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBint duplicate \"%s\".\n", name.c_str());
    exit(1);
  }
  BZDB.setInt(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.setPersistent(name, saveOnExit);
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBint::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBfloat
//

BZDBfloat::BZDBfloat(const std::string& _name, float defVal,
                     float _min, float _max,
                     bool _neverZero, bool save)
                     : BZDBLocal(_name, save), data(defVal),
                       min(_min), max(_max), neverZero(_neverZero)
{
  BZDBLocalManager::manager.addEntry(this);
  logDebugMessage(3,"Added BZDBfloat(%s) callback\n", name.c_str());
  return;
}


BZDBfloat::~BZDBfloat()
{
  return;
}


void BZDBfloat::callback()
{
  float tmp = BZDB.eval(name);

  if (tmp < min) {
    logDebugMessage(3,"BZDBfloat(%s) min: %f < %f\n", name.c_str(), tmp, min);
    tmp = min; // clamp to min
    safeSetFloat(name, tmp);
  }

  if (tmp > max) {
    logDebugMessage(3,"BZDBfloat(%s) max: %f > %f\n", name.c_str(), tmp, max);
    tmp = max; // clamp to max
    safeSetFloat(name, tmp);
  }

  if (neverZero && (tmp == 0.0f)) {
    logDebugMessage(3,"BZDBfloat(%s) neverZero\n", name.c_str());
    return; // bail out
  }

  data = tmp; // set the new value

  logDebugMessage(4,"BZDBfloat(%s) = %f\n", name.c_str(), data);

  return;
}


void BZDBfloat::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBfloat*)data)->callback();
  return;
}


void BZDBfloat::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBfloat duplicate \"%s\".\n", name.c_str());
    exit(1);
  }
  BZDB.setFloat(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.setPersistent(name, saveOnExit);
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBfloat::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBcolor
//

BZDBcolor::BZDBcolor(const std::string& _name,
                     float r, float g, float b, float a,
                     bool _neverAlpha, bool save)
                     : BZDBLocal(_name, save), neverAlpha(_neverAlpha)
{
  data[0] = r;
  data[1] = g;
  data[2] = b;
  data[3] = a;
  BZDBLocalManager::manager.addEntry(this);
  logDebugMessage(3,"Added BZDBcolor(%s) callback\n", name.c_str());
  return;
}


BZDBcolor::~BZDBcolor()
{
  return;
}


void BZDBcolor::callback()
{
  const std::string& expr = BZDB.get(name);
  float color[4];
  
  if (!parseColorString(expr, color)) {
    logDebugMessage(3,"BZDBcolor(%s) bad string: %s\n", name.c_str(), expr.c_str());
    return;
  }

  if (neverAlpha && (color[3] < 1.0f)) {
    logDebugMessage(3,"BZDBcolor(%s) made opaque: %f\n", name.c_str(), color[3]);
    color[3] = 1.0f;
    char buf[256];
    snprintf(buf, 256, " %f %f %f %f", color[0], color[1], color[2], color[3]);
    safeSetString(name, buf);
  }

  // set the new value
  memcpy(data, color, sizeof(float[4]));

  logDebugMessage(4,"BZDBcolor(%s) = %f, %f, %f, %f\n", name.c_str(),
         data[0], data[1], data[2], data[3]);

  return;
}


void BZDBcolor::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBcolor*)data)->callback();
  return;
}


void BZDBcolor::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBcolor duplicate \"%s\".\n", name.c_str());
    exit(1);
  }

  char buf[256];
  snprintf(buf, 256, " %f %f %f %f", data[0], data[1], data[2], data[3]);
  BZDB.set(name, buf);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.setPersistent(name, saveOnExit);
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBcolor::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBstring
//

BZDBstring::BZDBstring(const std::string& _name, const std::string& defVal,
                       bool _neverEmpty, bool save)
                       : BZDBLocal(_name, save),
                         data(defVal), neverEmpty(_neverEmpty)
{
  BZDBLocalManager::manager.addEntry(this);
  logDebugMessage(3,"Added BZDBstring(%s) callback\n", name.c_str());
  return;
}


BZDBstring::~BZDBstring()
{
  return;
}


void BZDBstring::callback()
{
  const std::string& tmp = BZDB.get(name);
  
  if (neverEmpty && (tmp.size() <= 0)) {
    logDebugMessage(3,"BZDBstring(%s) empty string: %s\n", name.c_str(), tmp.c_str());
    safeSetString(name, tmp);
    return;
  }

  data = tmp; // set the new value

  logDebugMessage(4,"BZDBstring(%s) = %s\n", name.c_str(), data.c_str());

  return;
}


void BZDBstring::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBstring*)data)->callback();
  return;
}


void BZDBstring::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBstring duplicate \"%s\".\n", name.c_str());
    exit(1);
  }

  BZDB.set(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.setPersistent(name, saveOnExit);
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBstring::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/

BZDBLocalManager BZDBLocalManager::manager;


static std::vector<BZDBLocal*>& getEntryVector()
{
  static std::vector<BZDBLocal*> vec;
  return vec;
}


BZDBLocalManager::BZDBLocalManager()
{
  return;
}


BZDBLocalManager::~BZDBLocalManager()
{
  return;
}


bool BZDBLocalManager::addEntry(BZDBLocal* entry)
{
  std::vector<BZDBLocal*>& vec = getEntryVector();
  vec.push_back(entry);
  return true;
}


void BZDBLocalManager::init()
{
  std::vector<BZDBLocal*>& entries = getEntryVector();
  for (unsigned int i = 0; i < entries.size(); i++) {
    entries[i]->addCallbacks();
  }
  return;
}


void BZDBLocalManager::kill()
{
  std::vector<BZDBLocal*>& entries = getEntryVector();
  for (unsigned int i = 0; i < entries.size(); i++) {
    entries[i]->removeCallbacks();
  }
  return;
}


/******************************************************************************/
/******************************************************************************/

static bool Inside = false;

static void safeSetInt(const std::string& name, int value)
{
  if (!Inside) {
    Inside = true;
    BZDB.setInt(name, value);
    Inside = false;
  }
  return;
}


static void safeSetFloat(const std::string& name, float value)
{
  if (!Inside) {
    Inside = true;
    BZDB.setFloat(name, value);
    Inside = false;
  }
  return;
}


static void safeSetString(const std::string& name, const std::string&value)
{
  if (!Inside) {
    Inside = true;
    BZDB.set(name, value);
    Inside = false;
  }
  return;
}


/******************************************************************************/

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
