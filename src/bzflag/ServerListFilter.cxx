/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "ServerListFilter.h"

// system headers
#include <vector>
#include <string>
#include <string.h>
#include <ctype.h>

// common implementation headers
#include "AnsiCodes.h"
#include "ServerItem.h"
#include "TextUtils.h"
#include "bzglob.h"
#include "global.h"

// local headers
#include "playing.h"


std::map<std::string, size_t> ServerListFilter::boolMap;
std::map<std::string, size_t> ServerListFilter::rangeMap;
std::map<std::string, size_t> ServerListFilter::patternMap;


//============================================================================//

static void errorMessage(const std::string& msg)
{
  addMessage(NULL, ANSI_STR_FG_RED "ServerListFilter: " + msg);
}


//============================================================================//

ServerListFilter::ServerListFilter()
: orFilter(NULL)
{
  fflush(stdout);
  reset();
}


ServerListFilter::ServerListFilter(const std::string& filter)
: orFilter(NULL)
{
  fflush(stdout);
  parse(filter);
}


//============================================================================//

void ServerListFilter::reset()
{
  source = "";

  // OR clauses
  if (orFilter) {
    delete orFilter;
    orFilter = NULL;
  }

  // pattern filters
  addrPat     .reset();
  descPat     .reset();
  addrDescPat .reset();

  // boolean filters
  jump     .reset();
  rico     .reset();
  flags    .reset();
  teams    .reset();
  handi    .reset();
  rabbit   .reset();
  replay   .reset();
  inertia  .reset();
  antidote .reset();
  favorite .reset();
  cached   .reset();

  // range filters
  shots      .reset();
  players    .reset();
  freeSlots  .reset();
  validTeams .reset();

  maxTime	.reset();
  maxPlayers     .reset();
  maxTeamScore   .reset();
  maxPlayerScore .reset();
  shakeWins      .reset();
  shakeTime      .reset();

  rogueCount    .reset();
  redCount      .reset();
  greenCount    .reset();
  blueCount     .reset();
  purpleCount   .reset();
  observerCount .reset();

  rogueMax    .reset();
  redMax      .reset();
  greenMax    .reset();
  blueMax     .reset();
  purpleMax   .reset();
  observerMax .reset();

  rogueFree    .reset();
  redFree      .reset();
  greenFree    .reset();
  blueFree     .reset();
  purpleFree   .reset();
  observerFree .reset();
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
  // OR clauses
  if (orFilter && orFilter->check(item)) {
    return true;
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
  const uint16_t type = p.gameType;
  if (!flags    .check(type == TeamFFA))   { return false; }
  if (!teams    .check(type == ClassicCTF))  { return false; }
  if (!rabbit   .check(type == RabbitChase)) { return false; }

  const uint16_t options = p.gameOptions;
  if (!jump     .check((options & JumpingGameStyle)!=0))     { return false; }
  if (!rico     .check((options & RicochetGameStyle)!=0))    { return false; }
  if (!handi    .check((options & HandicapGameStyle)!=0))    { return false; }
  if (!inertia  .check((options & InertiaGameStyle)!=0))     { return false; }
  if (!antidote .check((options & AntidoteGameStyle)!=0))    { return false; }

  if (!replay   .check(isReplay(item))) { return false; }
  if (!favorite .check(item.favorite))  { return false; }
  if (!cached   .check(item.cached))    { return false; }

  // range filters
  if (!shots	  .check(p.maxShots))	    { return false; }
  if (!players	.check(countPlayers(item)))    { return false; }
  if (!freeSlots      .check(countFreeSlots(item)))  { return false; }
  if (!validTeams     .check(countValidTeams(item))) { return false; }

  if (!maxTime	.check(p.maxTime))	{ return false; }
  if (!maxPlayers     .check(p.maxPlayers))     { return false; }
  if (!maxTeamScore   .check(p.maxTeamScore))   { return false; }
  if (!maxPlayerScore .check(p.maxPlayerScore)) { return false; }
  if (!shakeWins      .check(p.shakeWins))      { return false; }
  if (!shakeTime      .check(p.shakeTimeout))   { return false; }

  if (!rogueCount    .check(p.rogueCount))    { return false; }
  if (!redCount      .check(p.redCount))      { return false; }
  if (!greenCount    .check(p.greenCount))    { return false; }
  if (!blueCount     .check(p.blueCount))     { return false; }
  if (!purpleCount   .check(p.purpleCount))   { return false; }
  if (!observerCount .check(p.observerCount)) { return false; }

  if (!rogueMax    .check(p.rogueMax))    { return false; }
  if (!redMax      .check(p.redMax))      { return false; }
  if (!greenMax    .check(p.greenMax))    { return false; }
  if (!blueMax     .check(p.blueMax))     { return false; }
  if (!purpleMax   .check(p.purpleMax))   { return false; }
  if (!observerMax .check(p.observerMax)) { return false; }

  const int totalFree = p.maxPlayers - (countPlayers(item) + p.observerCount);
#define FREE_SLOTS(T) std::min(totalFree, p. T ## Max - p.T ## Count)
  if (!rogueFree    .check(FREE_SLOTS(rogue)))    { return false; }
  if (!redFree      .check(FREE_SLOTS(red)))      { return false; }
  if (!greenFree    .check(FREE_SLOTS(green)))    { return false; }
  if (!blueFree     .check(FREE_SLOTS(blue)))     { return false; }
  if (!purpleFree   .check(FREE_SLOTS(purple)))   { return false; }
  if (!observerFree .check(FREE_SLOTS(observer))) { return false; }
#undef FREE_SLOTS

  return true;
}


//============================================================================//

static std::string stripLeadingWhite(const std::string& s)
{
  if (s.empty()) {
    return s;
  }
  const std::string::size_type pos = s.find_first_not_of(" \t");
  if (pos == std::string::npos) {
    return s;
  }
  return s.substr(pos);
}


bool ServerListFilter::parse(const std::string& filter)
{
  setupBoolMap();
  setupRangeMap();
  setupPatternMap();

  reset();

  source = filter;

  std::string adGlobStr;
  std::string filterStr;
  const std::string::size_type optPos = source.find_first_of('/');
  if (optPos == std::string::npos) {
    adGlobStr = source;
  }
  else {
    adGlobStr = source.substr(0, optPos);
    filterStr = source.substr(optPos + 1);
    const std::string::size_type orPos = filterStr.find_first_of('/');
    if (orPos != std::string::npos) {
      orFilter = new ServerListFilter;
      orFilter->parse("/" + filterStr.substr(orPos + 1));
      filterStr.resize(orPos);
    }
  }

  if (!adGlobStr.empty()) {
    addrDescPat.setupGlob(adGlobStr, true);
  }

  const std::vector<std::string> filters = TextUtils::tokenize(filterStr, ",");
  for (size_t i = 0; i < filters.size(); i++) {
    parseFilter(filters[i]);
  }

  return true;
}


//============================================================================//

bool ServerListFilter::parseFilter(const std::string& f)
{
  char op;
  std::string label, param;
  switch (parseFilterType(f, op, label, param)) {
    case '#': { return true; }
    case 'b': { return parseBoolFilter(label, op);	   }
    case 'r': { return parseRangeFilter(label, op, param);   }
    case 'p': { return parsePatternFilter(label, op, param); }
    default: {
      errorMessage("invalid filter, '" + f + "'");
      return false;
    }
  }
}


char ServerListFilter::parseFilterType(const std::string& _f, char& op,
				       std::string& label, std::string& param)
{
  const std::string f = stripLeadingWhite(_f);

  if (f.empty() || (f[0] == '#')) {
    return '#'; // comment
  }

  if ((f[0] == '+') || (f[0] == '-')) {
    label = f.substr(1);
    param = "";
    op = f[0];
    return 'b'; // boolean
  }

  const char* s = f.c_str();
  const char* c = s;
  while ((*c != 0) && (isalnum(*c) || (*c == '_'))) { c++; }

  switch (*c) {
    case '=':
    case '>':
    case '<': {
      label = std::string(s, c - s);
      if ((*c == '=') || (*(c + 1) != '=')) {
	param = std::string(c + 1);
	op = *c;
      } else {
	param = std::string(c + 2);
	op = *c + 0x3f; // map [<>] to [{}]
      }
      return 'r'; // range
    }
    case ')':
    case ']': {
      label = std::string(s, c - s);
      param = std::string(c + 1);
      op = *c;
      return 'p'; // pattern
    }
  }

  return 0; // invalid
}

//============================================================================//

bool ServerListFilter::parseBoolFilter(const std::string& label, char op)
{
  std::map<std::string, size_t>::const_iterator it = boolMap.find(label);
  if (it == boolMap.end()) {
    errorMessage("unknown boolean label, '" + label + "'");
    return false;
  }

  BoolFilter* bf = (BoolFilter*)((char*)this + it->second);
  bf->active = true;
  bf->value  = (op == '+');

  return true;
}


bool ServerListFilter::parseRangeFilter(const std::string& label, char op,
					const std::string& param)
{
  std::map<std::string, size_t>::const_iterator it = rangeMap.find(label);
  if (it == rangeMap.end()) {
    errorMessage("unknown range label, '" + label + "'");
    return false;
  }

  const char* s = param.c_str();
  char* e;
  float value = (float)strtod(s, &e);
  if ((e == s) || (param.size() != size_t(e - s))) {
    errorMessage("bad range value, '" + param + "'");
    return false;
  }

  RangeFilter* rf = (RangeFilter*)((char*)this + it->second);
  switch (op) {
    case '<': { rf->maxActive = true; rf->maxValue = value;	break; }
    case '{': { rf->maxActive = true; rf->maxValue = value + 1.0f; break; }
    case '>': { rf->minActive = true; rf->minValue = value;	break; }
    case '}': { rf->minActive = true; rf->minValue = value - 1.0f; break; }
    case '=': {
      rf->minActive = true;
      rf->maxActive = true;
      rf->minValue = value - 1.0f;
      rf->maxValue = value + 1.0f;
      break;
    }
    default: { return false; }
  }

  return true;
}


bool ServerListFilter::parsePatternFilter(const std::string& label, char op,
					  const std::string& param)
{
  const std::string lower = TextUtils::tolower(label);
  std::map<std::string, size_t>::const_iterator it = patternMap.find(lower);
  if (it == patternMap.end()) {
    errorMessage("unknown pattern label, '" + label + "'");
    return false;
  }

  bool useGlob;
  switch (op) {
    case ')': { useGlob = true;  break; }
    case ']': { useGlob = false; break; }
    default:  { return false; }
  }

  bool noCase = islower(label[0]) != 0;

  PatternFilter* pf = (PatternFilter*)((char*)this + it->second);

  return useGlob ? pf->setupGlob(param, noCase)
		 : pf->setupRegex(param, noCase);
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

  m["s"]   = m["shots"]	   = OFFSETOF(shots);
  m["p"]   = m["players"]	 = OFFSETOF(players);
  m["f"]   = m["freeSlots"]       = OFFSETOF(freeSlots);
  m["vt"]  = m["validTeams"]      = OFFSETOF(validTeams);

  m["mt"]  = m["maxTime"]	 = OFFSETOF(maxTime);
  m["mp"]  = m["maxPlayers"]      = OFFSETOF(maxPlayers);
  m["mts"] = m["maxTeamScore"]    = OFFSETOF(maxTeamScore);
  m["mps"] = m["maxPlayerScore"]  = OFFSETOF(maxPlayerScore);

  m["sw"]  = m["shakeWins"]       = OFFSETOF(shakeWins);
  m["st"]  = m["shakeTime"]       = OFFSETOF(shakeTime);

  m["Rm"]  = m["rogueMax"]	= OFFSETOF(rogueMax);
  m["rm"]  = m["redMax"]	  = OFFSETOF(redMax);
  m["gm"]  = m["greenMax"]	= OFFSETOF(greenMax);
  m["bm"]  = m["blueMax"]	 = OFFSETOF(blueMax);
  m["pm"]  = m["purpleMax"]       = OFFSETOF(purpleMax);
  m["om"]  = m["observerMax"]     = OFFSETOF(observerMax);

  m["Rp"]  = m["roguePlayers"]    = OFFSETOF(rogueCount);
  m["rp"]  = m["redPlayers"]      = OFFSETOF(redCount);
  m["gp"]  = m["greenPlayers"]    = OFFSETOF(greenCount);
  m["bp"]  = m["bluePlayers"]     = OFFSETOF(blueCount);
  m["pp"]  = m["purplePlayers"]   = OFFSETOF(purpleCount);
  m["op"]  = m["observerPlayers"] = OFFSETOF(observerCount);

  m["Rf"]  = m["rogueFree"]       = OFFSETOF(rogueFree);
  m["rf"]  = m["redFree"]	 = OFFSETOF(redFree);
  m["gf"]  = m["greenFree"]       = OFFSETOF(greenFree);
  m["bf"]  = m["blueFree"]	= OFFSETOF(blueFree);
  m["pf"]  = m["purpleFree"]      = OFFSETOF(purpleFree);
  m["of"]  = m["observerFree"]    = OFFSETOF(observerFree);
}


void ServerListFilter::setupPatternMap()
{
  if (!patternMap.empty()) { return; }

  std::map<std::string, size_t>& m = patternMap;

  m["a"] = m["addr"] = m["address"]     = OFFSETOF(addrPat);
  m["d"] = m["desc"] = m["description"] = OFFSETOF(descPat);
  m["ad"] = m["addrdesc"]	       = OFFSETOF(addrDescPat);
}


bool ServerListFilter::isBoolLabel(const std::string& label)
{
  return boolMap.find(label) != boolMap.end();
}


bool ServerListFilter::isRangeLabel(const std::string& label)
{
  return rangeMap.find(label) != rangeMap.end();
}


bool ServerListFilter::isPatternLabel(const std::string& label)
{
  return patternMap.find(TextUtils::tolower(label)) != patternMap.end();
}


//============================================================================//

void ServerListFilter::print(const std::string& origIndent) const
{
  printf("%sServerListFilter <'%s'>\n", origIndent.c_str(), source.c_str());

  const std::string indent = origIndent + "  ";

  // pattern filters
  addrPat     .print("addrPat",     indent);
  descPat     .print("descPat",     indent);
  addrDescPat .print("addrDescPat", indent);

  // boolean filters
  jump     .print("jump",     indent);
  rico     .print("rico",     indent);
  flags    .print("flags",    indent);
  teams    .print("teams",    indent);
  handi    .print("handi",    indent);
  rabbit   .print("rabbit",   indent);
  replay   .print("replay",   indent);
  inertia  .print("inertia",  indent);
  antidote .print("antidote", indent);
  favorite .print("favorite", indent);
  cached   .print("cached",   indent);

  // range filters
  shots      .print("shots",      indent);
  players    .print("players",    indent);
  freeSlots  .print("freeSlots",  indent);
  validTeams .print("validTeams", indent);

  maxTime	.print("maxTime",	indent);
  maxPlayers     .print("maxPlayers",     indent);
  maxTeamScore   .print("maxTeamScore",   indent);
  maxPlayerScore .print("maxPlayerScore", indent);
  shakeWins      .print("shakeWins",      indent);
  shakeTime      .print("shakeTime",      indent);

  rogueCount    .print("rogueCount",    indent);
  redCount      .print("redCount",      indent);
  greenCount    .print("greenCount",    indent);
  blueCount     .print("blueCount",     indent);
  purpleCount   .print("purpleCount",   indent);
  observerCount .print("observerCount", indent);

  rogueMax     .print("rogueMax",    indent);
  redMax       .print("redMax",      indent);
  greenMax     .print("greenMax",    indent);
  blueMax      .print("blueMax",     indent);
  purpleMax    .print("purpleMax",   indent);
  observerMax  .print("observerMax", indent);

  rogueFree    .print("rogueFree",    indent);
  redFree      .print("redFree",      indent);
  greenFree    .print("greenFree",    indent);
  blueFree     .print("blueFree",     indent);
  purpleFree   .print("purpleFree",   indent);
  observerFree .print("observerFree", indent);

  // OR clauses
  if (orFilter) {
    orFilter->print(indent);
  }
}


//============================================================================//
//
//  ServerListFilter::PatternFilter
//

void ServerListFilter::PatternFilter::reset()
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


bool ServerListFilter::PatternFilter::setupGlob(const std::string& _pattern,
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


bool ServerListFilter::PatternFilter::setupRegex(const std::string& _pattern,
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

  const int errcode = regcomp(re, pattern.c_str(), opts);
  if (errcode != 0) {
    char errbuf[256];
    const size_t len = regerror(errcode, re, errbuf, sizeof(errbuf));
    const std::string errstr(errbuf, len);
    errorMessage("bad regex, " + errstr + " '" + pattern + "'");
    reset();
    return false;
  }

  type = RegexPattern;

  return true;
}


bool ServerListFilter::PatternFilter::check(const std::string& s) const
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
//  ServerListFilter::*Filter::print()
//

void ServerListFilter::BoolFilter::print(const std::string& name,
					 const std::string& indent) const
{
  if (active) {
    printf("%s%s = %s\n", indent.c_str(),
	   name.c_str(), value ? "true" : "false");
  }
}


void ServerListFilter::RangeFilter::print(const std::string& name,
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


void ServerListFilter::PatternFilter::print(const std::string& name,
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
