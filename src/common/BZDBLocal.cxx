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


/******************************************************************************/
//
// BZDBLocalBool
//

BZDBLocalBool::BZDBLocalBool(const std::string& _name, bool defVal) :
                             BZDBLocal(_name), data(defVal)
{
  BZDBLocalManager::manager.addEntry(this);
  return;
}


BZDBLocalBool::~BZDBLocalBool()
{
  return;
}


void BZDBLocalBool::callback()
{
  data = BZDB.isTrue(name);
  DEBUG4("BZDBLocalBool(%s) = %s\n", name.c_str(), data ? "true" : "false");
  return;
}


void BZDBLocalBool::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBLocalBool*)data)->callback();
  return;
}


void BZDBLocalBool::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBLocalBool duplicate \"%s\".\n", name.c_str());
    exit(1);
  }
  BZDB.setBool(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBLocalBool::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBLocalInt
//

BZDBLocalInt::BZDBLocalInt(const std::string& _name, int defVal,
                           int _min, int _max, bool _neverZero) :
                           BZDBLocal(_name), data(defVal),
                           min(_min), max(_max), neverZero(_neverZero)
{
  BZDBLocalManager::manager.addEntry(this);
  return;
}


BZDBLocalInt::~BZDBLocalInt()
{
  return;
}


void BZDBLocalInt::callback()
{
  int tmp = BZDB.evalInt(name);

  if ((min != INT_MIN) && (tmp < min)) {
    DEBUG3("BZDBLocalInt(%s) min: %f < %f\n", name.c_str(), tmp, min);
    tmp = min; // clamp to min
  }

  if ((max != INT_MAX) && (tmp > max)) {
    DEBUG3("BZDBLocalInt(%s) max: %f > %f\n", name.c_str(), tmp, max);
    tmp = max; // clamp to max
  }

  if (neverZero && (tmp == 0)) {
    DEBUG3("BZDBLocalInt(%s) neverZero\n", name.c_str());
    return; // bail out
  }

  data = tmp; // set the new value

  DEBUG4("BZDBLocalInt(%s) = %i\n", name.c_str(), data);

  return;
}


void BZDBLocalInt::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBLocalInt*)data)->callback();
  return;
}


void BZDBLocalInt::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBLocalInt duplicate \"%s\".\n", name.c_str());
    exit(1);
  }
  BZDB.setInt(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBLocalInt::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBLocalFloat
//

BZDBLocalFloat::BZDBLocalFloat(const std::string& _name, float defVal,
                               float _min, float _max, bool _neverZero) :
                               BZDBLocal(_name), data(defVal),
                               min(_min), max(_max), neverZero(_neverZero)
{
  BZDBLocalManager::manager.addEntry(this);
  return;
}


BZDBLocalFloat::~BZDBLocalFloat()
{
  return;
}


void BZDBLocalFloat::callback()
{
  float tmp = BZDB.eval(name);

  if ((min != -MAXFLOAT) && (tmp < min)) {
    DEBUG3("BZDBLocalFloat(%s) min: %f < %f\n", name.c_str(), tmp, min);
    tmp = min; // clamp to min
  }

  if ((max != +MAXFLOAT) && (tmp > max)) {
    DEBUG3("BZDBLocalFloat(%s) max: %f > %f\n", name.c_str(), tmp, max);
    tmp = max; // clamp to max
  }

  if (neverZero && (tmp == 0.0f)) {
    DEBUG3("BZDBLocalFloat(%s) neverZero\n", name.c_str());
    return; // bail out
  }

  data = tmp; // set the new value

  DEBUG4("BZDBLocalFloat(%s) = %f\n", name.c_str(), data);

  return;
}


void BZDBLocalFloat::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBLocalFloat*)data)->callback();
  return;
}


void BZDBLocalFloat::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBLocalFloat duplicate \"%s\".\n", name.c_str());
    exit(1);
  }
  BZDB.setFloat(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBLocalFloat::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBLocalColor
//

BZDBLocalColor::BZDBLocalColor(const std::string& _name,
                               float r, float g, float b, float a,
                               bool _neverAlpha) :
                               BZDBLocal(_name), neverAlpha(_neverAlpha)
{
  data[0] = r;
  data[1] = g;
  data[2] = b;
  data[3] = a;
  BZDBLocalManager::manager.addEntry(this);
  return;
}


BZDBLocalColor::~BZDBLocalColor()
{
  return;
}


void BZDBLocalColor::callback()
{
  const std::string& expr = BZDB.get(name);
  float color[4];
  
  if (!parseColorString(expr, color)) {
    DEBUG3("BZDBLocalColor(%s) bad string: %s\n", name.c_str(), expr.c_str());
    return;
  }

  if (neverAlpha && (color[3] < 1.0f)) {
    DEBUG3("BZDBLocalColor(%s) made opaque: %f\n", name.c_str(), color[3]);
    color[3] = 1.0f;
  }

  // set the new value
  memcpy(data, color, sizeof(float[4]));

  DEBUG4("BZDBLocalColor(%s) = %f, %f, %f, %f\n", name.c_str(),
         data[0], data[1], data[2], data[3]);

  return;
}


void BZDBLocalColor::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBLocalColor*)data)->callback();
  return;
}


void BZDBLocalColor::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBLocalColor duplicate \"%s\".\n", name.c_str());
    exit(1);
  }

  char buf[256];
  snprintf(buf, 256, " %f %f %f %f", data[0], data[1], data[2], data[3]);
  BZDB.set(name, buf);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBLocalColor::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/
//
// BZDBLocalString
//

BZDBLocalString::BZDBLocalString(const std::string& _name,
                                 const std::string& defVal, bool _neverEmpty) :
                                 BZDBLocal(_name),
                                 data(defVal), neverEmpty(_neverEmpty)
{
  BZDBLocalManager::manager.addEntry(this);
  return;
}


BZDBLocalString::~BZDBLocalString()
{
  return;
}


void BZDBLocalString::callback()
{
  const std::string& tmp = BZDB.get(name);
  
  if (neverEmpty && (tmp.size() <= 0)) {
    DEBUG3("BZDBLocalString(%s) empty string: %s\n", name.c_str(), tmp.c_str());
    return;
  }

  data = tmp; // set the new value

  DEBUG4("BZDBLocalString(%s) = %s\n", name.c_str(), data.c_str());

  return;
}


void BZDBLocalString::staticCallback(const std::string& /*name*/, void* data)
{
  ((BZDBLocalString*)data)->callback();
  return;
}


void BZDBLocalString::addCallbacks()
{
  if (BZDB.isSet(name)) {
    fprintf(stderr, "BZDBLocalString duplicate \"%s\".\n", name.c_str());
    exit(1);
  }

  BZDB.set(name, data);
  BZDB.setDefault(name, BZDB.get(name));
  BZDB.addCallback(name, staticCallback, this);
  return;
}

    
void BZDBLocalString::removeCallbacks()
{
  BZDB.removeCallback(name, staticCallback, this);
  return;
}

    
/******************************************************************************/

BZDBLocalManager::BZDBLocalManager BZDBLocalManager::manager;


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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
