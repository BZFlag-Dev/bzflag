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

#ifndef _DYNAMIC_COLOR_H_
#define _DYNAMIC_COLOR_H_

#include "common.h"

/* system interface headers */
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

/* common interface headers */
#include "BzTime.h"
#include "vectors.h"


class DynamicColor {

    friend class DynamicColorManager;

  public:
    DynamicColor();
    ~DynamicColor();

    bool setName(const std::string& name);

    bool setTeamMask(const std::string& maskStr);

    void setVariableName(const std::string& name);
    void setVariableTiming(float seconds);
    void setVariableNoAlpha(bool);

    void setDelay(float delay);

    void addState(float duration, const fvec4& color);
    void clearStates();

    void finalize();

    void update(double time);
    void setColor(const fvec4& color);

    bool canHaveAlpha() const;
    const fvec4& getColor() const;
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

    std::string teamMaskString() const;

  private:
    static const float minPeriod;

    std::string name;

    fvec4 color;
    bool possibleAlpha;

    int teamMask;

    std::string varName;
    bool        varNoAlpha;
    float       varTime;
    bool        varInit;
    bool        varTransition;
    float       varTimeTmp;
    fvec4       varOldColor;
    fvec4       varNewColor;
    bool        varOldStates;
    bool        varNewStates;
    BzTime  varLastChange;

    struct ColorState {
      ColorState() {
        color = fvec4(1.0f, 1.0f, 1.0f, 1.0f);
        duration = 0.0f;
      }
      ColorState(const fvec4& c, float d) {
        color = c;
        duration = d;
      }
      fvec4 color;
      float duration;
    };
    std::vector<ColorState> colorStates;
    std::map<float, int> colorEnds;
    float statesDelay;
    float statesLength;
};

inline bool DynamicColor::canHaveAlpha() const {
  return possibleAlpha;
}

inline const fvec4& DynamicColor::getColor() const {
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

    void getVariables(std::set<std::string>& vars) const;

    void setVisualTeam(int team);
    inline int getTeamMask() const { return teamMask; }

    void setActive(DynamicColor* dyncol);
    void setInactive(DynamicColor* dyncol);

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    std::vector<DynamicColor*> colors;
    std::set<DynamicColor*> active;
    std::set<DynamicColor*> inactive;

    int teamMask;
    int oldTeamMask;
};

extern DynamicColorManager DYNCOLORMGR;


#endif //_DYNAMIC_COLOR_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
