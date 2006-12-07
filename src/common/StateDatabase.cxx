/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(_MSC_VER)
#pragma warning(4:4503)
#endif

// interface header
#include "StateDatabase.h"

// system headers
#include <assert.h>
#include <ctype.h>
#include <stack>
#include <set>
#include <iostream>
#include <math.h>
#include <string>

// local implementation headers
#include "ErrorHandler.h"
#include "TextUtils.h"

#if defined(DEBUG) || defined(_DEBUG)
// headers needed only for _debugLookups()
#include <map>
#include "TimeKeeper.h"

void	_debugLookups(const std::string &name)
{
  if (!BZDB.getDebug())
    return;

  typedef std::map<std::string,int> EvalCntMap;
  static const float interval = 20.0f;

  /* This bit of nastyness help debug BDZB->eval accesses sorted from worst to best*/
  static EvalCntMap cnts;
  static TimeKeeper last = TimeKeeper::getCurrent();

  EvalCntMap::iterator it = cnts.find(name);
  if (it == cnts.end())
    cnts[name] = 1;
  else
    it->second++;

  TimeKeeper now = TimeKeeper::getCurrent();
  if (now - last > interval) {
    std::multimap<int,std::string> order;
    for (it = cnts.begin(); it != cnts.end(); it++) {
      order.insert(std::pair<int,std::string>(-it->second, it->first));
      it->second = 0;
    }

    for (std::multimap<int,std::string>::iterator it2 = order.begin(); it2 != order.end(); ++it2) {
      if (-it2->first / interval < 1.0f)
	break;
      DEBUG1("%-25s = %.2f acc/sec\n", it2->second.c_str(), -it2->first / interval);
    }
    last = now;
  }
}

  #define debugLookups(name) _debugLookups(name)
#else
  #define debugLookups(name)
#endif

// initialize the singleton
template <>
StateDatabase* Singleton<StateDatabase>::_instance = (StateDatabase*)0;


const std::string StateDatabase::BZDB_AGILITYADVEL	= std::string("_agilityAdVel");
const std::string StateDatabase::BZDB_AGILITYTIMEWINDOW	= std::string("_agilityTimeWindow");
const std::string StateDatabase::BZDB_AGILITYVELDELTA	= std::string("_agilityVelDelta");
const std::string StateDatabase::BZDB_AMBIENTLIGHT	= std::string("_ambientLight");
const std::string StateDatabase::BZDB_ANGLETOLERANCE    = std::string("_angleTolerance");
const std::string StateDatabase::BZDB_ANGULARAD		= std::string("_angularAd");
const std::string StateDatabase::BZDB_AUTOALLOWTIME	= std::string("_autoAllowTime");
const std::string StateDatabase::BZDB_AVENUESIZE	= std::string("_avenueSize");
const std::string StateDatabase::BZDB_BASESIZE		= std::string("_baseSize");
const std::string StateDatabase::BZDB_BOXBASE		= std::string("_boxBase");
const std::string StateDatabase::BZDB_BOXHEIGHT		= std::string("_boxHeight");
const std::string StateDatabase::BZDB_BURROWDEPTH       = std::string("_burrowDepth");
const std::string StateDatabase::BZDB_BURROWSPEEDAD     = std::string("_burrowSpeedAd");
const std::string StateDatabase::BZDB_BURROWANGULARAD   = std::string("_burrowAngularAd");
const std::string StateDatabase::BZDB_COLDETDEPTH       = std::string("_coldetDepth");
const std::string StateDatabase::BZDB_COLDETELEMENTS    = std::string("_coldetElements");
const std::string StateDatabase::BZDB_COLLISIONLIMIT	= std::string("_collisionLimit");
const std::string StateDatabase::BZDB_COUNTDOWNRESDELAY	= std::string("_countdownResumeDelay");
const std::string StateDatabase::BZDB_CULLDEPTH		= std::string("_cullDepth");
const std::string StateDatabase::BZDB_CULLELEMENTS      = std::string("_cullElements");
const std::string StateDatabase::BZDB_CULLOCCLUDERS     = std::string("_cullOccluders");
const std::string StateDatabase::BZDB_DISABLEBOTS       = std::string("_disableBots");
const std::string StateDatabase::BZDB_DMZWIDTH		= std::string("_dmzWidth");
const std::string StateDatabase::BZDB_DRAWCELESTIAL	= std::string("_drawCelestial");
const std::string StateDatabase::BZDB_DRAWCLOUDS	= std::string("_drawClouds");
const std::string StateDatabase::BZDB_DRAWGROUND	= std::string("_drawGround");
const std::string StateDatabase::BZDB_DRAWGROUNDLIGHTS	= std::string("_drawGroundLights");
const std::string StateDatabase::BZDB_DRAWMOUNTAINS	= std::string("_drawMountains");
const std::string StateDatabase::BZDB_DRAWSKY		= std::string("_drawSky");
const std::string StateDatabase::BZDB_ENDSHOTDETECTION  = std::string("_endShotDetection");
const std::string StateDatabase::BZDB_EXPLODETIME       = std::string("_explodeTime");
const std::string StateDatabase::BZDB_FLAGALTITUDE      = std::string("_flagAltitude");
const std::string StateDatabase::BZDB_FLAGEFFECTTIME    = std::string("_flagEffectTime");
const std::string StateDatabase::BZDB_FLAGHEIGHT	= std::string("_flagHeight");
const std::string StateDatabase::BZDB_FLAGPOLEWIDTH     = std::string("_flagPoleWidth");
const std::string StateDatabase::BZDB_FLAGPOLESIZE      = std::string("_flagPoleSize");
const std::string StateDatabase::BZDB_FLAGRADIUS	= std::string("_flagRadius");
const std::string StateDatabase::BZDB_FOGMODE		= std::string("_fogMode");
const std::string StateDatabase::BZDB_FOGDENSITY	= std::string("_fogDensity");
const std::string StateDatabase::BZDB_FOGSTART		= std::string("_fogStart");
const std::string StateDatabase::BZDB_FOGEND		= std::string("_fogEnd");
const std::string StateDatabase::BZDB_FOGCOLOR		= std::string("_fogColor");
const std::string StateDatabase::BZDB_FOGENFORCE	= std::string("_fogEnforce");
const std::string StateDatabase::BZDB_GRABOWNFLAG	= std::string("_grabOwnFlag");
const std::string StateDatabase::BZDB_FREEZETAGRADIUS	= std::string("_freezeTagRadius");
const std::string StateDatabase::BZDB_FRICTION		= std::string("_friction");
const std::string StateDatabase::BZDB_GMACTIVATIONTIME	= std::string("_gmActivationTime");
const std::string StateDatabase::BZDB_GMADLIFE		= std::string("_gmAdLife");
const std::string StateDatabase::BZDB_GMTURNANGLE	= std::string("_gmTurnAngle");
const std::string StateDatabase::BZDB_GRAVITY		= std::string("_gravity");
const std::string StateDatabase::BZDB_HANDICAPSCOREDIFF = std::string("_handicapScoreDiff");
const std::string StateDatabase::BZDB_HANDICAPVELAD	= std::string("_handicapVelAd");
const std::string StateDatabase::BZDB_HANDICAPANGAD	= std::string("_handicapAngAd");
const std::string StateDatabase::BZDB_HANDICAPSHOTAD	= std::string("_handicapShotAd");
const std::string StateDatabase::BZDB_HEIGHTCHECKTOL	= std::string("_heightCheckTolerance");
const std::string StateDatabase::BZDB_IDENTIFYRANGE	= std::string("_identifyRange");
const std::string StateDatabase::BZDB_INERTIALINEAR	= std::string("_inertiaLinear");
const std::string StateDatabase::BZDB_INERTIAANGULAR	= std::string("_inertiaAngular");
const std::string StateDatabase::BZDB_JUMPVELOCITY	= std::string("_jumpVelocity");
const std::string StateDatabase::BZDB_LASERADVEL	= std::string("_laserAdVel");
const std::string StateDatabase::BZDB_LASERADRATE	= std::string("_laserAdRate");
const std::string StateDatabase::BZDB_LASERADLIFE	= std::string("_laserAdLife");
const std::string StateDatabase::BZDB_LATITUDE		= std::string("_latitude");
const std::string StateDatabase::BZDB_LGGRAVITY		= std::string("_lgGravity");
const std::string StateDatabase::BZDB_LOCKONANGLE	= std::string("_lockOnAngle");
const std::string StateDatabase::BZDB_LONGITUDE		= std::string("_longitude");
const std::string StateDatabase::BZDB_LRADRATE		= std::string("_lRAdRate");
const std::string StateDatabase::BZDB_MAXBUMPHEIGHT     = std::string("_maxBumpHeight");
const std::string StateDatabase::BZDB_MAXFLAGGRABS      = std::string("_maxFlagGrabs");
const std::string StateDatabase::BZDB_MAXLOD		= std::string("_maxLOD");
const std::string StateDatabase::BZDB_MIRROR		= std::string("_mirror");
const std::string StateDatabase::BZDB_MOMENTUMLINACC	= std::string("_momentumLinAcc");
const std::string StateDatabase::BZDB_MOMENTUMANGACC	= std::string("_momentumAngAcc");
const std::string StateDatabase::BZDB_MOMENTUMFRICTION	= std::string("_momentumFriction");
const std::string StateDatabase::BZDB_MGUNADVEL		= std::string("_mGunAdVel");
const std::string StateDatabase::BZDB_MGUNADRATE	= std::string("_mGunAdRate");
const std::string StateDatabase::BZDB_MGUNADLIFE	= std::string("_mGunAdLife");
const std::string StateDatabase::BZDB_MUZZLEFRONT	= std::string("_muzzleFront");
const std::string StateDatabase::BZDB_MUZZLEHEIGHT	= std::string("_muzzleHeight");
const std::string StateDatabase::BZDB_NOCLIMB		= std::string("_noClimb");
const std::string StateDatabase::BZDB_NOSHADOWS		= std::string("_noShadows");
const std::string StateDatabase::BZDB_NOSMALLPACKETS    = std::string("_noSmallPackets");
const std::string StateDatabase::BZDB_NOTRESPONDINGTIME = std::string("_notRespondingTime");
const std::string StateDatabase::BZDB_OBESEFACTOR       = std::string("_obeseFactor");
const std::string StateDatabase::BZDB_PAUSEDROPTIME	= std::string("_pauseDropTime");
const std::string StateDatabase::BZDB_POSITIONTOLERANCE = std::string("_positionTolerance");
const std::string StateDatabase::BZDB_PYRBASE		= std::string("_pyrBase");
const std::string StateDatabase::BZDB_PYRHEIGHT		= std::string("_pyrHeight");
const std::string StateDatabase::BZDB_RADARLIMIT	= std::string("_radarLimit");
const std::string StateDatabase::BZDB_REJOINTIME	= std::string("_rejoinTime");
const std::string StateDatabase::BZDB_RELOADTIME	= std::string("_reloadTime");
const std::string StateDatabase::BZDB_RFIREADVEL	= std::string("_rFireAdVel");
const std::string StateDatabase::BZDB_RFIREADRATE       = std::string("_rFireAdRate");
const std::string StateDatabase::BZDB_RFIREADLIFE       = std::string("_rFireAdLife");
const std::string StateDatabase::BZDB_SHIELDFLIGHT      = std::string("_shieldFlight");
const std::string StateDatabase::BZDB_SHOCKADLIFE       = std::string("_shockAdLife");
const std::string StateDatabase::BZDB_SHOCKINRADIUS     = std::string("_shockInRadius");
const std::string StateDatabase::BZDB_SHOCKOUTRADIUS    = std::string("_shockOutRadius");
const std::string StateDatabase::BZDB_SHOTRADIUS	= std::string("_shotRadius");
const std::string StateDatabase::BZDB_SHOTRANGE		= std::string("_shotRange");
const std::string StateDatabase::BZDB_SHOTSPEED		= std::string("_shotSpeed");
const std::string StateDatabase::BZDB_SHOTTAILLENGTH	= std::string("_shotTailLength");
const std::string StateDatabase::BZDB_SHOTSKEEPVERTICALV= std::string("_shotsKeepVerticalVelocity");
const std::string StateDatabase::BZDB_SRRADIUSMULT	= std::string("_srRadiusMult");
const std::string StateDatabase::BZDB_SQUISHFACTOR	= std::string("_squishFactor");
const std::string StateDatabase::BZDB_SQUISHTIME	= std::string("_squishTime");
const std::string StateDatabase::BZDB_SYNCTIME		= std::string("_syncTime");
const std::string StateDatabase::BZDB_SYNCLOCATION      = std::string("_syncLocation");
const std::string StateDatabase::BZDB_TANKANGVEL	= std::string("_tankAngVel");
const std::string StateDatabase::BZDB_TANKEXPLOSIONSIZE = std::string("_tankExplosionSize");
const std::string StateDatabase::BZDB_TANKHEIGHT	= std::string("_tankHeight");
const std::string StateDatabase::BZDB_TANKLENGTH	= std::string("_tankLength");
const std::string StateDatabase::BZDB_TANKRADIUS	= std::string("_tankRadius");
const std::string StateDatabase::BZDB_TANKSPEED		= std::string("_tankSpeed");
const std::string StateDatabase::BZDB_TANKWIDTH		= std::string("_tankWidth");
const std::string StateDatabase::BZDB_TARGETINGANGLE	= std::string("_targetingAngle");
const std::string StateDatabase::BZDB_TELEBREADTH       = std::string("_teleportBreadth");
const std::string StateDatabase::BZDB_TELEHEIGHT	= std::string("_teleportHeight");
const std::string StateDatabase::BZDB_TELEPORTTIME      = std::string("_teleportTime");
const std::string StateDatabase::BZDB_TELEWIDTH		= std::string("_teleportWidth");
const std::string StateDatabase::BZDB_THIEFADLIFE	= std::string("_thiefAdLife");
const std::string StateDatabase::BZDB_THIEFADRATE       = std::string("_thiefAdRate");
const std::string StateDatabase::BZDB_THIEFADSHOTVEL    = std::string("_thiefAdShotVel");
const std::string StateDatabase::BZDB_THIEFTINYFACTOR   = std::string("_thiefTinyFactor");
const std::string StateDatabase::BZDB_THIEFVELAD	= std::string("_thiefVelAd");
const std::string StateDatabase::BZDB_THIEFDROPTIME     = std::string("_thiefDropTime");
const std::string StateDatabase::BZDB_TINYFACTOR	= std::string("_tinyFactor");
const std::string StateDatabase::BZDB_TRACKFADE		= std::string("_trackFade");
const std::string StateDatabase::BZDB_UPDATETHROTTLERATE= std::string("_updateThrottleRate");
const std::string StateDatabase::BZDB_VELOCITYAD	= std::string("_velocityAd");
const std::string StateDatabase::BZDB_WALLHEIGHT	= std::string("_wallHeight");
const std::string StateDatabase::BZDB_WEAPONS		= std::string("_weapons");
const std::string StateDatabase::BZDB_WIDEANGLEANG      = std::string("_wideAngleAng");
const std::string StateDatabase::BZDB_WINGSGRAVITY      = std::string("_wingsGravity");
const std::string StateDatabase::BZDB_WINGSJUMPCOUNT    = std::string("_wingsJumpCount");
const std::string StateDatabase::BZDB_WINGSJUMPVELOCITY = std::string("_wingsJumpVelocity");
const std::string StateDatabase::BZDB_WINGSSLIDETIME    = std::string("_wingsSlideTime");
const std::string StateDatabase::BZDB_WORLDSIZE		= std::string("_worldSize");

//
// StateDatabase::Item
//

StateDatabase::Item::Item() : value(),
			      defValue(),
			      isSet(false),
			      isTrue(false),
			      save(true), // FIXME -- false by default?
			      permission(ReadWrite)
{
  // do nothing
}


//
// StateDatabase
//


StateDatabase::StateDatabase() : debug(false), saveDefault(false)
{
  // do nothing
}

StateDatabase::~StateDatabase()
{
  // do nothing
}

void			StateDatabase::set(const std::string& name,
					   const std::string& value,
					   Permission access)
{
  Map::iterator index = lookup(name);
  if (access >= index->second.permission) {
    index->second.value  = value;
    index->second.isSet  = true;
    index->second.isTrue = (index->second.value != "0" &&
			    index->second.value != "False" &&
			    index->second.value != "false" &&
			    index->second.value != "FALSE" &&
			    index->second.value != "no" &&
			    index->second.value != "No" &&
			    index->second.value != "NO" &&
			    index->second.value != "disable");

	if (saveDefault)
		index->second.defValue = value;
    notify(index);
  }
}

void		     StateDatabase::setInt(const std::string& name,
					   const int& value,
					   Permission access)
{
  set(name,TextUtils::format("%d",value),access);
}

void		    StateDatabase::setBool(const std::string& name,
					   const bool& value,
					   Permission access)
{
  set(name,value ? std::string("1") : std::string("0"),access);
}

void		   StateDatabase::setFloat(const std::string& name,
					   const float& value,
					   Permission access)
{
  set(name,TextUtils::format("%f",value),access);
}

void			StateDatabase::setPointer(const std::string& name,
					   const void * value,
					   Permission access)
{
  char address[32];
  memset(address, 0, 32);
  snprintf(address, 32, "%lu", (unsigned long)value);
  std::string ssaddress = address;
  this->set(name, ssaddress, access);
}


void			StateDatabase::unset(const std::string& name,
					     Permission access)
{
  Map::iterator index = lookup(name);
  if (access >= index->second.permission) {
    index->second.value  = "";
    index->second.isSet  = false;
    index->second.isTrue = false;
    notify(index);
  }
}

void			StateDatabase::touch(const std::string& name,
					     Permission access)
{
  Map::iterator index = lookup(name);
  if (access >= index->second.permission)
    notify(index);
}

void			StateDatabase::setPersistent(
					const std::string& name, bool save)
{
  Map::iterator index = lookup(name);
  index->second.save = save;
}

void			StateDatabase::setDefault(
					const std::string& name, const std::string& value)
{
  Map::iterator index = lookup(name);
  index->second.defValue = value;
}

void			StateDatabase::setPermission(
					const std::string& name,
					Permission permission)
{
  Map::iterator index = lookup(name);
  index->second.permission = permission;
}

void			StateDatabase::addCallback(
					const std::string& name,
					Callback callback,
					void* userData)
{
  Map::iterator index = lookup(name);
  index->second.callbacks.add(callback, userData);
}

void			StateDatabase::removeCallback(
					const std::string& name,
					Callback callback,
					void* userData)
{
  Map::iterator index = lookup(name);
  index->second.callbacks.remove(callback, userData);
}

bool			StateDatabase::isSet(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  return !(index == items.end() || !index->second.isSet);
}

std::string		StateDatabase::get(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  if (index == items.end() || !index->second.isSet)
    return std::string();
  else
    return index->second.value;
}

int		StateDatabase::getIntClamped(const std::string& name, const int min, const int max) const
{
  int val;
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  if (index == items.end() || !index->second.isSet)
    val=0;
  else
    val = atoi(index->second.value.c_str());
  if (val < min)
    return min;
  else if (val > max)
    return max;
  return val;
}

void *		StateDatabase::getPointer(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  if (index == items.end() || !index->second.isSet)
    return (void *)NULL;
  else
    return (void *)strtoul(index->second.value.c_str(), NULL, 0);
}

float			StateDatabase::eval(const std::string& name)
{
  typedef std::set<std::string> VariableSet;
  debugLookups(name);

  EvalMap::const_iterator cit = evalCache.find(name);
  if (cit != evalCache.end())
    return cit->second;

  //this is to catch recursive definitions
  static VariableSet variables;
  // ugly hack, since gcc 2.95 doesn't have <limits>
  float NaN;
  memset(&NaN, 0xff, sizeof(float));

  if (variables.find(name) != variables.end())
    return NaN;

  VariableSet::iterator ins_it = variables.insert(name).first;

  Map::const_iterator index = items.find(name);
  if (index == items.end() || !index->second.isSet || index->second.value.empty()) {
    variables.erase(ins_it);
    return NaN;
  }
  Expression pre, inf;
  std::string value = index->second.value;
  value >> inf;
  pre = infixToPrefix(inf);
  float retn = evaluate(pre);
  variables.erase(ins_it);

  evalCache[name] = retn;
  return retn;
}

int			StateDatabase::evalInt(const std::string& name)
{
  return (int)eval(name);
}

bool		StateDatabase::evalTriplet(const std::string& name, float data[4])
{
  if (!isSet(name) || !data)
    return false;
  if (sscanf(get(name).c_str(), "%f %f %f", data, data+1, data+2) != 3)
    return false;
  return true;
}

bool		StateDatabase::evalPair(const std::string& name, float data[2])
{
  if (!isSet(name) || !data)
    return false;
  if (sscanf(get(name).c_str(), "%f %f", data, data+1) != 2)
    return false;
  return true;
}

bool			StateDatabase::isTrue(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  return !(index == items.end() || !index->second.isTrue);
}

bool			StateDatabase::isEmpty(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  return (index == items.end() || !index->second.isSet ||
	  index->second.value.empty());
}

bool			StateDatabase::isPersistent(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  return (index != items.end() && index->second.save);
}

std::string		StateDatabase::getDefault(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  if (index != items.end())
    return index->second.defValue;
  else
    return "";
}

StateDatabase::Permission
			StateDatabase::getPermission(const std::string& name) const
{
  debugLookups(name);
  Map::const_iterator index = items.find(name);
  if (index != items.end())
    return index->second.permission;
  else
    return ReadWrite;
}

StateDatabase::Map::iterator
			StateDatabase::lookup(const std::string& name)
{
  debugLookups(name);
  Map::iterator index = items.find(name);
  if (index == items.end()) {
    Item tmp;
    return items.insert(std::make_pair(name, tmp)).first;
  } else {
    return index;
  }
}

void			StateDatabase::notify(Map::iterator index)
{
  evalCache.erase(index->first);
  index->second.callbacks.iterate(&onCallback, const_cast<void*>(static_cast<const void*>(&index->first)));
}

bool			StateDatabase::onCallback(Callback callback,
						  void* userData,
						  void* iterateData)
{
  callback(*static_cast<std::string*>(iterateData), userData);
  return true;
}

void			StateDatabase::iterate(Callback callback, void* userData) const
{
  assert(callback != NULL);

  for (Map::const_iterator index = items.begin(); index != items.end(); ++index) {
    if (index->second.isSet) {
      (*callback)(index->first, userData);
    }
  }
}

void			StateDatabase::write(Callback callback, void* userData) const
{
  assert(callback != NULL);

  for (Map::const_iterator index = items.begin(); index != items.end(); ++index) {
    if (index->second.isSet && index->second.save) {
      (*callback)(index->first, userData);
    }
  }
}

void		     StateDatabase::setDebug(bool print) {
  debug = print;
}

void		     StateDatabase::setSaveDefault(bool save) {
	saveDefault = save;
}

StateDatabase::ExpressionToken::ExpressionToken()
{
  tokenType = number;
  tokenContents.number = 0;
}

StateDatabase::ExpressionToken::ExpressionToken(Type _tokenType)
{
  tokenType = _tokenType;
  switch(tokenType) {
    case number:
      tokenContents.number = 0;
      break;
    case variable:
      break;
    case oper:
      tokenContents.oper = none;
      break;
  }
}

StateDatabase::ExpressionToken::ExpressionToken(Type _tokenType, Contents _tokenContents)
{
  tokenType = _tokenType;
  tokenContents = _tokenContents;
}

void StateDatabase::ExpressionToken::setType(Type _tokenType)
{
  tokenType = _tokenType;
  switch(tokenType) {
    case number:
      tokenContents.number = 0;
      break;
    case variable:
      break;
    case oper:
      tokenContents.oper = none;
      break;
  }
}

void StateDatabase::ExpressionToken::setContents(Contents _tokenContents)
{
  tokenContents = _tokenContents;
}

void StateDatabase::ExpressionToken::setNumber(double num)
{
  tokenType = number;
  tokenContents.number = num;
}

void StateDatabase::ExpressionToken::setVariable(std::string var)
{
  tokenType = variable;
  tokenContents.variable = var;
}

void StateDatabase::ExpressionToken::setOper(Operator op)
{
  tokenType = oper;
  tokenContents.oper = op;
}

StateDatabase::ExpressionToken::Type StateDatabase::ExpressionToken::getTokenType() const
{
  return tokenType;
}

StateDatabase::ExpressionToken::Contents StateDatabase::ExpressionToken::getTokenContents() const
{
  return tokenContents;
}

double StateDatabase::ExpressionToken::getNumber() const
{
  // note that the necessary type check must be done first
  return tokenContents.number;
}

std::string	StateDatabase::ExpressionToken::getVariable() const
{
  // note that the necessary type check must be done first
  return tokenContents.variable;
}

StateDatabase::ExpressionToken::Operator StateDatabase::ExpressionToken::getOperator() const
{
  // note that the necessary type check must be done first
  return tokenContents.oper;
}

int StateDatabase::ExpressionToken::getPrecedence() const
{
  switch (tokenContents.oper) {
    case add:
    case subtract:
      return 1;
    case multiply:
    case divide:
      return 2;
    case power:
      return 3;
    case lparen:
      return 4;
    case rparen:
    default:
      return 0;
  }
}

std::istream&operator >> (std::istream& src, StateDatabase::ExpressionToken& dst)
{
  char temp;
  std::string tempname;

  src >> temp;
  if ((temp >= '0' && temp <= '9') || temp == '.') {
    // number
    tempname += temp;
    temp = src.peek();
    while ((temp >= '0' && temp <= '9') || temp == '.') {
      src >> temp;
      tempname += temp;
      temp = src.peek();
    }
    dst.setNumber(atof(tempname.c_str()));
  } else if (temp == '+' || temp == '-' || temp == '*' || temp == '/' || temp == '^' || temp == '(' || temp == ')') {
    // operator
    switch(temp) {
      case '+':
	dst.setOper(StateDatabase::ExpressionToken::add);
	break;
      case '-':
	dst.setOper(StateDatabase::ExpressionToken::subtract);
	break;
      case '*':
	dst.setOper(StateDatabase::ExpressionToken::multiply);
	break;
      case '/':
	dst.setOper(StateDatabase::ExpressionToken::divide);
	break;
      case '^':
	dst.setOper(StateDatabase::ExpressionToken::power);
	break;
      case '(':
	dst.setOper(StateDatabase::ExpressionToken::lparen);
	break;
      case ')':
	dst.setOper(StateDatabase::ExpressionToken::rparen);
	break;
    }
  } else if ((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') {
    // variable (perhaps prefix with $?)
    tempname += temp;
    temp = src.peek();
    while ((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') {
      src >> temp;
      tempname += temp;
      temp = src.peek();
    }
    dst.setVariable(tempname);
  } else {
    // throw an error?
  }
  return src;
}

std::string& operator >> (std::string& src, StateDatabase::ExpressionToken& dst)
{
  char temp;
  std::string tempname;

  temp = src[0]; src = src.substr(1);
  if ((temp >= '0' && temp <= '9') || temp == '.') {
    // number
    tempname += temp;
    temp = src[0];
    while (((temp >= '0' && temp <= '9') || temp == '.') && (src.length() != 0)) {
      src = src.substr(1);
      tempname += temp;
      temp = src[0];
    }
    dst.setNumber(atof(tempname.c_str()));
  } else if (temp == '+' || temp == '-' || temp == '*' || temp == '/' || temp == '^' || temp == '(' || temp == ')') {
    // operator
    switch (temp) {
      case '+':
	dst.setOper(StateDatabase::ExpressionToken::add);
	break;
      case '-':
	dst.setOper(StateDatabase::ExpressionToken::subtract);
	break;
      case '*':
	dst.setOper(StateDatabase::ExpressionToken::multiply);
	break;
      case '/':
	dst.setOper(StateDatabase::ExpressionToken::divide);
	break;
      case '^':
	dst.setOper(StateDatabase::ExpressionToken::power);
	break;
      case '(':
	dst.setOper(StateDatabase::ExpressionToken::lparen);
	break;
      case ')':
	dst.setOper(StateDatabase::ExpressionToken::rparen);
	break;
    }
  } else if ((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') {
    tempname += temp;
    temp = src[0];
    while (((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') && (src.length() != 0)) {
      src = src.substr(1);
      tempname += temp;
      temp = src[0];
    }
    dst.setVariable(tempname);
  } else {
    // throw an error?
  }
  return src;
}

std::ostream& operator << (std::ostream& dst, const StateDatabase::ExpressionToken& src)
{
  switch (src.getTokenType()) {
    case StateDatabase::ExpressionToken::number:
      dst << src.getNumber();
      break;
    case StateDatabase::ExpressionToken::oper:
      switch (src.getOperator()) {
	case StateDatabase::ExpressionToken::add:
	  dst << '+';
	  break;
	case StateDatabase::ExpressionToken::subtract:
	  dst << '-';
	  break;
	case StateDatabase::ExpressionToken::multiply:
	  dst << '*';
	  break;
	case StateDatabase::ExpressionToken::divide:
	  dst << '/';
	  break;
	case StateDatabase::ExpressionToken::power:
	  dst << '^';
	  break;
	case StateDatabase::ExpressionToken::lparen:
	  dst << '(';
	  break;
	case StateDatabase::ExpressionToken::rparen:
	  dst << ')';
	  break;
	case StateDatabase::ExpressionToken::none:
	  break;
      }
      break;
    case StateDatabase::ExpressionToken::variable:
      dst << src.getVariable();
      break;
  }
  return dst;
}

std::istream& operator >> (std::istream& src, StateDatabase::Expression& dst)
{
  StateDatabase::ExpressionToken temp;
  char tempc;

  dst.clear();
  src.unsetf(std::ios::skipws);
  while (src.peek() != '\n') {
    while (src.peek() == ' ' || src.peek() == '\t')
      src >> tempc;
    src >> temp;
    dst.push_back(temp);
  }
  src >> tempc;
  src.setf(std::ios::skipws);
  return src;
}

std::string& operator >> (std::string& src, StateDatabase::Expression& dst)
{
  StateDatabase::ExpressionToken temp;

  dst.clear();
  while (src.length() != 0) {
    while (src[0] == ' ' || src[0] == '\t') {
      src = src.substr(1);
    }
    if (src.length() != 0) {
      src >> temp;
      dst.push_back(temp);
    }
  }
  return src;
}

std::ostream& operator << (std::ostream& dst, const StateDatabase::Expression& src)
{
  if (src.size()) {
    for (unsigned int i = 0; i < src.size() - 1; i++) {
      dst << src[i] << ' ';
    }
    dst << src[src.size() - 1];
  }
  return dst;
}

StateDatabase::Expression StateDatabase::infixToPrefix(const Expression &infix)
{
  Expression postfix, prefix;
  std::stack<ExpressionToken> operators;

  for (Expression::const_iterator i = infix.begin(); i != infix.end(); i++) {
    if (i->getTokenType() == ExpressionToken::variable || i->getTokenType() == ExpressionToken::number) {
      postfix.push_back(*i);
    } else if (i->getTokenType() == ExpressionToken::oper) {
      if (i->getOperator() == ExpressionToken::lparen) {
	operators.push(*i);
      } else if (i->getOperator() == ExpressionToken::rparen) {
	// unstack operators until a matching ( is found
	while(operators.top().getOperator() != ExpressionToken::lparen) {
	  postfix.push_back(operators.top()); operators.pop();
	}
	// discard (
	operators.pop();
      } else {
	while((operators.size() > 0) && (operators.top().getPrecedence() < i->getPrecedence()) && (operators.top().getOperator() != ExpressionToken::lparen)) {
	  postfix.push_back(operators.top()); operators.pop();
	}
	operators.push(*i);
      }
    }
  }
  while (operators.size() > 0) {
    postfix.push_back(operators.top()); operators.pop();
  }

  for (Expression::reverse_iterator ri = postfix.rbegin(); ri != postfix.rend(); ri++)
    prefix.push_back(*ri);
  return prefix;
}

float StateDatabase::evaluate(Expression e) const
{
  std::stack<ExpressionToken> evaluationStack;
  ExpressionToken tok, lvalue, rvalue;
  bool unary;

  for (Expression::reverse_iterator i = e.rbegin(); i != e.rend(); i++) {
    unary = false;
    switch(i->getTokenType()) {
      case ExpressionToken::number:
	evaluationStack.push(*i);
	break;
      case ExpressionToken::variable:
	// strip off '$'?
	tok.setNumber(BZDB.eval(i->getVariable()));
	evaluationStack.push(tok);
	break;
      case ExpressionToken::oper:
	if (evaluationStack.size() == 0) {
	  // syntax error
	}
	// rvalue and lvalue are switched, since we're reversed
	rvalue = evaluationStack.top(); evaluationStack.pop();
	if (evaluationStack.size() == 0) {
	  unary = true; // syntax error or unary operator
	}
	if (!unary) {
	  lvalue = evaluationStack.top(); evaluationStack.pop();
	}
	switch(i->getOperator()) {
	  case ExpressionToken::add:
	    tok.setNumber(lvalue.getNumber() + rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  case ExpressionToken::subtract:
	    if (unary)
	      tok.setNumber(-rvalue.getNumber());
	    else
	      tok.setNumber(lvalue.getNumber() - rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  case ExpressionToken::multiply:
	    tok.setNumber(lvalue.getNumber() * rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  case ExpressionToken::divide:
	    tok.setNumber(lvalue.getNumber() / rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  case ExpressionToken::power:
	    tok.setNumber(pow(lvalue.getNumber(), rvalue.getNumber()));
	    evaluationStack.push(tok);
	    break;
	default:
	  // lparen and rparen should have been stripped out
	  // throw something here, too
	  break;
	}
	break;
    }
  }
  if (!evaluationStack.size())
	  return 0; // yeah we are screwed. TODO, don't let us get this far
  return (float)evaluationStack.top().getNumber();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
