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
#include <string.h>

// local implementation headers
#include "ErrorHandler.h"
#include "TextUtils.h"


#if defined(DEBUG) || defined(_DEBUG)
// headers needed only for _debugLookups()
#include <map>
#include "BzTime.h"


void _debugLookups(const std::string &name)
{
  typedef std::map<std::string, int> EvalCntMap;
  static const float interval = 20.0f;

  /* This bit of nastyness help debug BDZB->eval accesses sorted from worst to best*/
  static EvalCntMap cnts;
  static BzTime last = BzTime::getCurrent();

  EvalCntMap::iterator it = cnts.find(name);
  if (it == cnts.end())
    cnts.insert(std::pair<std::string, int>(name, 1));
  else
    it->second++;

  BzTime now = BzTime::getCurrent();
  if ((now - last) <= interval) {
    return;
  }

  std::multimap<int, std::string> order;
  for (it = cnts.begin(); it != cnts.end(); it++) {
    order.insert(std::pair<int, std::string>(-it->second, it->first));
    it->second = 0;
  }

  for (std::multimap<int, std::string>::iterator it2 = order.begin(); it2 != order.end(); ++it2) {
    if (-it2->first / interval < 1.0f)
      break;
    logDebugMessage(1, "%-25s = %.2f acc/sec\n", it2->second.c_str(), -it2->first / interval);
  }

  last = now;
}

#  define DEBUG_LOOKUPS(name) if (getDebug()) { _debugLookups(name); }
#else
#  define DEBUG_LOOKUPS(name)
#endif // defined(DEBUG) || defined(_DEBUG)


// initialize the singleton
template <>
StateDatabase* Singleton<StateDatabase>::_instance = (StateDatabase*)NULL;


//============================================================================//
//
// StateDatabase::Item
//

StateDatabase::Item::Item()
: value()
, defValue()
, isSet(false)
, isTrue(false)
, save(true) // FIXME -- false by default?
, permission(ReadWrite)
{
  // do nothing
}


//============================================================================//
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


void StateDatabase::set(const std::string& name,
			const std::string& value,
			Permission accessLevel)
{

  ItemMap::iterator it = lookup(name);
  Item& item = it->second;
  if (accessLevel < item.permission) {
    return;
  }

  item.value  = value;
  item.isSet  = true;
  item.isTrue =
    (value != "0")  &&
    (value != "no") && (value != "false") &&
    (value != "No") && (value != "False") &&
    (value != "NO") && (value != "FALSE") && (value != "disable");

  if (saveDefault) {
    item.defValue = value;
  }

  notify(it);
}


void StateDatabase::setInt(const std::string& name, const int& value,
			   Permission accessLevel)
{
  set(name, TextUtils::format("%d", value), accessLevel);
}


void StateDatabase::setBool(const std::string& name, const bool& value,
			    Permission accessLevel)
{
  set(name, value ? std::string("1") : std::string("0"), accessLevel);
}


void StateDatabase::setFloat(const std::string& name, const float& value,
			     Permission accessLevel)
{
  set(name, TextUtils::format("%f", value), accessLevel);
}


void StateDatabase::unset(const std::string& name, Permission accessLevel)
{
  ItemMap::iterator it = lookup(name);
  if (accessLevel >= it->second.permission) {
    it->second.value  = "";
    it->second.isSet  = false;
    it->second.isTrue = false;
    notify(it);
  }
}


void StateDatabase::touch(const std::string& name, Permission accessLevel)
{
  ItemMap::iterator it = lookup(name);
  if (accessLevel >= it->second.permission) {
    notify(it);
  }
}


void StateDatabase::setPersistent(const std::string& name, bool save)
{
  ItemMap::iterator it = lookup(name);
  it->second.save = save;
}


void StateDatabase::setDefault(const std::string& name,
                               const std::string& value)
{
  ItemMap::iterator it = lookup(name);
  it->second.defValue = value;
}


void StateDatabase::setPermission(const std::string& name,
                                  Permission permission)
{
  ItemMap::iterator it = lookup(name);
  it->second.permission = permission;
}


void StateDatabase::addCallback(const std::string& name,
				Callback callback, void* userData)
{
  ItemMap::iterator it = lookup(name);
  it->second.callbacks.add(callback, userData);
}


void StateDatabase::removeCallback(const std::string& name,
				   Callback callback, void* userData)
{
  ItemMap::iterator it = lookup(name);
  it->second.callbacks.remove(callback, userData);
}


void StateDatabase::addGlobalCallback(Callback callback, void* userData)
{
  globalCallbacks.add(callback, userData);
}


void StateDatabase::removeGlobalCallback(Callback callback, void* userData)
{
  globalCallbacks.remove(callback, userData);
}


bool StateDatabase::isSet(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  return !(it == items.end() || !it->second.isSet);
}


std::string StateDatabase::get(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  if (it == items.end() || !it->second.isSet) {
    return std::string();
  } else {
    return it->second.value;
  }
}


int StateDatabase::getIntClamped(const std::string& name,
                                 const int min, const int max) const
{
  int val;
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);

  if (it == items.end() || !it->second.isSet) {
    val = 0;
  } else {
    val = atoi(it->second.value.c_str());
  }

  if (val < min) {
    return min;
  } else if (val > max) {
    return max;
  }

  return val;
}


static float getNaN()
{
  // ugly hack, since gcc 2.95 doesn't have <limits>
  float NaN;
  memset(&NaN, 0xff, sizeof(float));
  return NaN;
}


void StateDatabase::addDependents(ItemMap::iterator& it,
                                  const StringSet& dependents)
{
  const std::string& name = it->first;
  Item& item = it->second;

  StringSet::const_iterator setIt;
  for (setIt = dependents.begin(); setIt != dependents.end(); ++setIt) {
    const std::string& depName = *setIt;
    DEBUG_LOOKUPS(depName);
    ItemMap::iterator itemIt = items.find(depName);
    if ((itemIt != items.end()) && (name != depName)) {
      if (debugLevel >= 6) {
        printf("BZDB: added '%s' to '%s' dependents set\n",
               depName.c_str(), name.c_str());
      }
      item.dependents.insert(itemIt->first);
      itemIt->second.dependencies.insert(name);
    }
  }
}


float StateDatabase::eval(const std::string& name)
{
  static StringSet evalStack;

  // avoid recursive evaluation
  if (evalStack.find(name) != evalStack.end()) {
    return getNaN();
  }

  EvalMap::const_iterator cit = evalCache.find(name);
  if (cit != evalCache.end()) {
    if (!evalStack.empty()) {
      // add this item to its dependents' sets
      ItemMap::iterator it = lookup(name);
      addDependents(it, evalStack);
    }
    return cit->second;
  }

  StringSet::iterator myStackIt = evalStack.insert(name).first;

  DEBUG_LOOKUPS(name);
  ItemMap::iterator it = items.find(name);
  if ((it == items.end()) ||
      !it->second.isSet || it->second.value.empty()) {
    evalStack.erase(myStackIt);
    return getNaN();
  }

  Expression infix;
  std::string input = it->second.value;
  input >> infix;

  Expression prefix = infixToPrefix(infix);
  const float value = evaluate(prefix);
  evalCache[name] = value;

  evalStack.erase(myStackIt);

  // add this item to its dependents' sets
  addDependents(it, evalStack);

  return value;
}


int StateDatabase::evalInt(const std::string& name)
{
  const float value = eval(name);
  return isnan(value) ? 0 : (int)value;
}


fvec2 StateDatabase::evalFVec2(const std::string& name)
{
  static const fvec2 nanValue(getNaN(), getNaN());
  if (!isSet(name)) {
    return nanValue;
  }
  float data[2];
  if (sscanf(get(name).c_str(), "%f %f",
             &data[0], &data[1]) != 2) {
    return nanValue;
  }
  if (isnan(data[0]) || isnan(data[1])) {
    return nanValue;
  }
  return fvec2(data[0], data[1]);
}


fvec3 StateDatabase::evalFVec3(const std::string& name)
{
  static const fvec3 nanValue(getNaN(), getNaN(), getNaN());
  if (!isSet(name)) {
    return nanValue;
  }
  float data[3];
  if (sscanf(get(name).c_str(), "%f %f %f",
             &data[0], &data[1], &data[2]) != 3) {
    return nanValue;
  }
  if (isnan(data[0]) || isnan(data[1]) || isnan(data[2])) {
    return nanValue;
  }
  return fvec3(data[0], data[1], data[2]);
}


fvec4 StateDatabase::evalFVec4(const std::string& name)
{
  static const fvec4 nanValue(getNaN(), getNaN(), getNaN(), getNaN());
  if (!isSet(name)) {
    return nanValue;
  }
  float data[4];
  if (sscanf(get(name).c_str(), "%f %f %f %f",
             &data[0], &data[1], &data[2], &data[3]) != 4) {
    return nanValue;
  }
  if (isnan(data[0]) || isnan(data[1]) || isnan(data[2]) || isnan(data[3])) {
    return nanValue;
  }
  return fvec4(data[0], data[1], data[2], data[3]);
}


bool StateDatabase::evalTriplet(const std::string& name, float data[4])
{
  if (!isSet(name) || !data) {
    return false;
  }
  if (sscanf(get(name).c_str(), "%f %f %f", data, data+1, data+2) != 3) {
    return false;
  }
  return true;
}


bool StateDatabase::evalPair(const std::string& name, float data[2])
{
  if (!isSet(name) || !data) {
    return false;
  }
  if (sscanf(get(name).c_str(), "%f %f", data, data+1) != 2) {
    return false;
  }
  return true;
}


bool StateDatabase::isTrue(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  return !(it == items.end() || !it->second.isTrue);
}


bool StateDatabase::isEmpty(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  return (it == items.end() || !it->second.isSet ||
	  it->second.value.empty());
}


bool StateDatabase::isPersistent(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  return (it != items.end() && it->second.save);
}


std::string StateDatabase::getDefault(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  if (it != items.end()) {
    return it->second.defValue;
  } else {
    return "";
  }
}


StateDatabase::Permission
  StateDatabase::getPermission(const std::string& name) const
{
  DEBUG_LOOKUPS(name);
  ItemMap::const_iterator it = items.find(name);
  if (it != items.end()) {
    return it->second.permission;
  } else {
    return ReadWrite;
  }
}


StateDatabase::ItemMap::iterator StateDatabase::lookup(const std::string& name)
{
  DEBUG_LOOKUPS(name);
  ItemMap::iterator it = items.find(name);
  if (it == items.end()) {
    Item tmp; // add a new item
    it = items.insert(std::make_pair(name, tmp)).first;
  }
  return it;
}


void StateDatabase::notify(ItemMap::iterator it)
{
  const std::string& name = it->first;
  const Item& item = it->second;

  if (debugLevel >= 6) {
    printf("BZDB: notify for '%s'\n", name.c_str());
  }

  // erase the cached value for the item
  evalCache.erase(name);

  // execute the callbacks
  void* namePtr = const_cast<void*>(static_cast<const void*>(&name));
  globalCallbacks.iterate(&onCallback, namePtr);
  item.callbacks.iterate(&onCallback, namePtr);

  // notify the dependents
  const StringSet dependents = item.dependents; // make a copy
  StringSet::const_iterator depIt;
  for (depIt = dependents.begin(); depIt != dependents.end(); ++depIt) {
    if (*depIt != name) {
      DEBUG_LOOKUPS(*depIt);
      ItemMap::iterator depItem = items.find(*depIt);
      if (depItem != items.end()) {
        const std::string& depName = *depIt;

        // erase the cached value for the item
        evalCache.erase(depName);

        // execute the callbacks
        void* namePtr2 = const_cast<void*>(static_cast<const void*>(&depName));
        globalCallbacks.iterate(&onCallback, namePtr2);
        item.callbacks.iterate(&onCallback, namePtr2);
      }
    }
  }
}


bool StateDatabase::onCallback(Callback callback,
                               void* userData, void* iterateData)
{
  callback(*static_cast<std::string*>(iterateData), userData);
  return true;
}


void StateDatabase::iterate(Callback callback, void* userData) const
{
  assert(callback != NULL);

  for (ItemMap::const_iterator it = items.begin(); it != items.end(); ++it) {
    if (it->second.isSet) {
      (*callback)(it->first, userData);
    }
  }
}


void StateDatabase::write(Callback callback, void* userData) const
{
  assert(callback != NULL);

  for (ItemMap::const_iterator it = items.begin(); it != items.end(); ++it) {
    if (it->second.isSet && it->second.save) {
      (*callback)(it->first, userData);
    }
  }
}


void StateDatabase::setDebug(bool print) {
  debug = print;
}


void StateDatabase::setSaveDefault(bool save) {
  saveDefault = save;
}


//============================================================================//
//
//  StateDatabase::ExpressionToken
//

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


std::string StateDatabase::ExpressionToken::getVariable() const
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
    case add:      { return 1; }
    case subtract: { return 1; }
    case multiply: { return 2; }
    case divide:   { return 2; }
    case power:    { return 3; }
    case lparen:   { return 4; }
    case rparen:   { return 0; }
    default:       { return 0; }
  }
}


//============================================================================//
//
//  static StateDatabase operators
//

std::istream&operator >> (std::istream& src, StateDatabase::ExpressionToken& dst)
{
  char temp;
  std::string tempname;

  src >> temp;

  if ((temp >= '0' && temp <= '9') || (temp == '.')) {
    // number
    tempname += temp;
    temp = src.peek();
    while ((temp >= '0' && temp <= '9') || (temp == '.')) {
      src >> temp;
      tempname += temp;
      temp = src.peek();
    }
    dst.setNumber(atof(tempname.c_str()));
  }
  else if ((temp == '+') || (temp == '-') || (temp == '*') || (temp == '/') ||
           (temp == '^') || (temp == '(') || (temp == ')')) {
    // operator
    switch(temp) {
      case '+': { dst.setOper(StateDatabase::ExpressionToken::add);      break; }
      case '-': { dst.setOper(StateDatabase::ExpressionToken::subtract); break; }
      case '*': { dst.setOper(StateDatabase::ExpressionToken::multiply); break; }
      case '/': { dst.setOper(StateDatabase::ExpressionToken::divide);   break; }
      case '^': { dst.setOper(StateDatabase::ExpressionToken::power);    break; }
      case '(': { dst.setOper(StateDatabase::ExpressionToken::lparen);   break; }
      case ')': { dst.setOper(StateDatabase::ExpressionToken::rparen);   break; }
    }
  }
  else if ((temp >= 'a' && temp <= 'z') ||
           (temp >= 'A' && temp <= 'Z') || (temp == '_')) {
    // variable (perhaps prefix with $?)
    tempname += temp;
    temp = src.peek();
    while ((temp >= 'a' && temp <= 'z') ||
           (temp >= 'A' && temp <= 'Z') || (temp == '_')) {
      src >> temp;
      tempname += temp;
      temp = src.peek();
    }
    dst.setVariable(tempname);
  }
  else {
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
  }
  else if ((temp == '+') || (temp == '-') || (temp == '*') || (temp == '/') ||
           (temp == '^') || (temp == '(') || (temp == ')')) {
    // operator
    switch (temp) {
      case '+': { dst.setOper(StateDatabase::ExpressionToken::add);      break; }
      case '-': { dst.setOper(StateDatabase::ExpressionToken::subtract); break; }
      case '*': { dst.setOper(StateDatabase::ExpressionToken::multiply); break; }
      case '/': { dst.setOper(StateDatabase::ExpressionToken::divide);   break; }
      case '^': { dst.setOper(StateDatabase::ExpressionToken::power);    break; }
      case '(': { dst.setOper(StateDatabase::ExpressionToken::lparen);   break; }
      case ')': { dst.setOper(StateDatabase::ExpressionToken::rparen);   break; }
    }
  }
  else if ((temp >= 'a' && temp <= 'z') ||
           (temp >= 'A' && temp <= 'Z') || (temp == '_')) {
    tempname += temp;
    temp = src[0];
    while (((temp >= 'a' && temp <= 'z') ||
            (temp >= 'A' && temp <= 'Z') || (temp == '_'))
           && (src.length() != 0)) {
      src = src.substr(1);
      tempname += temp;
      temp = src[0];
    }
    dst.setVariable(tempname);
  }
  else {
    // throw an error?
  }
  return src;
}


std::ostream& operator << (std::ostream& dst, const StateDatabase::ExpressionToken& src)
{
  switch (src.getTokenType()) {
    case StateDatabase::ExpressionToken::number: {
      dst << src.getNumber();
      break;
    }
    case StateDatabase::ExpressionToken::oper: {
      switch (src.getOperator()) {
	case StateDatabase::ExpressionToken::add:      { dst << '+'; break; }
	case StateDatabase::ExpressionToken::subtract: { dst << '-'; break; }
	case StateDatabase::ExpressionToken::multiply: { dst << '*'; break; }
	case StateDatabase::ExpressionToken::divide:   { dst << '/'; break; }
	case StateDatabase::ExpressionToken::power:    { dst << '^'; break; }
	case StateDatabase::ExpressionToken::lparen:   { dst << '('; break; }
	case StateDatabase::ExpressionToken::rparen:   { dst << ')'; break; }
	case StateDatabase::ExpressionToken::none:     { break; }
      }
      break;
    }
    case StateDatabase::ExpressionToken::variable: {
      dst << src.getVariable();
      break;
    }
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
    while (src.peek() == ' ' || src.peek() == '\t') {
      src >> tempc;
    }
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
    if ((i->getTokenType() == ExpressionToken::variable) ||
        (i->getTokenType() == ExpressionToken::number)) {
      postfix.push_back(*i);
    }
    else if (i->getTokenType() == ExpressionToken::oper) {
      if (i->getOperator() == ExpressionToken::lparen) {
	operators.push(*i);
      }
      else if (i->getOperator() == ExpressionToken::rparen) {
	// unstack operators until a matching (is found
	while ((operators.size() > 0) &&
	       (operators.top().getOperator() != ExpressionToken::lparen)) {
	  postfix.push_back(operators.top()); operators.pop();
	}
	// discard (
	if (operators.size() > 0) // handle extra-rparen case
	  operators.pop();
      }
      else {
	while ((operators.size() > 0) &&
	       (operators.top().getPrecedence() < i->getPrecedence()) &&
	       (operators.top().getOperator() != ExpressionToken::lparen)) {
	  postfix.push_back(operators.top()); operators.pop();
	}
	operators.push(*i);
      }
    }
  }

  while (operators.size() > 0) {
    postfix.push_back(operators.top()); operators.pop();
  }

  for (Expression::reverse_iterator ri = postfix.rbegin(); ri != postfix.rend(); ri++) {
    prefix.push_back(*ri);
  }

  return prefix;
}


float StateDatabase::evaluate(Expression e)
{
  std::stack<ExpressionToken> evaluationStack;
  ExpressionToken tok, lvalue, rvalue;
  bool unary;

  for (Expression::reverse_iterator i = e.rbegin(); i != e.rend(); i++) {
    unary = false;
    switch(i->getTokenType()) {
      case ExpressionToken::number: {
	evaluationStack.push(*i);
	break;
      }
      case ExpressionToken::variable: {
	// strip off '$'?
	tok.setNumber(eval(i->getVariable()));
	evaluationStack.push(tok);
	break;
      }
      case ExpressionToken::oper: {
	if ((i->getOperator() == ExpressionToken::lparen) ||
	    (i->getOperator() == ExpressionToken::rparen)) {
	  break;  // should not have any parens here, skip them
	}
	if (evaluationStack.size() == 0) {
	  // syntax error
	  return getNaN();
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
	  case ExpressionToken::add: {
	    tok.setNumber(lvalue.getNumber() + rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  }
	  case ExpressionToken::subtract: {
	    if (unary) {
	      tok.setNumber(-rvalue.getNumber());
	    } else {
	      tok.setNumber(lvalue.getNumber() - rvalue.getNumber());
            }
	    evaluationStack.push(tok);
	    break;
	  }
	  case ExpressionToken::multiply: {
	    tok.setNumber(lvalue.getNumber() * rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  }
	  case ExpressionToken::divide: {
	    tok.setNumber(lvalue.getNumber() / rvalue.getNumber());
	    evaluationStack.push(tok);
	    break;
	  }
	  case ExpressionToken::power: {
	    tok.setNumber(pow(lvalue.getNumber(), rvalue.getNumber()));
	    evaluationStack.push(tok);
	    break;
	  }
	  default: {
	    // lparen and rparen should have been stripped out
	    // throw something here, too
	    break;
	  }
	}
	break;
      }
    }
  }

  if (!evaluationStack.size()) {
    return 0; // yeah we are screwed. TODO, don't let us get this far
  }

  return (float)evaluationStack.top().getNumber();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
