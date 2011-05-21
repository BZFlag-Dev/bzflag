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

/* interface header */
#include "DynamicColor.h"

/* system implementation headers */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

/* common implementation headers */
#include "global.h"
#include "GameTime.h"
#include "Pack.h"
#include "TextUtils.h"
#include "BzTime.h"
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
: teamMask(0)
, oldTeamMask(0)
{
}


DynamicColorManager::~DynamicColorManager()
{
  clear();
}


void DynamicColorManager::clear()
{
  std::vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    delete *it;
  }
  colors.clear();
  active.clear();
  inactive.clear();
  teamMask = 0;
  oldTeamMask = 0;
}


void DynamicColorManager::update()
{
  const double gameTime = GameTime::getStepTime();
  if (teamMask != oldTeamMask) { // update them all
    for (size_t i = 0; i < colors.size(); i++) {
      colors[i]->update(gameTime);
    }
  }
  else { // update the ones that need it
    std::set<DynamicColor*>::iterator it, nextIt;
    for (it = active.begin(); it != active.end(); it = nextIt) {
      nextIt = it;
      nextIt++; // be wary of setInactive()
      (*it)->update(gameTime);
    }
  }
  oldTeamMask = teamMask;
}


void DynamicColorManager::setActive(DynamicColor* color)
{
  std::set<DynamicColor*>::iterator it = inactive.find(color);
  if (it != inactive.end()) {
    inactive.erase(it);
    active.insert(color);
  }
}


void DynamicColorManager::setInactive(DynamicColor* color)
{
  std::set<DynamicColor*>::iterator it = active.find(color);
  if (it != active.end()) {
    active.erase(it);
    inactive.insert(color);
  }
}


int DynamicColorManager::addColor(DynamicColor* color)
{
  active.insert(color);
  colors.push_back(color);
  return ((int)colors.size() - 1);
}


int DynamicColorManager::findColor(const std::string& name) const
{
  if (name.empty()) {
    return -1;
  }
  else if ((name[0] >= '0') && (name[0] <= '9')) {
    int index = atoi(name.c_str());
    if ((index < 0) || (index >= (int)colors.size())) {
      return -1;
    } else {
      return index;
    }
  } else {
    for (int i = 0; i < (int)colors.size(); i++) {
      if (colors[i]->getName() == name) {
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


void DynamicColorManager::getVariables(std::set<std::string>& vars) const
{
  std::vector<DynamicColor*>::const_iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    if (!color->varName.empty()) {
      vars.insert(color->varName);
    }
  }
}


int DynamicColorManager::packSize() const
{
  int fullSize = sizeof (uint32_t);
  std::vector<DynamicColor*>::const_iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    fullSize = fullSize + color->packSize();
  }
  return fullSize;
}


void* DynamicColorManager::pack(void *buf) const
{
  std::vector<DynamicColor*>::const_iterator it;
  buf = nboPackUInt32(buf, (int)colors.size());
  for (it = colors.begin(); it != colors.end(); it++) {
    const DynamicColor* color = *it;
    buf = color->pack(buf);
  }
  return buf;
}


void* DynamicColorManager::unpack(void *buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt32 (buf, count);
  for (i = 0; i < count; i++) {
    DynamicColor* color = new DynamicColor;
    buf = color->unpack(buf);
    addColor(color);
  }
  return buf;
}


void DynamicColorManager::print(std::ostream& out, const std::string& indent) const
{
  std::vector<DynamicColor*>::const_iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    color->print(out, indent);
  }
}


void DynamicColorManager::setVisualTeam(int team)
{
  oldTeamMask = teamMask;
  teamMask = (1 << team);
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

  teamMask = 0;

  varName = "";
  varTime = 1.0f;
  varNoAlpha = false;

  varInit = false;
  varTimeTmp = varTime;
  varTransition = false;
  varOldColor = white;
  varNewColor = white;
  varOldStates = false;
  varNewStates = false;
  varLastChange = BzTime::getSunGenesisTime();

  statesDelay = 0.0f;
}


DynamicColor::~DynamicColor()
{
  if (varInit) {
    BZDB.removeCallback(varName, bzdbCallback, this);
  }
}


void DynamicColor::finalize()
{
  possibleAlpha = false;

  // variables take priority
  if (!varName.empty()) {
    possibleAlpha = !varNoAlpha;
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

      const float alpha = state.color.a;
      if (((alpha > 0.0f) && (alpha < 1.0f)) ||
          ((alpha != next.color.a) && (state.duration > 0.0f))) {
        possibleAlpha = true;
      }
    }
  }
}


bool DynamicColor::setName(const std::string& newName)
{
  if (newName.empty()) {
    name = "";
    return false;
  } else if ((newName[0] >= '0') && (newName[0] <= '9')) {
    name = "";
    return false;
  } else {
    name = newName;
  }
  return true;
}


const std::string& DynamicColor::getName() const
{
  return name;
}


void DynamicColor::setVariableName(const std::string& vName)
{
  varName = vName;
}


void DynamicColor::setVariableTiming(float seconds)
{
  varTime = seconds;
}


void DynamicColor::setVariableNoAlpha(bool value)
{
  varNoAlpha = value;
}


void DynamicColor::setDelay(float delay)
{
  statesDelay = delay;
}


void DynamicColor::addState(float duration, const fvec4& _color)
{
  if (duration < 0.0f) {
    duration = 0.0f;
  }
  colorStates.push_back(ColorState(_color, duration));
}


void DynamicColor::clearStates()
{
  colorStates.clear();
}


void DynamicColor::bzdbCallback(const std::string& /*varName*/, void* data)
{
  ((DynamicColor*)data)->updateVariable();
}


void DynamicColor::updateVariable()
{
  // setup the basics
  varTransition = true;
  varLastChange = BzTime::getTick();
  varOldColor = color;
  varOldStates = varNewStates;

  std::string expr = BZDB.get(varName);

  // parse the optional delay timing
  std::string::size_type atpos = expr.find_first_of('@');
  if (atpos == std::string::npos) {
    varTimeTmp = varTime;
  }
  else {
    char* end;
    const char* start = expr.c_str() + atpos + 1;
    varTimeTmp = (float)strtod(start, &end);
    if ((end == start) || isnan(varTimeTmp)) {
      varTimeTmp = varTime; // conversion failed or invalid number
    }
    expr.resize(atpos); // strip everything after '@'
  }

  // parse the new color
  if (expr != "*") {
    parseColorString(expr, varNewColor);
    varNewStates = false;
  } else {
    varNewStates = true;
  }

  DYNCOLORMGR.setActive(this);
}


void DynamicColor::update(double t)
{
  // teamMask
  if ((DYNCOLORMGR.getTeamMask() & teamMask) != 0) {
    color.a = 0.0f;
    return;
  }

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


void DynamicColor::colorByVariable(double t)
{
  // process the variable value
  if (!varInit) {
    varInit = true;
    varTransition = false;
    BZDB.addCallback(varName, bzdbCallback, this);

    std::string expr = BZDB.get(varName);
    std::string::size_type atpos = expr.find_first_of('@');
    if (atpos != std::string::npos) {
      expr.resize(atpos); // strip the delay specification
    }

    if (expr != "*") {
      DYNCOLORMGR.setInactive(this);
      parseColorString(expr, color);
      varOldColor = color;
      varNewColor = color;
      varOldStates = varNewStates = false;
    } else {
      varOldStates = varNewStates = true;
    }
  }

  // setup the color value
  if (!varTransition) {
    if (varNewStates) {
      colorByStates(t);
    }
    return;
  }

  // transitioning
  const float diffTime = (float)(BzTime::getTick() - varLastChange);
  if (diffTime < varTimeTmp) {
    const float newScale = (varTimeTmp > 0.0f) ?
                           (diffTime / varTimeTmp) : 1.0f;
    const float oldScale = 1.0f - newScale;
    if (varOldStates) { colorByStates(t); varOldColor = color; }
    if (varNewStates) { colorByStates(t); varNewColor = color; }
    color = (oldScale * varOldColor) + (newScale * varNewColor);
  }
  else { // complete the transition
    varTransition = false;
    if (!varNewStates) {
      DYNCOLORMGR.setInactive(this);
      // make sure the final color is set exactly
      color = varNewColor;
    }
    else {
      colorByStates(t);
      if (colorStates.size() <= 1) {
        DYNCOLORMGR.setInactive(this);
      }
    }
  }

  // clamp alpha
  if (varNoAlpha) {
    color.a = (color.a >= 0.5f) ? 1.0f : 0.0f;
  }
}


void DynamicColor::colorByStates(double t)
{
  if ((colorStates.size() <= 1) ||
      (statesLength <= 0.0f)) {
    color = colorStates[0].color;
    if (varName.empty()) {
      DYNCOLORMGR.setInactive(this);
    }
    return;
  }

  const float phase =
    (float)fmod((t + (double)statesDelay), (double)statesLength);

  int prevIndex = 0;
  float endTime = colorStates[0].duration;
  // finds the first element whose key is not less than the value
  std::map<float, int>::const_iterator it = colorEnds.lower_bound(phase);
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

  buf = nboPackInt32(buf, teamMask);

  buf = nboPackStdString(buf, varName);
  buf = nboPackFloat(buf, varTime);
  buf = nboPackUInt8(buf, varNoAlpha ? 1 : 0);

  buf = nboPackFloat(buf, statesDelay);
  buf = nboPackUInt32(buf, (uint32_t)colorStates.size());
  for (size_t i = 0; i < colorStates.size(); i++) {
    const ColorState& state = colorStates[i];
    buf = nboPackFVec4(buf, state.color);
    buf = nboPackFloat(buf, state.duration);
  }

  return buf;
}


void* DynamicColor::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  int32_t mask;
  buf = nboUnpackInt32(buf, mask);
  teamMask = mask;

  uint8_t u8;
  buf = nboUnpackStdString(buf, varName);
  buf = nboUnpackFloat(buf, varTime);
  buf = nboUnpackUInt8(buf, u8);
  varNoAlpha = (u8 != 0);

  uint32_t statesCount;
  buf = nboUnpackFloat(buf, statesDelay);
  buf = nboUnpackUInt32(buf, statesCount);
  for (size_t i = 0; i < statesCount; i++) {
    ColorState state;
    buf = nboUnpackFVec4(buf, state.color);
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

  fullSize += sizeof(int32_t); // teamMask

  fullSize += nboStdStringPackSize(varName);
  fullSize += sizeof(float);   // varTime
  fullSize += sizeof(uint8_t); // varNoAlpha

  fullSize += sizeof(float);    // states delay
  fullSize += sizeof(uint32_t); // states count
  fullSize += (int)colorStates.size() * (
                sizeof(fvec4) + // color
                sizeof(float)   // duration
              );

  return fullSize;
}


bool DynamicColor::setTeamMask(const std::string& maskStr)
{
  if (maskStr.empty()) {
    return false;
  }

  const std::string lower = TextUtils::tolower(maskStr);
  const std::vector<std::string> args = TextUtils::tokenize(lower, " \t");

  int mask = 0;

  for (size_t i = 0; i < args.size(); i++) {
    std::string arg = args[i];
    bool invert = false;
    if (!arg.empty() && (arg[0] == '-')) {
      invert = true;
      arg = arg.substr(1);
    }

    int bits = 0;
         if (arg == "all")      { bits = ~0; } // all bits
    else if (arg == "rogue")    { bits = (1 << RogueTeam);    } // 1
    else if (arg == "red")      { bits = (1 << RedTeam);      } // 2
    else if (arg == "green")    { bits = (1 << GreenTeam);    } // 4
    else if (arg == "blue")     { bits = (1 << BlueTeam);     } // 8
    else if (arg == "purple")   { bits = (1 << PurpleTeam);   } // 16
    else if (arg == "observer") { bits = (1 << ObserverTeam); } // 32
    else if (arg == "rabbit")   { bits = (1 << RabbitTeam);   } // 64
    else if (arg == "hunter")   { bits = (1 << HunterTeam);   } // 128
    else {
      return false;
    }

    if (!invert) {
      mask |= bits;
    } else {
      mask &= ~bits;
    }
  }

  teamMask = mask;

  return true;
}


std::string DynamicColor::teamMaskString() const
{
  std::string s;
  if (teamMask & (1 << RogueTeam))    { s += " rogue";    }
  if (teamMask & (1 << RedTeam))      { s += " red";      }
  if (teamMask & (1 << GreenTeam))    { s += " green";    }
  if (teamMask & (1 << BlueTeam))     { s += " blue";     }
  if (teamMask & (1 << PurpleTeam))   { s += " purple";   }
  if (teamMask & (1 << ObserverTeam)) { s += " observer"; }
  if (teamMask & (1 << RabbitTeam))   { s += " rabbit";   }
  if (teamMask & (1 << HunterTeam))   { s += " hunter";   }
  return s;
}


void DynamicColor::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "dynamicColor" << std::endl;

  if (!name.empty()) {
    out << indent << "  name " << name << std::endl;
  }

  if (teamMask != 0) {
    out << indent << "  teamMask" << teamMaskString() << std::endl;
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

  if ((colorStates.size() == 1) && (colorStates[0].duration == 0.0f)) {
    out << indent << "  color " << colorStates[0].color << std::endl;
  }
  else {
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
                    << state.color<< std::endl;
    }
  }

  out << indent << "end" << std::endl << std::endl;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
