/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_STATE_DATABASE_H
#define BZF_STATE_DATABASE_H

#include "common.h"
#include <string>
#include <vector>
#include "CallbackList.h"
#include "bzfio.h"

#define BZDB (StateDatabase::getInstance())

/** would somebody please document this durn thing ;)  at least how
 * it should be used: how to add, modify, and retrieve data.
 */
class StateDatabase {
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

  ~StateDatabase();

  /** set a name/value pair.  if access is less than the permission
   *level of the name then this has no effect.
   */
  void				set(const std::string& name,
				    const std::string& value,
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

  /** get the value as a floating point number. this will evaluate
   * the string as an expression
   */
  float				eval(const std::string& name);

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

  /** get the singleton instance of the state database
   */
  static StateDatabase* getInstance();

  static const std::string	BZDB_ANGULARAD;
  static const std::string	BZDB_BURROWDEPTH;
  static const std::string	BZDB_BURROWSPEEDAD;
  static const std::string	BZDB_BURROWANGULARAD;
  static const std::string	BZDB_EXPLODETIME;
  static const std::string	BZDB_FLAGALTITUDE;
  static const std::string	BZDB_FLAGRADIUS;
  static const std::string	BZDB_GMISSILEANG;
  static const std::string	BZDB_GRAVITY;
  static const std::string	BZDB_IDENTIFYRANGE;
  static const std::string	BZDB_JUMPVELOCITY;
  static const std::string	BZDB_LASERADVEL;
  static const std::string	BZDB_LASERADRATE;
  static const std::string	BZDB_LASERADLIFE;
  static const std::string	BZDB_LOCKONANGLE;
  static const std::string	BZDB_LRADRATE;
  static const std::string	BZDB_MOMENTUMLINACC;
  static const std::string	BZDB_MOMENTUMANGACC;
  static const std::string	BZDB_MGUNADVEL;
  static const std::string	BZDB_MGUNADRATE;
  static const std::string	BZDB_MGUNADLIFE;
  static const std::string	BZDB_MUZZLEFRONT;
  static const std::string	BZDB_MUZZLEHEIGHT;
  static const std::string	BZDB_OBESEFACTOR;
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
  static const std::string	BZDB_SHOTLENGTH;
  static const std::string	BZDB_SHOTTAILLENGTH;
  static const std::string	BZDB_SRRADIUSMULT;
  static const std::string	BZDB_TANKLENGTH;
  static const std::string	BZDB_TANKWIDTH;
  static const std::string	BZDB_TANKHEIGHT;
  static const std::string	BZDB_TANKSPEED;
  static const std::string	BZDB_TANKRADIUS;
  static const std::string	BZDB_TANKANGVEL;
  static const std::string	BZDB_TARGETINGANGLE;
  static const std::string	BZDB_TELEPORTTIME;
  static const std::string	BZDB_THIEFVELAD;
  static const std::string	BZDB_THIEFTINYFACTOR;
  static const std::string	BZDB_THIEFADSHOTVEL;
  static const std::string	BZDB_THIEFADRATE;
  static const std::string	BZDB_TINYFACTOR;
  static const std::string	BZDB_VELOCITYAD;
  static const std::string	BZDB_WIDEANGLEANG;
  static const std::string	BZDB_WORLDSIZE;

private:
  StateDatabase();

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
  static StateDatabase*	s_instance;

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
};

std::istream& operator >> (std::istream& src, StateDatabase::Expression& dst);
std::string& operator >> (std::string& src, StateDatabase::Expression& dst);
std::ostream& operator << (std::ostream& dst, const StateDatabase::Expression& src);

#endif // BZF_STATE_DATABASE_H
// ex: shiftwidth=2 tabstop=8
