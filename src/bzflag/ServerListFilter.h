/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SERVERLISTFILTER_H__
#define __SERVERLISTFILTER_H__

#include "common.h"
#include <string>
#include <map>


class ServerItem;


class ServerListFilter {
private:
  struct BoolFilter {
    static bool parse(const std::string& s);
    BoolFilter() {
      reset();
    }
    void reset() {
      active = false; value = false;
    }
    bool check(bool v) const {
      return !active || (v == value);
    }
    void print(const std::string& name, const std::string& indent) const;
    bool active;
    bool value;
  };

  struct RangeFilter {
    static bool parse(const std::string& s);
    RangeFilter(): minValue(), maxValue() {
      reset();
    }
    void reset() {
      minActive = maxActive = false;
    }
    bool check(float v) const {
      return !(minActive && (v <= minValue)) &&
	     !(maxActive && (v >= maxValue));
    }
    bool check(int v) const {return check(float(v));}
    void print(const std::string& name, const std::string& indent) const;
    float minValue,  maxValue;
    bool  minActive, maxActive;
  };

  struct PatternFilter {
    enum PatternType {
      NoPattern    = 0,
      GlobPattern  = 1,
      RegexPattern = 2
    };
    static bool parse(const std::string& s);
    PatternFilter() : re(NULL) {
      reset();
    }
    ~PatternFilter() {
      reset();
    }
    bool setupGlob(const std::string& pattern, bool noCase);
    bool setupRegex(const std::string& pattern, bool noCase);
    void reset();
    bool check(const std::string& s) const;
    void print(const std::string& name, const std::string& indent) const;
    PatternType type;
    bool noCase;
    std::string pattern;
    regex_t* re;
  private: // no copying
    PatternFilter(const PatternFilter&);
    PatternFilter& operator=(const PatternFilter&);
  };

  //==========================================================================//

public:
  ServerListFilter();
  ServerListFilter(const std::string& filter);

  bool parse(const std::string& filter);

  const std::string& getSource() const {return source;}

  bool check(const ServerItem& item) const;

  bool operator==(const ServerListFilter& f) const {
    return (source == f.source);
  }
  bool operator!=(const ServerListFilter& f) const {
    return (source != f.source);
  }

  void print(const std::string& indent = "") const;

private: // no copying
  ServerListFilter(const ServerListFilter&);
  ServerListFilter& operator=(const ServerListFilter&);

public:
  static bool isBoolLabel(const std::string& label);
  static bool isRangeLabel(const std::string& label);
  static bool isPatternLabel(const std::string& label);
  static char parseFilterType(const std::string& f, char& op,
			      std::string& label, std::string& param);
  static std::string colorizeSearch(const std::string& in);

private:
  void reset();
  bool parseFilter(const std::string& f);
  bool parseBoolFilter(const std::string& label, char op);
  bool parseRangeFilter(const std::string& label, char op,
			const std::string& param);
  bool parsePatternFilter(const std::string& label, char op,
			  const std::string& param);

private:
  std::string source;

  // 'OR' clause chaining
  ServerListFilter* orFilter;

  // pattern filters
  PatternFilter addrPat;
  PatternFilter descPat;
  PatternFilter addrDescPat;

  // boolean filters
  BoolFilter ffa;
  BoolFilter offa;
  BoolFilter ctf;
  BoolFilter rabbit;

  BoolFilter jump;
  BoolFilter rico;
  BoolFilter handi;
  BoolFilter replay;
  BoolFilter inertia;
  BoolFilter antidote;
  BoolFilter favorite;

  // range filters
  RangeFilter shots;
  RangeFilter players;
  RangeFilter freeSlots;
  RangeFilter validTeams;

  RangeFilter maxTime;
  RangeFilter maxPlayers;
  RangeFilter maxTeamScore;
  RangeFilter maxPlayerScore;
  RangeFilter shakeWins;
  RangeFilter shakeTime;

  RangeFilter rogueCount;
  RangeFilter redCount;
  RangeFilter greenCount;
  RangeFilter blueCount;
  RangeFilter purpleCount;
  RangeFilter observerCount;

  RangeFilter rogueMax;
  RangeFilter redMax;
  RangeFilter greenMax;
  RangeFilter blueMax;
  RangeFilter purpleMax;
  RangeFilter observerMax;

  RangeFilter rogueFree;
  RangeFilter redFree;
  RangeFilter greenFree;
  RangeFilter blueFree;
  RangeFilter purpleFree;
  RangeFilter observerFree;

private:
  void setupBoolMap();
  void setupRangeMap();
  void setupPatternMap();
  static std::map<std::string, size_t> boolMap;
  static std::map<std::string, size_t> rangeMap;
  static std::map<std::string, size_t> patternMap;
};


#endif /* __SERVERLISTFILTER_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
