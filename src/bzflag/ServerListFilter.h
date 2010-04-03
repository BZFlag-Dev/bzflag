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

#ifndef __SERVERLISTFILTER_H__
#define __SERVERLISTFILTER_H__

#include "common.h"
#include <string>
#include <map>


class ServerItem;


class ServerListFilter {
  private:
    struct BoolOpt {
      static bool parse(const std::string& s);
      BoolOpt() {
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

    struct RangeOpt {
      static bool parse(const std::string& s);
      RangeOpt() {
        reset();
      }
      void reset() {
        minActive = maxActive = false;
      }
      bool check(float v) const {
        return !(minActive && (v <= minValue)) &&
               !(maxActive && (v >= maxValue));
      }
      bool check(int v) const { return check(float(v)); }
      void print(const std::string& name, const std::string& indent) const;
      float minValue,  maxValue;
      bool  minActive, maxActive;
    };

    struct PatternOpt {
      enum PatternType {
        NoPattern    = 0,
        GlobPattern  = 1,
        RegexPattern = 2
      };
      static bool parse(const std::string& s);
      PatternOpt() : re(NULL) {
        reset();
      }
      ~PatternOpt() {
        reset();
      }
      bool setupGlob(const std::string& pattern, bool noCase);
      bool setupRegex(const std::string& pattern, bool noCase);
      void reset();
      bool check(const std::string& s) const;
      void print(const std::string& name, const std::string& indent) const;
      PatternType type;
      bool        noCase;
      std::string pattern;
      struct regex_t* re;
      private: // no copying
        PatternOpt(const PatternOpt&);
        PatternOpt& operator=(const PatternOpt&);
    };

  public:
    ServerListFilter();
    ServerListFilter(const std::string& filter);

    bool parse(const std::string& filter);

    const std::string& getSource() const { return source; }

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

  private:
    void reset();
    bool parseOption(const std::string& opt);
    bool parseBoolOpt(const std::string& opt);
    bool parseRangeOpt(const std::string& opt);
    bool parsePatternOpt(const std::string& opt);

  private:
    std::string source;

    // sub-filter
    ServerListFilter* subFilter;

    // pattern filters
    PatternOpt addrPat;
    PatternOpt descPat;
    PatternOpt addrDescPat;
    
    // boolean filters
    BoolOpt jump;
    BoolOpt rico;
    BoolOpt flags;
    BoolOpt teams;
    BoolOpt handi;
    BoolOpt rabbit;
    BoolOpt replay;
    BoolOpt inertia;
    BoolOpt antidote;
    BoolOpt favorite;
    BoolOpt cached;

    // range filters
    RangeOpt shots;
    RangeOpt players;
    RangeOpt freeSlots;
    RangeOpt validTeams;

    RangeOpt maxTime;
    RangeOpt maxPlayers;
    RangeOpt maxTeamScore;
    RangeOpt maxPlayerScore;
    RangeOpt shakeWins;
    RangeOpt shakeTime;

    RangeOpt rogueCount;
    RangeOpt redCount;
    RangeOpt greenCount;
    RangeOpt blueCount;
    RangeOpt purpleCount;
    RangeOpt observerCount;

    RangeOpt rogueMax;
    RangeOpt redMax;
    RangeOpt greenMax;
    RangeOpt blueMax;
    RangeOpt purpleMax;
    RangeOpt observerMax;

    RangeOpt rogueFree;
    RangeOpt redFree;
    RangeOpt greenFree;
    RangeOpt blueFree;
    RangeOpt purpleFree;
    RangeOpt observerFree;

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
