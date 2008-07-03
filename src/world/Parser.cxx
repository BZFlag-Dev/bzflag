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
    if(!in.peek())
    {
      return false;
    }
    //TODO
    return true;
  }

  void Parser::manageObject(std::string name, Parser::Object& object)
  {
    if(managedObjects.count(name))
    {
      //TODO:Complain about duplicate object entry, keep newest entry
    }
    //FIXME: See Parser.h
    managedObjects[name] = object;
  }

  //XXX/FIXME: variable names shadow member names. Is this acceptable?
  Parser::Parameter::Parameter(enum Parser::Parameter::ValueType type, int number_of_values, bool repeatable)
  {
    this->type = type;
    if(number_of_values <= 0)
    {
      //TODO:Complain about impossible number of values, but don't worry about
      //it.
    }
    this->number_of_values = number_of_values;
    this->repeatable = repeatable;
  }

  Parser::Parameter::~Parameter()
  {
    //TODO
  }

  void Parser::Parameter::parse(std::istream& in)
  {
    //TODO
  }

  std::vector<union Parser::Parameter::ValueValue>* Parser::Parameter::getValues()
  {
    return &values;
  }

  Parser::Object::Object(bool repeatable)
  {
    this->repeatable = repeatable;
  }

  Parser::Object::~Object()
  {
    //TODO
  }

  void Parser::Object::manage(std::string name, Parser::Field& field)
  {
    if(managedFields.count(name))
    {
      //TODO: Complain about duplicate object entry, keep newest entry
    }
    managedFields[name] = field;
  }

  std::multimap<std::string, Parser::Field>* Parser::Object::getFields()
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
