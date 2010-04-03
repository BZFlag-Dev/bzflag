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
#include "ServerListFilter.h"

// system headers
#include <vector>
#include <string>
#include <string.h>
#include <ctype.h>

/* common implementation headers */
#include "ServerItem.h"
#include "TextUtils.h"
#include "bzglob.h"
#include "bzregex.h"
#include "global.h"


std::map<std::string, size_t> ServerListFilter::boolMap;
std::map<std::string, size_t> ServerListFilter::rangeMap;
std::map<std::string, size_t> ServerListFilter::patternMap;


//============================================================================//

ServerListFilter::ServerListFilter()
: subFilter(NULL)
{
  fflush(stdout);
  reset();
}


ServerListFilter::ServerListFilter(const std::string& filter)
: subFilter(NULL)
{
  fflush(stdout);
  parse(filter);
}


//============================================================================//

static int countPlayers(const ServerItem& item)
{
  const PingPacket& ping = item.ping;
  int players = 0;
  players += ping.rogueCount;
  players += ping.redCount;
  players += ping.greenCount;
  players += ping.blueCount;
  players += ping.purpleCount;
  return players;
}


static int countFreeSlots(const ServerItem& item)
{
  const PingPacket& ping = item.ping;
  int freeSlots = 0;
  freeSlots += (ping.rogueMax  - ping.rogueCount);
  freeSlots += (ping.redMax    - ping.redCount);
  freeSlots += (ping.greenMax  - ping.greenCount);
  freeSlots += (ping.blueMax   - ping.blueCount);
  freeSlots += (ping.purpleMax - ping.purpleCount);

  // also check against the 'maxPlayers' limit
  const int totalPlayers = countPlayers(item) + ping.observerCount;
  const int totalFreeSlots = ping.maxPlayers - totalPlayers;
  if (freeSlots > totalFreeSlots) {
    freeSlots = totalFreeSlots;
  }

  return freeSlots;
}


static int countValidTeams(const ServerItem& item)
{
  const PingPacket& ping = item.ping;
  int count = 0;
  if (ping.rogueMax  > 0) { count++; }
  if (ping.redMax    > 0) { count++; }
  if (ping.greenMax  > 0) { count++; }
  if (ping.blueMax   > 0) { count++; }
  if (ping.purpleMax > 0) { count++; }
  return count;
}


static bool isReplay(const ServerItem& item)
{
  return ((item.ping.observerMax == 16) &&
          (item.ping.maxPlayers == 200));
}


//============================================================================//

bool ServerListFilter::check(const ServerItem& item) const
{
  if (subFilter) {
    if (subFilter->check(item)) {
      return true;
    }
  }

  // pattern filters
  std::string addr, desc;
  item.splitAddrTitle(addr, desc);
  if (!addrPat.check(addr)) { return false; }
  if (!descPat.check(desc)) { return false; }
  if (!addrDescPat.check(addr) &&
      !addrDescPat.check(desc)) {
    return false;
  }

  const PingPacket& p = item.ping;

  // boolean filters
  const uint16_t style = p.gameStyle;
  if (!jump    .check(style & JumpingGameStyle))     { return false; }
  if (!rico    .check(style & RicochetGameStyle))    { return false; }
  if (!flags   .check(style & SuperFlagGameStyle))   { return false; }
  if (!teams   .check(style & TeamFlagGameStyle))    { return false; }
  if (!handi   .check(style & HandicapGameStyle))    { return false; }
  if (!rabbit  .check(style & RabbitChaseGameStyle)) { return false; }
  if (!inertia .check(style & InertiaGameStyle))     { return false; }
  if (!antidote.check(style & AntidoteGameStyle))    { return false; }
  if (!replay  .check(isReplay(item)))               { return false; }
  if (!favorite.check(item.favorite))                { return false; }
  if (!cached  .check(item.cached))                  { return false; }

  // range filters
  if (!shots         .check(p.maxShots))            { return false; }
  if (!players       .check(countPlayers(item)))    { return false; }
  if (!freeSlots     .check(countFreeSlots(item)))  { return false; }
  if (!validTeams    .check(countValidTeams(item))) { return false; }

  if (!maxTime       .check(p.maxTime))             { return false; }
  if (!maxPlayers    .check(p.maxPlayers))          { return false; }
  if (!maxTeamScore  .check(p.maxTeamScore))        { return false; }
  if (!maxPlayerScore.check(p.maxPlayerScore))      { return false; }
  if (!shakeWins     .check(p.shakeWins))           { return false; }
  if (!shakeTime     .check(p.shakeTimeout))        { return false; }

  if (!rogueCount   .check(p.rogueCount))    { return false; }
  if (!redCount     .check(p.redCount))      { return false; }
  if (!greenCount   .check(p.greenCount))    { return false; }
  if (!blueCount    .check(p.blueCount))     { return false; }
  if (!purpleCount  .check(p.purpleCount))   { return false; }
  if (!observerCount.check(p.observerCount)) { return false; }

  if (!rogueMax   .check(p.rogueMax))    { return false; }
  if (!redMax     .check(p.redMax))      { return false; }
  if (!greenMax   .check(p.greenMax))    { return false; }
  if (!blueMax    .check(p.blueMax))     { return false; }
  if (!purpleMax  .check(p.purpleMax))   { return false; }
  if (!observerMax.check(p.observerMax)) { return false; }

  if (!rogueFree   .check(p.rogueMax    - p.rogueCount))    { return false; }
  if (!redFree     .check(p.redMax      - p.redCount))      { return false; }
  if (!greenFree   .check(p.greenMax    - p.greenCount))    { return false; }
  if (!blueFree    .check(p.blueMax     - p.blueCount))     { return false; }
  if (!purpleFree  .check(p.purpleMax   - p.purpleCount))   { return false; }
  if (!observerFree.check(p.observerMax - p.observerCount)) { return false; }

  return true;
}


//============================================================================//

void ServerListFilter::reset()
{
  source = "";

  // subFilter
  if (subFilter) {
    delete subFilter;
    subFilter = NULL;
  }

  // patterns
  addrPat.reset();
  descPat.reset();
  addrDescPat.reset();

  // boolean options
  jump.reset();
  rico.reset();
  flags.reset();
  teams.reset();
  handi.reset();
  rabbit.reset();
  replay.reset();
  inertia.reset();
  antidote.reset();
  favorite.reset();
  cached.reset();

  // range options
  shots.reset();
  players.reset();
  freeSlots.reset();
  validTeams.reset();

  maxTime.reset();
  maxPlayers.reset();
  maxTeamScore.reset();
  maxPlayerScore.reset();
  shakeWins.reset();
  shakeTime.reset();

  rogueCount.reset();
  redCount.reset();
  greenCount.reset();
  blueCount.reset();
  purpleCount.reset();
  observerCount.reset();

  rogueMax.reset();
  redMax.reset();
  greenMax.reset();
  blueMax.reset();
  purpleMax.reset();
  observerMax.reset();

  rogueFree.reset();
  redFree.reset();
  greenFree.reset();
  blueFree.reset();
  purpleFree.reset();
  observerFree.reset();
}


//============================================================================//

bool ServerListFilter::parse(const std::string& filter)
{
  setupBoolMap();
  setupRangeMap();
  setupPatternMap();

  reset();

  source = filter;

  std::string glob;
  std::string opts;
  const std::string::size_type optPos = source.find_first_of('/');
  if (optPos == std::string::npos) {
    glob = source;
  }
  else {
    glob = source.substr(0, optPos);
    opts = source.substr(optPos + 1);
    const std::string::size_type subPos = opts.find_first_of('|');
    if (subPos != std::string::npos) {
      subFilter = new ServerListFilter;
      subFilter->parse("/" + opts.substr(subPos + 1));
      opts.resize(subPos);
    }
  }

  if (!glob.empty()) {
    addrDescPat.setupGlob(glob, true);
  }
  
  const std::vector<std::string> optArgs = TextUtils::tokenize(opts, ",");
  for (size_t i = 0; i < optArgs.size(); i++) {
    parseOption(optArgs[i]);
  }

  return true;
}


//============================================================================//


bool ServerListFilter::parseBoolOpt(const std::string& s)
{
  if (s.empty()) {
    return false;
  }
  char type = s[0];
  if ((type != '+') && (type != '-')) {
    return false;
  }
  std::map<std::string, size_t>::const_iterator it = boolMap.find(s.substr(1));
  if (it == boolMap.end()) {
    return false;
  }

  BoolOpt* bo = (BoolOpt*)((char*)this + it->second);
  bo->active = true;
  bo->value  = (type == '+');

  return true;
}


bool ServerListFilter::parseRangeOpt(const std::string& s)
{
  const char* c = s.c_str();
  const char* s0 = c;
  while ((*c != 0) && isalpha(*c)) {
    c++;
  }
  char type = *c;
  if ((type != '<') && (type != '>') && (type != '=')) {
    return false;
  }
  const std::string label(s0, c - s0);
  c++;
  char* e;
  float value = strtof(c, &e);
  if (e == c) {
    return false;
  }

  std::map<std::string, size_t>::const_iterator it = rangeMap.find(label);
  if (it == rangeMap.end()) {
    return false;
  }

  RangeOpt* ro = (RangeOpt*)((char*)this + it->second);
  switch (type) {
    case '<': { ro->maxActive = true; ro->maxValue = value; break; }
    case '>': { ro->minActive = true; ro->minValue = value; break; }
    case '=': {
      ro->minActive = true;
      ro->maxActive = true;
      ro->minValue = value - 1.0f;
      ro->maxValue = value + 1.0f;
      break;
    }
  }

  return true;
}


bool ServerListFilter::parsePatternOpt(const std::string& s)
{
  const char* c = s.c_str();
  const char* s0 = c;
  while ((*c != 0) && isalpha(*c)) {
    c++;
  }

  const char type = *c;
  bool useGlob;
  if (type == '=') {
    useGlob = true;
  } else if (type == '~') {
    useGlob = false;
  } else {
    return false;
  }
  
  const std::string label(s0, c - s0);
  if (label.empty()) {
    return false;
  }
  bool noCase = islower(label[0]);

  c++;
  std::string pattern = c;

  const std::string lower = TextUtils::tolower(label);
  std::map<std::string, size_t>::const_iterator it = patternMap.find(lower);
  if (it == patternMap.end()) {
    return false;
  }

  PatternOpt* po = (PatternOpt*)((char*)this + it->second);
  if (useGlob) {
    po->setupGlob(pattern, noCase);
  } else {
    po->setupRegex(pattern, noCase);
  }

  return true;
}


//============================================================================//

bool ServerListFilter::parseOption(const std::string& opt)
{
  if (opt.empty()) {
    return false;
  }

  if (parseBoolOpt(opt))    { return true; }
  if (parseRangeOpt(opt))   { return true; }
  if (parsePatternOpt(opt)) { return true; }

  return false;
}


//============================================================================//
//
//  Filter name maps
//

#undef  OFFSETOF
#define OFFSETOF(x) (size_t)((char*)&this->x - (char*)this)


void ServerListFilter::setupBoolMap()
{
  if (!boolMap.empty()) { return; }

  std::map<std::string, size_t>& m = boolMap;

  m["j"] = m["jump"]     = OFFSETOF(jump);
  m["r"] = m["rico"]     = OFFSETOF(rico);
  m["f"] = m["flags"]    = OFFSETOF(flags);
  m["t"] = m["teams"]    = OFFSETOF(teams);
  m["h"] = m["handicap"] = OFFSETOF(handi);
  m["R"] = m["rabbit"]   = OFFSETOF(rabbit);
  m["P"] = m["replay"]   = OFFSETOF(replay);
  m["i"] = m["inertia"]  = OFFSETOF(inertia);
  m["a"] = m["antidote"] = OFFSETOF(antidote);
  m["F"] = m["favorite"] = OFFSETOF(favorite);
  m["C"] = m["cached"]   = OFFSETOF(cached);
}


void ServerListFilter::setupRangeMap()
{
  if (!rangeMap.empty()) { return; }

  std::map<std::string, size_t>& m = rangeMap;

  m["s"]   = m["shots"]           = OFFSETOF(shots);
  m["p"]   = m["players"]         = OFFSETOF(players);
  m["f"]   = m["freeSlots"]       = OFFSETOF(freeSlots);
  m["vt"]  = m["validTeams"]      = OFFSETOF(validTeams);

  m["mt"]  = m["maxTime"]         = OFFSETOF(maxTime);
  m["mp"]  = m["maxPlayers"]      = OFFSETOF(maxPlayers);
  m["mts"] = m["maxTeamScore"]    = OFFSETOF(maxTeamScore);
  m["mps"] = m["maxPlayerScore"]  = OFFSETOF(maxPlayerScore);

  m["sw"]  = m["shakeWins"]       = OFFSETOF(shakeWins);
  m["st"]  = m["shakeTime"]       = OFFSETOF(shakeTime);

  m["Rm"]  = m["rogueMax"]        = OFFSETOF(rogueMax);
  m["rm"]  = m["redMax"]          = OFFSETOF(redMax);
  m["gm"]  = m["greenMax"]        = OFFSETOF(greenMax);
  m["bm"]  = m["blueMax"]         = OFFSETOF(blueMax);
  m["pm"]  = m["purpleMax"]       = OFFSETOF(purpleMax);
  m["om"]  = m["observerMax"]     = OFFSETOF(observerMax);

  m["Rp"]  = m["roguePlayers"]    = OFFSETOF(rogueCount);
  m["rp"]  = m["redPlayers"]      = OFFSETOF(redCount);
  m["gp"]  = m["greenPlayers"]    = OFFSETOF(greenCount);
  m["bp"]  = m["bluePlayers"]     = OFFSETOF(blueCount);
  m["pp"]  = m["purplePlayers"]   = OFFSETOF(purpleCount);
  m["op"]  = m["observerPlayers"] = OFFSETOF(observerCount);

  m["Rf"]  = m["rogueFree"]       = OFFSETOF(rogueFree);
  m["rf"]  = m["redFree"]         = OFFSETOF(redFree);
  m["gf"]  = m["greenFree"]       = OFFSETOF(greenFree);
  m["bf"]  = m["blueFree"]        = OFFSETOF(blueFree);
  m["pf"]  = m["purpleFree"]      = OFFSETOF(purpleFree);
  m["of"]  = m["observerFree"]    = OFFSETOF(observerFree);
}


void ServerListFilter::setupPatternMap()
{
  if (!patternMap.empty()) { return; }

  std::map<std::string, size_t>& m = patternMap;

  m["a"] = m["addr"] = m["address"]     = OFFSETOF(addrPat);
  m["d"] = m["desc"] = m["description"] = OFFSETOF(descPat);
  m["ad"] = m["addrdesc"]               = OFFSETOF(addrDescPat);
}


//============================================================================//

void ServerListFilter::print(const std::string& origIndent) const
{
  printf("%sServerListFilter <'%s'>\n", origIndent.c_str(), source.c_str());

  const std::string indent = origIndent + "  ";

  // pattern filters
  addrPat.print("addrPat", indent);
  descPat.print("descPat", indent);
  addrDescPat.print("addrDescPat", indent);

  // boolean filters
  jump.print("jump", indent);
  rico.print("rico", indent);
  flags.print("flags", indent);
  teams.print("teams", indent);
  handi.print("handi", indent);
  rabbit.print("rabbit", indent);
  replay.print("replay", indent);
  inertia.print("inertia", indent);
  antidote.print("antidote", indent);
  favorite.print("favorite", indent);
  cached.print("cached", indent);

  // range filters
  shots.print("shots", indent);
  players.print("players", indent);
  freeSlots.print("freeSlots", indent);
  validTeams.print("validTeams", indent);

  maxTime.print("maxTime", indent);
  maxPlayers.print("maxPlayers", indent);
  maxTeamScore.print("maxTeamScore", indent);
  maxPlayerScore.print("maxPlayerScore", indent);
  shakeWins.print("shakeWins", indent);
  shakeTime.print("shakeTime", indent);

  rogueCount.print("rogueCount", indent);
  redCount.print("redCount", indent);
  greenCount.print("greenCount", indent);
  blueCount.print("blueCount", indent);
  purpleCount.print("purpleCount", indent);
  observerCount.print("observerCount", indent);

  rogueMax.print("rogueMax", indent);
  redMax.print("redMax", indent);
  greenMax.print("greenMax", indent);
  blueMax.print("blueMax", indent);
  purpleMax.print("purpleMax", indent);
  observerMax.print("observerMax", indent);

  rogueFree.print("rogueFree", indent);
  redFree.print("redFree", indent);
  greenFree.print("greenFree", indent);
  blueFree.print("blueFree", indent);
  purpleFree.print("purpleFree", indent);
  observerFree.print("observerFree", indent);

  if (subFilter) {
    subFilter->print(indent);
  }
}


//============================================================================//
//
//  ServerListFilter::PatternOpt
//

void ServerListFilter::PatternOpt::reset()
{
  type = NoPattern;
  noCase = true;
  pattern = "";
  if (re) {
    regfree(re);
    delete re;
    re = NULL;
  }
}


bool ServerListFilter::PatternOpt::setupGlob(const std::string& _pattern,
                                             bool _noCase)
{
  reset();

  noCase = _noCase;
  pattern = _pattern;

  if (pattern.empty()) {
    reset();
    return false;
  }

  if (noCase) {
    pattern = TextUtils::tolower(pattern);
  }

  if ((pattern.find("*") == std::string::npos) &&
      (pattern.find("?") == std::string::npos)) {
    pattern = "*" + pattern + "*";
  }

  type = GlobPattern;

  return true;
}


bool ServerListFilter::PatternOpt::setupRegex(const std::string& _pattern,
                                              bool _noCase)
{
  reset();

  noCase = _noCase;
  pattern = _pattern;

  int opts = REG_EXTENDED | REG_NOSUB;
  if (noCase) {
    opts |= REG_ICASE;
  }

  re = new regex_t;

  if (regcomp(re, pattern.c_str(), opts) != 0) {
    reset();
    return false;
  }

  type = RegexPattern;

  return true;
}


bool ServerListFilter::PatternOpt::check(const std::string& s) const
{
  switch (type) {
    case NoPattern: {
      break;
    }
    case RegexPattern: {
      return (regexec(re, s.c_str(), 0, NULL, 0) == 0);
    }
    case GlobPattern: {
      if (noCase) {
        return glob_match(pattern, TextUtils::tolower(s));
      } else {
        return glob_match(pattern, s);
      }
    }
  }
  return true;
}


//============================================================================//
//
//  ServerListFilter::*Opt::print()
//

void ServerListFilter::BoolOpt::print(const std::string& name,
                                      const std::string& indent) const
{
  if (active) {
    printf("%s%s = %s\n", indent.c_str(),
           name.c_str(), value ? "true" : "false");
  }
}


void ServerListFilter::RangeOpt::print(const std::string& name,
                                       const std::string& indent) const
{
  if (!minActive && !maxActive) {
    return;
  }
  else if (minActive && maxActive) {
    printf("%s%.3g < %s < %.3g\n", indent.c_str(),
           minValue, name.c_str(), maxValue);
  }
  else if (minActive) {
    printf("%s%s > %.3g\n", indent.c_str(),
           name.c_str(), minValue);
  }
  else if (maxActive) {
    printf("%s%s < %.3g\n", indent.c_str(),
           name.c_str(), maxValue);
  }
}


void ServerListFilter::PatternOpt::print(const std::string& name,
                                         const std::string& indent) const
{
  if (type == NoPattern) {
    return;
  }
  const char* typeStr = (type == GlobPattern) ? "glob" : "regex";
  const char* caseStr = noCase ? "nocase" : "case";
  printf("%s%s = '%s' <%s|%s>\n", indent.c_str(),
         name.c_str(), pattern.c_str(), typeStr, caseStr);
}


//============================================================================//
