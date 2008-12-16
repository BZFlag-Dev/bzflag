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

#ifndef _DYNAMIC_COLOR_H_
#define _DYNAMIC_COLOR_H_

#include "common.h"

/* system interface headers */
#include <string>
#include <vector>
#include <map>
#include <iostream>

/* common interface headers */
#include "TimeKeeper.h"


class DynamicColor {
  public:
    DynamicColor();
    ~DynamicColor();

    bool setName(const std::string& name);

    void setVariableName(const std::string& name);
    void setVariableTiming(float seconds);
    void setVariableNoAlpha(bool);

    void setDelay(float delay);
    void addState(float duration, const float color[4]);
    void addState(float duration,
                  float r, float g, float b, float a);

    void finalize();

    void update(double time);
    void setColor(const float color[4]);

    bool canHaveAlpha() const;
    const float* getColor() const;
    const std::string& getName() const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    void colorByVariable(double t);
    void colorByStates(double t);

    void updateVariable();
    static void bzdbCallback(const std::string& varName, void* data);

  private:
    static const float minPeriod;

    std::string name;
    float color[4];

    std::string varName;

    bool varNoAlpha;
    float varTime;
    bool varInit;
    bool varTransition;
    float varTimeTmp;
    float varOldColor[4];
    float varNewColor[4];
    TimeKeeper varLastChange;

    struct ColorState {
      ColorState() {
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
        duration = 0.0f;
      }
      ColorState(const float c[4], float d) {
        color[0] = c[0];
        color[1] = c[1];
        color[2] = c[2];
        color[3] = c[3];
        duration = d;
      }
      float color[4];
      float duration;
    };
    std::vector<ColorState> colorStates;
    std::map<float, int> colorEnds;
    float statesDelay;
    float statesLength;

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
    int findColor(const std::string& name) const;
    const DynamicColor* getColor(int id) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::vector<DynamicColor*> colors;
};

extern DynamicColorManager DYNCOLORMGR;


#endif //_DYNAMIC_COLOR_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
