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


//
// Dynamic Color Manager
//

DynamicColorManager DYNCOLORMGR;


DynamicColorManager::DynamicColorManager()
{
  startTime = TimeKeeper::getCurrent();
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
  float t = TimeKeeper::getCurrent() - startTime;
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
    channels[c].sinusoidPeriod = 0.0f;
    channels[c].sinusoidOffset = 0.0f;
    channels[c].clampUpPeriod = 0.0f;
    channels[c].clampUpOffset = 0.0f;
    channels[c].clampUpWidth = 0.0f;
    channels[c].clampDownPeriod = 0.0f;
    channels[c].clampDownOffset = 0.0f;
    channels[c].clampDownWidth = 0.0f;
  }
  possibleAlpha = false;
  return;
}


DynamicColor::~DynamicColor()
{
}


void DynamicColor::finalize()
{
  // check if alpha can not be '1.0f' at some time
  const ChannelParams& p = channels[3];
  if ((p.sinusoidPeriod == 0.0f) && 
      (p.clampUpPeriod == 0.0f) &&
      (p.clampDownPeriod == 0.0f)) {
    if (p.maxValue != 1.0f) {
      possibleAlpha = true;
    } else {
      possibleAlpha = false;
    }
  } else {
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
  if ((channel < 0) || (channel > 3)) {
    return;
  }

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


void DynamicColor::setSinusoid(int channel, float period, float offset)
{
  if ((channel < 0) || (channel > 3)) {
    return;
  }
  channels[channel].sinusoidPeriod = period;
  channels[channel].sinusoidOffset = offset;
  return;
}


void DynamicColor::setClampUp(int channel,
                              float period, float offset, float width)
{
  if ((channel < 0) || (channel > 3)) {
    return;
  }
  channels[channel].clampUpPeriod = period;
  channels[channel].clampUpOffset = offset;
  channels[channel].clampUpWidth = width;
  return;
}


void DynamicColor::setClampDown(int channel,
                                float period, float offset, float width)
{
  if ((channel < 0) || (channel > 3)) {
    return;
  }
  channels[channel].clampDownPeriod = period;
  channels[channel].clampDownOffset = offset;
  channels[channel].clampDownWidth = width;
  return;
}


void DynamicColor::update (float t)
{
  for (int c = 0; c < 4; c++) {
    const float shortestPeriod = 0.1f;
    const ChannelParams& p = channels[c];
    bool clampUp = false;
    bool clampDown = false;
    float factor = 1.0f;

    if (fabsf(p.clampUpPeriod) >= shortestPeriod) {
      float upTime = (t - p.clampUpOffset);
      if (upTime < 0.0f) {
        upTime -= p.clampUpPeriod * floorf(upTime / p.clampUpPeriod);
      }
      upTime = fmodf (upTime, p.clampUpPeriod);
      if (upTime < p.clampUpWidth) {
        clampUp = true;
      }
    }
      
    if (fabsf(p.clampDownPeriod) >= shortestPeriod) {
      float downTime = (t - p.clampDownOffset);
      if (downTime < 0.0f) {
        downTime -= p.clampDownPeriod * floorf(downTime / p.clampDownPeriod);
      }
      downTime = fmodf (downTime, p.clampDownPeriod);
      if (downTime < p.clampDownWidth) {
        clampDown = true;
      }
    }

    if (clampUp && clampDown) {
      factor = 0.5f;
    }
    else if (clampUp) {
      factor = 1.0f;
    }
    else if (clampDown) {
      factor = 0.0f;
    }
    else {
      if (fabsf(p.sinusoidPeriod) >= shortestPeriod) {
        factor = 0.5f + (0.5f * cos ((M_PI * 2.0f) * (t - p.sinusoidOffset) 
                                     / p.sinusoidPeriod));
      }
    }
    
    color[c] = ((1.0f - factor) * p.minValue) + (factor * p.maxValue);
  }

  return;
}


void * DynamicColor::pack(void *buf)
{
  for (int c = 0; c < 4; c++) {
    buf = nboPackFloat (buf, channels[c].minValue);
    buf = nboPackFloat (buf, channels[c].maxValue);
    buf = nboPackFloat (buf, channels[c].sinusoidPeriod);
    buf = nboPackFloat (buf, channels[c].sinusoidOffset);
    buf = nboPackFloat (buf, channels[c].clampUpPeriod);
    buf = nboPackFloat (buf, channels[c].clampUpOffset);
    buf = nboPackFloat (buf, channels[c].clampUpWidth);
    buf = nboPackFloat (buf, channels[c].clampDownPeriod);
    buf = nboPackFloat (buf, channels[c].clampDownOffset);
    buf = nboPackFloat (buf, channels[c].clampDownWidth);
  }

  return buf;
}


void * DynamicColor::unpack(void *buf)
{
  for (int c = 0; c < 4; c++) {
    buf = nboUnpackFloat (buf, channels[c].minValue);
    buf = nboUnpackFloat (buf, channels[c].maxValue);
    buf = nboUnpackFloat (buf, channels[c].sinusoidPeriod);
    buf = nboUnpackFloat (buf, channels[c].sinusoidOffset);
    buf = nboUnpackFloat (buf, channels[c].clampUpPeriod);
    buf = nboUnpackFloat (buf, channels[c].clampUpOffset);
    buf = nboUnpackFloat (buf, channels[c].clampUpWidth);
    buf = nboUnpackFloat (buf, channels[c].clampDownPeriod);
    buf = nboUnpackFloat (buf, channels[c].clampDownOffset);
    buf = nboUnpackFloat (buf, channels[c].clampDownWidth);
  }
  
  finalize();

  return buf;
}


int DynamicColor::packSize()
{
  return sizeof(channels);
}


void DynamicColor::print(std::ostream& out, int /*level*/)
{
  const char *colorStrings[4] = { "red", "green", "blue", "alpha" };
    
  out << "dynamicColor" << std::endl;
  for (int c = 0; c < 4; c++) {
    const ChannelParams& p = channels[c];
    const char *colorStr = colorStrings[c];
    if ((p.minValue != 0.0f) || (p.maxValue != 1.0f)) {
      out << "  " << colorStr 
          << " limits " << p.minValue << " " << p.maxValue << std::endl;
    }
    if (p.sinusoidPeriod != 0.0f) {
      out << "  " << colorStr 
          << " sinusoid " << p.sinusoidPeriod << " "
                          << p.sinusoidOffset << std::endl;
    }
    if (p.clampUpPeriod != 0.0f) {
      out << "  " << colorStr
          << " clampUp " << p.clampUpPeriod << " "
                         << p.clampUpOffset << " "
                         << p.clampUpWidth << std::endl;
    }
    if (p.clampDownPeriod != 0.0f) {
      out << "  " << colorStr
          << " clampDown " << p.clampDownPeriod << " "
                           << p.clampDownOffset << " "
                           << p.clampDownWidth << std::endl;
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

