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

#ifndef BZF_STATE_DATABASE_H
#define BZF_STATE_DATABASE_H

// common header first
#include "common.h"

// system headers
#include <vector>
#include <map>
#include <iostream>
#include <string>

// implementation headers
#include "CallbackList.h"
#include "bzfio.h"
#include "Singleton.h"

#define BZDB (StateDatabase::instance())

/** BZDB is the generic name:value pair database within bzflag and bzfs. Its
 * useful for data that can be serialized to a string that needs to be
 * accessible to many areas of the code. It also provides facilities for
 * saving persistant pairs to the config file and downloading variables from
 * the server.
 *
 * BZDB is not an object broker, and isn't meant to be. If you have data
 * within an object that needs to be accessible from a number of places, but
 * don't want to pass the object around, you could store that data within BZDB
 * (if accessed often, such as game variables like gravity, you will need a
 * cached version anyway to avoid the overhead of lookup). Using BZDB adds
 * unnecessary overhead if objects generally keep their data hidden without
 * needing persistant state.
 *
 * Basically, if your data can be serialized to a string, and it makes sense
 * to do so (eg: config file option, game variable downloaded from server), use
 * BZDB. If you wanted an object broker, use a freakin' global.
 */
class StateDatabase : public Singleton<StateDatabase>
{
public:

  typedef void (*Callback)(const std::string& name, void* userData);
  enum Permission {
    // permission levels
    ReadWrite,
    Locked,
    ReadOnly,

    // access levels
    User   = ReadWrite,
    Server = Locked,
    Client = ReadOnly
  };

  /** set a name/value pair.  if access is less than the permission
   *level of the name then this has no effect.
   */
  void				set(const std::string& name,
				    const std::string& value,
				    Permission access = Client);

  void			     setInt(const std::string& name,
				    const int& value,
				    Permission access = Client);

  void			    setBool(const std::string& name,
				    const bool& value,
				    Permission access = Client);

  void			   setFloat(const std::string& name,
				    const float& value,
				    Permission access = Client);
  /** allow setting name/pointer pairs.  this allows simple object
   * pointer storage into the bzdb via pointer address serialization.
   */
  void				setPointer(const std::string& name,
				    const void *value,
				    Permission access = Client);

  /** unset a name if access is not less than the permission level
   * of the name.
   */
  void				unset(const std::string& name,
				      Permission access = Client);

  /** simulate a change to a value (i.e. invoke the callbacks on it)
   */
  void				touch(const std::string& name,
				      Permission access = Client);

  /** mark a value as persistent (i.e. to be saved) or volatile.
   * this state is stored independently of the existance of a value
   * with the given name.  that is, adding or removing the name
   * will not affect persistence of the name.  the default is
   * volatile.
   */
  void				setPersistent(const std::string& name,
					      bool = true);

  /** set the default value for a name.  if the default value is set
   * then the value will not be written by write() if the current
   * value is equal to the default value.
   */
  void				setDefault(const std::string& name,
					   const std::string& value);

  /** set the permission level of a name.  like persistence, this is
   * stored independently of a value with the name.  the default
   * permission is ReadWrite (i.e. full access).
   */
  void				setPermission(const std::string& name,
					      Permission);

  /** add/remove a callback to/from a name.  all callbacks on a name are
   * invoked when the value changes (either by being set or unset).
   * each name can have any number of callbacks but any given callback
   * function/userData pair on a name can only be registered once (i.e.
   * multiple adds have the same effect as a single add).
   */
  void				addCallback(const std::string& name,
					    Callback, void* userData);
  void				removeCallback(const std::string& name,
					       Callback, void* userData);

  /** test if a name is set or not
   */
  bool				isSet(const std::string& name) const;

  /** get the value associated with a name.  returns the empty string
   * if the name isn't set.
   */
  std::string			get(const std::string& name) const;

  /** get the INT value associated with a name. Clamp value within range.
   *  Returns 0 (or min) if value is not set.
   */
  int	getIntClamped(const std::string& name, const int min, const int max) const;

  /** returns the pointer stored with a name.  returns NULL if the
   * name isn't set.
   */
  void *			getPointer(const std::string& name) const;

  /** get the value as a floating point number. this will evaluate
   * the string as an expression
   */
  float				eval(const std::string& name);
  int				evalInt(const std::string& name);
  bool				evalTriplet(const std::string& name, float data[3]);
  bool				evalPair(const std::string& name, float data[2]);

  /** return true if the value associated with a name indicates
   * logical true, which is when the value is not empty and not
   * "0" and not "false" and not "no".
   */
  bool				isTrue(const std::string& name) const;

  /** test if a name is empty or not.  a name is empty if it's
   * not set or it's set to the empty string.
   */
  bool				isEmpty(const std::string& name) const;

  /** get the persistence, permission, and default for an entry
   */
  bool				isPersistent(const std::string& name) const;
  std::string			getDefault(const std::string& name) const;
  Permission			getPermission(const std::string& name) const;

  /** invoke the callback for each entry
   */
  void				iterate(Callback, void* userData) const;

  /** invoke the callback for each entry that should be written (i.e.
   * is set, persistent, and not the default).
   */
  void				write(Callback, void* userData) const;

  /** tell the state database whether it should print debug info to stdout
   * now and then.
   */
  void			  setDebug(bool print);

  /** do we want debug output?
   */
  bool			  getDebug() const;


  static const std::string	BZDB_AGILITYADVEL;
  static const std::string	BZDB_AGILITYTIMEWINDOW;
  static const std::string	BZDB_AGILITYVELDELTA;
  static const std::string	BZDB_ANGLETOLERANCE;
  static const std::string	BZDB_ANGULARAD;
  static const std::string	BZDB_AVENUESIZE;
  static const std::string	BZDB_BASESIZE;
  static const std::string	BZDB_BOXBASE;
  static const std::string	BZDB_BOXHEIGHT;
  static const std::string	BZDB_BURROWDEPTH;
  static const std::string	BZDB_BURROWSPEEDAD;
  static const std::string	BZDB_BURROWANGULARAD;
  static const std::string	BZDB_COLDETDEPTH;
  static const std::string	BZDB_COLDETELEMENTS;
  static const std::string	BZDB_CULLDEPTH;
  static const std::string	BZDB_CULLELEMENTS;
  static const std::string	BZDB_CULLOCCLUDERS;
  static const std::string	BZDB_DISABLEBOTS;
  static const std::string	BZDB_DRAWCELESTIAL;
  static const std::string	BZDB_DRAWCLOUDS;
  static const std::string	BZDB_DRAWGROUND;
  static const std::string	BZDB_DRAWGROUNDLIGHTS;
  static const std::string	BZDB_DRAWMOUNTAINS;
  static const std::string	BZDB_DRAWSKY;
  static const std::string	BZDB_ENDSHOTDETECTION;
  static const std::string	BZDB_EXPLODETIME;
  static const std::string	BZDB_FLAGALTITUDE;
  static const std::string	BZDB_FLAGEFFECTTIME;
  static const std::string	BZDB_FLAGHEIGHT;
  static const std::string	BZDB_FLAGPOLEWIDTH;
  static const std::string	BZDB_FLAGPOLESIZE;
  static const std::string	BZDB_FLAGRADIUS;
  static const std::string	BZDB_FOGMODE;
  static const std::string	BZDB_FOGDENSITY;
  static const std::string	BZDB_FOGSTART;
  static const std::string	BZDB_FOGEND;
  static const std::string	BZDB_FOGCOLOR;
  static const std::string	BZDB_GMACTIVATIONTIME;
  static const std::string	BZDB_GMADLIFE;
  static const std::string	BZDB_GMTURNANGLE;
  static const std::string	BZDB_GRAVITY;
  static const std::string	BZDB_FRICTION;
  static const std::string	BZDB_HANDICAPSCOREDIFF;
  static const std::string	BZDB_HANDICAPVELAD;
  static const std::string	BZDB_HANDICAPANGAD;
  static const std::string	BZDB_HANDICAPSHOTAD;
  static const std::string	BZDB_IDENTIFYRANGE;
  static const std::string	BZDB_JUMPVELOCITY;
  static const std::string	BZDB_LASERADVEL;
  static const std::string	BZDB_LASERADRATE;
  static const std::string	BZDB_LASERADLIFE;
  static const std::string	BZDB_LATITUDE;
  static const std::string	BZDB_LOCKONANGLE;
  static const std::string	BZDB_LONGITUDE;
  static const std::string	BZDB_LRADRATE;
  static const std::string	BZDB_MAXBUMPHEIGHT;
  static const std::string	BZDB_MAXFLAGGRABS;
  static const std::string	BZDB_MAXLOD;
  static const std::string	BZDB_MIRROR;
  static const std::string	BZDB_MOMENTUMLINACC;
  static const std::string	BZDB_MOMENTUMANGACC;
  static const std::string	BZDB_MOMENTUMFRICTION;
  static const std::string	BZDB_MGUNADVEL;
  static const std::string	BZDB_MGUNADRATE;
  static const std::string	BZDB_MGUNADLIFE;
  static const std::string	BZDB_MUZZLEFRONT;
  static const std::string	BZDB_MUZZLEHEIGHT;
  static const std::string	BZDB_NOCLIMB;
  static const std::string	BZDB_NOSHADOWS;
  static const std::string	BZDB_NOSMALLPACKETS;
  static const std::string	BZDB_NOTRESPONDINGTIME;
  static const std::string	BZDB_OBESEFACTOR;
  static const std::string	BZDB_PAUSEDROPTIME;
  static const std::string	BZDB_POSITIONTOLERANCE;
  static const std::string	BZDB_PYRBASE;
  static const std::string	BZDB_PYRHEIGHT;
  static const std::string	BZDB_RADARLIMIT;
  static const std::string	BZDB_REJOINTIME;
  static const std::string	BZDB_RELOADTIME;
  static const std::string	BZDB_RFIREADVEL;
  static const std::string	BZDB_RFIREADRATE;
  static const std::string	BZDB_RFIREADLIFE;
  static const std::string	BZDB_SHIELDFLIGHT;
  static const std::string	BZDB_SHOCKADLIFE;
  static const std::string	BZDB_SHOCKINRADIUS;
  static const std::string	BZDB_SHOCKOUTRADIUS;
  static const std::string	BZDB_SHOTSPEED;
  static const std::string	BZDB_SHOTRADIUS;
  static const std::string	BZDB_SHOTRANGE;
  static const std::string	BZDB_SHOTTAILLENGTH;
  static const std::string	BZDB_SHOTSKEEPVERTICALV;
  static const std::string	BZDB_SQUISHFACTOR;
  static const std::string	BZDB_SQUISHTIME;
  static const std::string	BZDB_SRRADIUSMULT;
  static const std::string	BZDB_SYNCLOCATION;
  static const std::string	BZDB_SYNCTIME;
  static const std::string	BZDB_TANKANGVEL;
  static const std::string	BZDB_TANKEXPLOSIONSIZE;
  static const std::string	BZDB_TANKLENGTH;
  static const std::string	BZDB_TANKWIDTH;
  static const std::string	BZDB_TANKHEIGHT;
  static const std::string	BZDB_TANKSPEED;
  static const std::string	BZDB_TANKRADIUS;
  static const std::string	BZDB_TARGETINGANGLE;
  static const std::string	BZDB_TELEBREADTH;
  static const std::string	BZDB_TELEHEIGHT;
  static const std::string	BZDB_TELEPORTTIME;
  static const std::string	BZDB_TELEWIDTH;
  static const std::string	BZDB_THIEFVELAD;
  static const std::string	BZDB_THIEFTINYFACTOR;
  static const std::string	BZDB_THIEFADSHOTVEL;
  static const std::string	BZDB_THIEFADRATE;
  static const std::string	BZDB_THIEFADLIFE;
  static const std::string	BZDB_THIEFDROPTIME;
  static const std::string	BZDB_TINYFACTOR;
  static const std::string	BZDB_TRACKFADE;
  static const std::string	BZDB_UPDATETHROTTLERATE;
  static const std::string	BZDB_VELOCITYAD;
  static const std::string	BZDB_WALLHEIGHT;
  static const std::string	BZDB_WEAPONS;
  static const std::string	BZDB_WIDEANGLEANG;
  static const std::string	BZDB_WINGSGRAVITY;
  static const std::string	BZDB_WINGSJUMPCOUNT;
  static const std::string	BZDB_WINGSJUMPVELOCITY;
  static const std::string	BZDB_WINGSSLIDETIME;
  static const std::string	BZDB_WORLDSIZE;


protected:
  friend class Singleton<StateDatabase>;

private:

  StateDatabase();
  ~StateDatabase();

  static bool			onCallback(Callback, void* userData,
					   void* iterateData);

  struct Item {
  public:
    Item();

  public:
    std::string			value;
    std::string			defValue;
    bool			isSet;
    bool			isTrue;
    bool			save;
    Permission			permission;
    CallbackList<Callback>	callbacks;
  };
  typedef std::map<std::string, Item> Map;

  Map::iterator			lookup(const std::string&);
  void				notify(Map::iterator);

private:
  Map				items;

public:
  class ExpressionToken {
  public:
    enum Type { oper, number, variable };
    enum Operator { add, subtract, multiply, divide, power, lparen, rparen, none };
    struct Contents {
    public:
      double number;
      std::string variable;
      Operator oper;
    };

    ExpressionToken();
    ExpressionToken(Type _tokenType);
    ExpressionToken(Type _tokenType, Contents _tokenContents);

    void			setType(Type _tokenType);
    void			setContents(Contents _tokenContents);
    void			setNumber(double number);
    void			setVariable(std::string variable);
    void			setOper(Operator oper);

    Type			getTokenType() const;
    Contents			getTokenContents() const;
    double			getNumber() const;
    std::string			getVariable() const;
    Operator			getOperator() const;

    int				getPrecedence() const;

private:
    Type tokenType;
    Contents tokenContents;
  };

  typedef std::vector<ExpressionToken> Expression;

private:
  static Expression		infixToPrefix(const Expression &infix);
  float				evaluate(Expression e) const;
  typedef std::map<std::string,float> EvalMap;
  EvalMap			evalCache;
  bool			  debug;
};

inline bool StateDatabase::getDebug() const
{
  return debug;
}

std::istream& operator >> (std::istream& src, StateDatabase::Expression& dst);
std::string& operator >> (std::string& src, StateDatabase::Expression& dst);
std::ostream& operator << (std::ostream& dst, const StateDatabase::Expression& src);

#endif // BZF_STATE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
