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

#ifndef _DYNAMIC_COLOR_H_
#define _DYNAMIC_COLOR_H_


#include "TimeKeeper.h"

#include <vector>
#include <iostream>


class DynamicColor {
  public:
    DynamicColor();
    ~DynamicColor();

    void setLimits(int channel, float min, float max);
    void setSinusoid(int channel, float period, float offset);
    void setClampUp(int channel, float period, float offset, float width);
    void setClampDown(int channel, float period, float offset, float width);

    void finalize();
    void update(float time);

    bool canHaveAlpha() const;
    const float* getColor() const;
    
    void* pack(void*);
    void* unpack(void*);
    int packSize();
    
    void print(std::ostream& out, int level);
  
  private:
    float color[4];

    typedef struct {
      float minValue, maxValue;
      float sinusoidPeriod, sinusoidOffset;
      float clampUpPeriod, clampUpOffset, clampUpWidth;
      float clampDownPeriod, clampDownOffset, clampDownWidth;
    } ChannelParams;
    ChannelParams channels[4];
    bool possibleAlpha;
};

inline bool DynamicColor::canHaveAlpha() const
{
  return possibleAlpha;
}

inline const float* DynamicColor::getColor() const {
  return color;
}


class DynamicColorManager {
  public:
    DynamicColorManager();
    ~DynamicColorManager();
    void update();
    void clear();
    int addColor(DynamicColor* dyncolor);
    DynamicColor* getColor(int id);

    void* pack(void*);
    void* unpack(void*);
    int packSize();
    
    void print(std::ostream& out, int level);

  private:
    std::vector<DynamicColor*> colors;
    TimeKeeper startTime;
};

extern DynamicColorManager DYNCOLORMGR;


#endif //_DYNAMIC_COLOR_H_
