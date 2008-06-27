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

#include <string>
#include <map>

#include "BZW/Parser.h"

namespace BZW
{
  Parser::Parser()
  {
    //TODO: Anything?
  }

  Parser::~Parser()
  {
    //TODO: deconstruct contents of map, multimap, and themselves
  }

  bool Parser::parse(std::istream& in)
  {
    if(!in->peek())
    {
      return false;
    }
    //TODO
    return true;
  }

  void Parser::manageObject(string name, Parser::Object& object);
  {
    if(managedObjects[name])
    {
      //TODO:Complain about duplicate object entry, keep newest entry
    }
    managedObjects[name] = object;
  }

  Parser::Parameter::Parameter(enum Parser::Parameter::ValueType type, int number_of_values bool repeatable)
  {
    this.type = type;
    if(number_of_values <= 0)
    {
      //TODO:Complain about impossible number of values, but don't worry about
      //it.
    }
    this.number_of_values = number_of_values;
    this.repeatable = repeatable;
  }

  Parser::Parameter::~Parameter()
  {
    //TODO
  }

  void Parser::Parameter::parse(std::istream& in)
  {
    //TODO
  }

  vector<union Parser::Parameter::ValueValue>* Parser::Parameter::getValues()
  {
    return &values;
  }

  Parser::Object::Object(bool repeatable)
  {
    this.repeatable = repeatable;
  }

  Parser::Object::~Object()
  {
    //TODO
  }

  void Parser::Object::manage(string name, Parser::Field &field)
  {
    if(map[name])
    {
      //TODO: Complain about duplicate object entry, keep newest entry
    }
    map[name] = field;
  }

  std::multimap<string, Field>* Parser::Object::getFields()
  {
    return &readFields;
  }


}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=87
