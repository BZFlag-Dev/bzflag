/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "DynamicColor.h"

#include <math.h>
#include <string.h>
#include <vector>

#include "TimeKeeper.h"
#include "Pack.h"


// this applies to sinusoid and clamp functions
const float DynamicColor::minPeriod = 0.1f;


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
  std::vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    delete *it;
  }
  colors.clear();
  return;
}


void DynamicColorManager::update()
{
  float t = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();
  std::vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* texmat = *it;
    texmat->update(t);
  }
  return;
}


int DynamicColorManager::addColor(DynamicColor* color)
{
  colors.push_back (color);
  return ((int)colors.size() - 1);
}


DynamicColor* DynamicColorManager::getColor(int id)
{
  if ((id >= 0) && (id < (int)colors.size())) {
    return colors[id];
  } else {
    return NULL;
  }
}


void * DynamicColorManager::pack(void *buf)
{
  std::vector<DynamicColor*>::iterator it;
  buf = nboPackUInt(buf, (int)colors.size());
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    buf = color->pack(buf);
  }
  return buf;
}


void * DynamicColorManager::unpack(void *buf)
{
  unsigned int i, count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    DynamicColor* color = new DynamicColor;
    buf = color->unpack(buf);
    addColor(color);
  }
  return buf;
}


int DynamicColorManager::packSize()
{
  int fullSize = sizeof (uint32_t);
  std::vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    fullSize = fullSize + color->packSize();
  }
  return fullSize;
}


void DynamicColorManager::print(std::ostream& out, int level)
{
  std::vector<DynamicColor*>::iterator it;
  for (it = colors.begin(); it != colors.end(); it++) {
    DynamicColor* color = *it;
    color->print(out, level);
  }
  return;
}


//
// Dynamic Color
//

DynamicColor::DynamicColor()
{
  // setup the channels
  for (int c = 0; c < 4; c++) {
    // the parameters are setup so that all channels 
    // are at 1.0f, and that there are no variations
    color[c] = 1.0f;
    channels[c].minValue = 0.0f;
    channels[c].maxValue = 1.0f;
  }
  possibleAlpha = false;
  return;
}


DynamicColor::~DynamicColor()
{
}


void DynamicColor::finalize()
{
  const ChannelParams& p = channels[3]; // the alpha channel
  if ((p.sinusoids.size() == 0) && 
      (p.clampUps.size() == 0) &&
      (p.clampDowns.size() == 0)) {
    // not using any functions
    if (p.maxValue != 1.0f) {
      possibleAlpha = true;
    } else {
      possibleAlpha = false;
    }
  } else {
    // functions are used
    if ((p.minValue != 1.0f) || (p.maxValue != 1.0f)) {
      possibleAlpha = true;
    } else {
      possibleAlpha = false;
    }
  }
  return;
}


void DynamicColor::setLimits(int channel, float min, float max)
{
  if (min < 0.0f) {
    min = 0.0f;
  }
  else if (min > 1.0f) {
    min = 1.0f;
  }

  if (max < 0.0f) {
    max = 0.0f;
  }
  else if (max > 1.0f) {
    max = 1.0f;
  }
  
  channels[channel].minValue = min;
  channels[channel].maxValue = max;
  
  return;
}


void DynamicColor::addSinusoid(int channel, const float sinusoid[3])
{
  sinusoidParams params;
  // check period and weight
  if ((sinusoid[0] >= minPeriod) && (sinusoid[2] > 0.0f)) {
    params.period = sinusoid[0];
    params.offset = sinusoid[1];
    params.weight = sinusoid[2];
    channels[channel].sinusoids.push_back(params);
  }
  return;
}


void DynamicColor::addClampUp(int channel, const float clampUp[3])
{
  clampParams params;
  // check period and width
  if ((clampUp[0] >= minPeriod) && (clampUp[2] > 0.0f)) {
    params.period = clampUp[0];
    params.offset = clampUp[1];
    params.width = clampUp[2];
    channels[channel].clampUps.push_back(params);
  }
  return;
}


void DynamicColor::addClampDown(int channel, const float clampDown[3])
{
  clampParams params;
  // check period and width
  if ((clampDown[0] >= minPeriod) && (clampDown[2] > 0.0f)) {
    params.period = clampDown[0];
    params.offset = clampDown[1];
    params.width = clampDown[2];
    channels[channel].clampDowns.push_back(params);
  }
  return;
}


void DynamicColor::update (float t)
{
  for (int c = 0; c < 4; c++) {
    const ChannelParams& channel = channels[c];
    unsigned int i;
    bool clampUp = false;
    bool clampDown = false;
    
    // check for active clampUp
    for (i = 0; i < channel.clampUps.size(); i++) {
      const clampParams& clamp = channel.clampUps[i];
      float upTime = (t - clamp.offset);
      if (upTime < 0.0f) {
        upTime -= clamp.period * floorf(upTime / clamp.period);
      }
      upTime = fmodf (upTime, clamp.period);
      if (upTime < clamp.width) {
        clampUp = true;
        break;
      }
    }
      
    // check for active clampDown
    for (i = 0; i < channel.clampDowns.size(); i++) {
      const clampParams& clamp = channel.clampDowns[i];
      float downTime = (t - clamp.offset);
      if (downTime < 0.0f) {
        downTime -= clamp.period * floorf(downTime / clamp.period);
      }
      downTime = fmodf (downTime, clamp.period);
      if (downTime < clamp.width) {
        clampDown = true;
        break;
      }
    }

    // the amount of 'max' in the resultant channel's value
    float factor = 1.0f;
    
    // check the clamps
    if (clampUp && clampDown) {
      factor = 0.5f;
    }
    else if (clampUp) {
      factor = 1.0f;
    }
    else if (clampDown) {
      factor = 0.0f;
    }
    // no clamps, try sinusoids
    else if (channel.sinusoids.size() > 0) {
      float value = 0.0f;
      for (i = 0; i < channel.sinusoids.size(); i++) {
        const sinusoidParams& s = channel.sinusoids[i];
        value += s.weight * cos (((t - s.offset) / s.period) * (M_PI * 2.0f));
      }
      // center the factor
      factor = 0.5f + (0.5f * value);
      if (factor < 0.0f) {
        factor = 0.0f;
      } 
      else if (factor > 1.0f) {
        factor = 1.0f;
      }
    }
    
    color[c] = (channel.minValue * (1.0f - factor)) +
               (channel.maxValue * factor);
  }

  return;
}


void * DynamicColor::pack(void *buf)
{
  for (int c = 0; c < 4; c++) {
    ChannelParams& p = channels[c];
    unsigned int i;

    buf = nboPackFloat (buf, p.minValue);
    buf = nboPackFloat (buf, p.maxValue);

    // sinusoids
    buf = nboPackUInt (buf, (unsigned int)p.sinusoids.size());
    for (i = 0; i < p.sinusoids.size(); i++) {
      buf = nboPackFloat (buf, p.sinusoids[i].period);
      buf = nboPackFloat (buf, p.sinusoids[i].offset);
      buf = nboPackFloat (buf, p.sinusoids[i].weight);
    }
    // clampUps
    buf = nboPackUInt (buf, (unsigned int)p.clampUps.size());
    for (i = 0; i < p.clampUps.size(); i++) {
      buf = nboPackFloat (buf, p.clampUps[i].period);
      buf = nboPackFloat (buf, p.clampUps[i].offset);
      buf = nboPackFloat (buf, p.clampUps[i].width);
    }
    // clampDowns
    buf = nboPackUInt (buf, (unsigned int)p.clampDowns.size());
    for (i = 0; i < p.clampDowns.size(); i++) {
      buf = nboPackFloat (buf, p.clampDowns[i].period);
      buf = nboPackFloat (buf, p.clampDowns[i].offset);
      buf = nboPackFloat (buf, p.clampDowns[i].width);
    }
  }

  return buf;
}


void * DynamicColor::unpack(void *buf)
{
  for (int c = 0; c < 4; c++) {
    ChannelParams& p = channels[c];
    unsigned int i, size;

    buf = nboUnpackFloat (buf, p.minValue);
    buf = nboUnpackFloat (buf, p.maxValue);

    // sinusoids
    buf = nboUnpackUInt (buf, size);
    p.sinusoids.resize(size);
    for (i = 0; i < size; i++) {
      buf = nboUnpackFloat (buf, p.sinusoids[i].period);
      buf = nboUnpackFloat (buf, p.sinusoids[i].offset);
      buf = nboUnpackFloat (buf, p.sinusoids[i].weight);
    }
    // clampUps
    buf = nboUnpackUInt (buf, size);
    p.clampUps.resize(size);
    for (i = 0; i < size; i++) {
      buf = nboUnpackFloat (buf, p.clampUps[i].period);
      buf = nboUnpackFloat (buf, p.clampUps[i].offset);
      buf = nboUnpackFloat (buf, p.clampUps[i].width);
    }
    // clampDowns
    buf = nboUnpackUInt (buf, size);
    p.clampDowns.resize(size);
    for (i = 0; i < size; i++) {
      buf = nboUnpackFloat (buf, p.clampDowns[i].period);
      buf = nboUnpackFloat (buf, p.clampDowns[i].offset);
      buf = nboUnpackFloat (buf, p.clampDowns[i].width);
    }
  }

  finalize();

  return buf;
}


int DynamicColor::packSize()
{
  int fullSize = 0;
  for (int c = 0; c < 4; c++) {
    fullSize += sizeof(float) * 2; // the limits
    fullSize += sizeof(unsigned int);
    fullSize += (int)(channels[c].sinusoids.size() * (sizeof(sinusoidParams)));
    fullSize += sizeof(unsigned int);
    fullSize += (int)(channels[c].clampUps.size() * (sizeof(clampParams)));
    fullSize += sizeof(unsigned int);
    fullSize += (int)(channels[c].clampDowns.size() * (sizeof(clampParams)));
  }
  return fullSize;
}


void DynamicColor::print(std::ostream& out, int /*level*/)
{
  const char *colorStrings[4] = { "red", "green", "blue", "alpha" };
    
  out << "dynamicColor" << std::endl;
  for (int c = 0; c < 4; c++) {
    const char *colorStr = colorStrings[c];
    const ChannelParams& p = channels[c];
    if ((p.minValue != 0.0f) || (p.maxValue != 1.0f)) {
      out << "  " << colorStr << " limits " 
          << p.minValue << " " << p.maxValue << std::endl;
    }
    unsigned int i;
    for (i = 0; i < p.sinusoids.size(); i++) {
      const sinusoidParams& f = p.sinusoids[i];
      out << "  " << colorStr << " sinusoid " 
          << f.period << " " << f.offset << " " << f.weight << std::endl;
    }
    for (i = 0; i < p.clampUps.size(); i++) {
      const clampParams& f = p.clampUps[i];
      out << "  " << colorStr << " clampup " 
          << f.period << " " << f.offset << " " << f.width << std::endl;
    }
    for (i = 0; i < p.clampDowns.size(); i++) {
      const clampParams& f = p.clampDowns[i];
      out << "  " << colorStr << " clampdown " 
          << f.period << " " << f.offset << " " << f.width << std::endl;
    }
  }

  out << "end" << std::endl << std::endl;
  
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

