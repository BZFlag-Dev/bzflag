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

/* interface header */
#include "DynamicColor.h"

/* system implementation headers */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;


/* common implementation headers */
#include "GameTime.h"
#include "Pack.h"
#include "TimeKeeper.h"
#include "ParseColor.h"
#include "StateDatabase.h"


// this applies to all function periods
const float DynamicColor::minPeriod = 0.01f;


//============================================================================//
//
// Dynamic Color Manager
//

DynamicColorManager DYNCOLORMGR;


DynamicColorManager::DynamicColorManager()
{
  return;
}


DynamicColorManager::~DynamicColorManager()
{
  clear();
  return;
}


void DynamicColorManager::clear()
{
  vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    delete *it;
  }
  colors.clear();
  return;
}


void DynamicColorManager::update()
{
  const double gameTime = GameTime::getStepTime();
  vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    color->update(gameTime);
  }
  return;
}


int DynamicColorManager::addColor(DynamicColor* color)
{
  colors.push_back(color);
  return ((int)colors.size() - 1);
}


int DynamicColorManager::findColor(const string& dyncol) const
{
  if (dyncol.empty()) {
    return -1;
  }
  else if ((dyncol[0] >= '0') && (dyncol[0] <= '9')) {
    int index = atoi (dyncol.c_str());
    if ((index < 0) || (index >= (int)colors.size())) {
      return -1;
    } else {
      return index;
    }
  } else {
    for (int i = 0; i < (int)colors.size(); i++) {
      if (colors[i]->getName() == dyncol) {
	return i;
      }
    }
    return -1;
  }
}


const DynamicColor* DynamicColorManager::getColor(int id) const
{
  if ((id >= 0) && (id < (int)colors.size())) {
    return colors[id];
  } else {
    return NULL;
  }
}


void * DynamicColorManager::pack(void *buf) const
{
  vector<DynamicColor*>::const_iterator it;
  buf = nboPackUInt(buf, (int)colors.size());
  for (it = colors.begin(); it != colors.end(); it++) {
    const DynamicColor* color = *it;
    buf = color->pack(buf);
  }
  return buf;
}


void * DynamicColorManager::unpack(void *buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    DynamicColor* color = new DynamicColor;
    buf = color->unpack(buf);
    addColor(color);
  }
  return buf;
}


int DynamicColorManager::packSize() const
{
  int fullSize = sizeof (uint32_t);
  vector<DynamicColor*>::const_iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    fullSize = fullSize + color->packSize();
  }
  return fullSize;
}


void DynamicColorManager::print(std::ostream& out, const string& indent) const
{
  vector<DynamicColor*>::const_iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    color->print(out, indent);
  }
  return;
}


//============================================================================//
//
// Dynamic Color
//

DynamicColor::DynamicColor()
{
  const fvec4 white(1.0f, 1.0f, 1.0f, 1.0f);

  name = "";
  possibleAlpha = false;

  color = white;

  varName = "";
  varTime = 1.0f;
  varNoAlpha = false;

  varInit = false;
  varTimeTmp = varTime;
  varTransition = false;
  varOldColor = white;
  varNewColor = white;
  varLastChange = TimeKeeper::getSunGenesisTime();

  statesDelay = 0.0f;

  return;
}


DynamicColor::~DynamicColor()
{
  // free the sequence values
  if (varInit) {
    BZDB.removeCallback(varName, bzdbCallback, this);
  }
  return;
}


void DynamicColor::finalize()
{
  possibleAlpha = false;

  // variables take priority
  if (!varName.empty()) {
    possibleAlpha = !varNoAlpha;
    return;
  }

  // followed by color states
  if (!colorStates.empty()) {
    statesLength = 0.0f;
    for (size_t i = 0; i < colorStates.size(); i++) {
      const ColorState& state = colorStates[i];
      const ColorState& next  = colorStates[(i + 1) % colorStates.size()];

      statesLength += state.duration;
      if (colorEnds.find(statesLength) == colorEnds.end()) {
        colorEnds[statesLength] = (int)i;
      }

      const float alpha = state.color[3];
      if (((alpha > 0.0f) && (alpha < 1.0f)) ||
          ((alpha != next.color[3]) && (state.duration > 0.0f))) {
        possibleAlpha = true;
      }
    }
  }
}


bool DynamicColor::setName(const string& dyncol)
{
  if (dyncol.empty()) {
    name = "";
    return false;
  } else if ((dyncol[0] >= '0') && (dyncol[0] <= '9')) {
    name = "";
    return false;
  } else {
    name = dyncol;
  }
  return true;
}


const string& DynamicColor::getName() const
{
  return name;
}


void DynamicColor::setVariableName(const string& vName)
{
  varName = vName;
  return;
}


void DynamicColor::setVariableTiming(float seconds)
{
  varTime = seconds;
  return;
}


void DynamicColor::setVariableNoAlpha(bool value)
{
  varNoAlpha = value;
  return;
}


void DynamicColor::setDelay(float delay)
{
  statesDelay = delay;
  return;
}


void DynamicColor::addState(float duration, const fvec4& _color)
{
  if (duration < 0.0f) {
    duration = 0.0f;
  }
  colorStates.push_back(ColorState(_color, duration));
  return;
}


void DynamicColor::addState(float duration,
                            float r, float g, float b, float a)
{
  if (duration < 0.0f) {
    duration = 0.0f;
  }
  colorStates.push_back(ColorState(fvec4(r, g, b, a), duration));
  return;
}


void DynamicColor::bzdbCallback(const string& /*varName*/, void* data)
{
  ((DynamicColor*)data)->updateVariable();
  return;
}


void DynamicColor::updateVariable()
{
  // setup the basics
  varTransition = true;
  varLastChange = TimeKeeper::getTick();
  varOldColor = color;
  string expr = BZDB.get(varName);

  // parse the optional delay timing
  string::size_type atpos = expr.find_first_of('@');
  if (atpos == string::npos) {
    varTimeTmp = varTime;
  }
  else {
    char* end;
    const char* start = expr.c_str() + atpos + 1;
    varTimeTmp = (float)strtod(start, &end);
    if (end == start) {
      varTimeTmp = varTime; // conversion failed
    }
    expr.resize(atpos); // strip everything after '@'
  }

  parseColorString(expr, varNewColor);
  return;
}


void DynamicColor::update(double t)
{
  // variables take priority
  if (!varName.empty()) {
    colorByVariable(t);
    return;
  }

  // followed by states
  if (!colorStates.empty()) {
    colorByStates(t);
    return;
  }
}


void DynamicColor::setColor(const fvec4& value)
{
  color = value;
}


void DynamicColor::colorByVariable(double /* t */)
{
  // process the variable value
  if (!varInit) {
    varInit = true;
    varTransition = false;
    string expr = BZDB.get(varName);
    string::size_type atpos = expr.find_first_of('@');
    if (atpos != string::npos) {
      expr.resize(atpos);
    }
    parseColorString(expr, color);
    varOldColor = color;
    varNewColor = color;
    BZDB.addCallback(varName, bzdbCallback, this);
  }

  // setup the color value
  if (varTransition) {
    const float diffTime = (float)(TimeKeeper::getTick() - varLastChange);
    if (diffTime < varTimeTmp) {
      const float newScale = (varTimeTmp > 0.0f) ? (diffTime / varTimeTmp) : 1.0f;
      const float oldScale = 1.0f - newScale;
      color = (oldScale * varOldColor) + (newScale * varNewColor);
    } else {
      // make sure the final color is set exactly
      varTransition = false;
      color = varNewColor;
    }
    if (varNoAlpha) {
      color[3] = (color[3] >= 0.5f) ? 1.0f : 0.0f;
    }
  }
}


void DynamicColor::colorByStates(double t)
{
  if ((colorStates.size() <= 1) ||
      (statesLength <= 0.0f)) {
    color = colorStates[0].color;
    return;
  }

  const float phase =
    (float)fmod((t + (double)statesDelay), (double)statesLength);

  int prevIndex = 0;
  float endTime = colorStates[0].duration;
  // finds the first element whose key is not less than the value
  map<float, int>::const_iterator it = colorEnds.lower_bound(phase);
  if (it != colorEnds.end()) {
    endTime = it->first;
    prevIndex = it->second;
  }

  const int nextIndex = (prevIndex + 1) % colorStates.size();

  const ColorState& prev = colorStates[prevIndex];
  const ColorState& next = colorStates[nextIndex];

  if (prev.duration <= 0.0f) {
    color = next.color;
    return;
  }

  const float left = endTime - phase;
  const float pf = left / prev.duration;
  const float nf = 1.0f - pf;

  color = (pf * prev.color) + (nf * next.color);
}


void* DynamicColor::pack(void *buf) const
{
  buf = nboPackStdString(buf, name);

  buf = nboPackStdString(buf, varName);
  buf = nboPackFloat(buf, varTime);
  buf = nboPackUByte(buf, varNoAlpha ? 1 : 0);

  buf = nboPackFloat(buf, statesDelay);
  buf = nboPackUInt(buf, (uint32_t)colorStates.size());
  for (size_t i = 0; i < colorStates.size(); i++) {
    const ColorState& state = colorStates[i];
    buf = nboPackFloat(buf, state.color[0]);
    buf = nboPackFloat(buf, state.color[1]);
    buf = nboPackFloat(buf, state.color[2]);
    buf = nboPackFloat(buf, state.color[3]);
    buf = nboPackFloat(buf, state.duration);
  }

  return buf;
}


void* DynamicColor::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  uint8_t u8;
  buf = nboUnpackStdString(buf, varName);
  buf = nboUnpackFloat(buf, varTime);
  buf = nboUnpackUByte(buf, u8);
  varNoAlpha = (u8 != 0);

  uint32_t statesCount;
  buf = nboUnpackFloat(buf, statesDelay);
  buf = nboUnpackUInt(buf, statesCount);
  for (size_t i = 0; i < statesCount; i++) {
    ColorState state;
    buf = nboUnpackFloat(buf, state.color[0]);
    buf = nboUnpackFloat(buf, state.color[1]);
    buf = nboUnpackFloat(buf, state.color[2]);
    buf = nboUnpackFloat(buf, state.color[3]);
    buf = nboUnpackFloat(buf, state.duration);
    colorStates.push_back(state);
  }

  finalize();

  return buf;
}


int DynamicColor::packSize() const
{
  int fullSize = 0;

  fullSize += nboStdStringPackSize(name);

  fullSize += nboStdStringPackSize(varName);
  fullSize += sizeof(float);   // varTime
  fullSize += sizeof(uint8_t); // varNoAlpha

  fullSize += sizeof(float);    // states delay
  fullSize += sizeof(uint32_t); // states count
  fullSize += sizeof(float) * 5 * (int)colorStates.size(); // states data

  return fullSize;
}


void DynamicColor::print(std::ostream& out, const string& indent) const
{
  out << indent << "dynamicColor" << std::endl;

  if (!name.empty()) {
    out << indent << "  name " << name << std::endl;
  }

  if (!varName.empty()) {
    out << indent << "  varName " << varName << std::endl;
    if (varTime != 1.0f) {
      out << indent << "  varTime " << varTime << std::endl;
    }
    if (varNoAlpha) {
      out << indent << "  varNoAlpha " << std::endl;
    }
  }

  if (statesDelay != 0.0f) {
    out << indent << "  delay " << statesDelay << std::endl;
  }
  for (size_t i = 0; i < colorStates.size(); i++) {
    const char* keyword = "  ramp ";
    const ColorState& state = colorStates[i];
    if ((i + 1) < colorStates.size()) {
      const ColorState& nextState = colorStates[i + 1];
      if ((nextState.duration <= 0.0f) &&
          (memcmp(state.color, nextState.color, sizeof(float[4])) == 0)) {
        keyword = "  level ";
        i++;
      }
    }
    out << indent << keyword
                  << state.duration << " "
                  << state.color[0] << " "
                  << state.color[1] << " "
                  << state.color[2] << " "
                  << state.color[3] << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
