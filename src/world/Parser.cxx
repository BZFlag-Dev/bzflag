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
  namespace Parser
  {
    /* Key methods */
    void Key::addParameter(ParameterType type, bool optional = false)
    {

    }

    Key::Key(std::string name, bool optional)
    {

    }

    Key::~Key()
    {

    }

    /* Object methods */
    /// Constructor
    Object::ObjectType(std::string name)
    {

    }

    /// Destructor
    Object::~ObjectType()
    {

    }

    /* Parser methods */
    /// Constructor
    Parser::Parser()
    {

    }

    /// Destructor
    Parser::~Parser()
    {

    }

    /// Adds an Object definition to the Parser
    ObjectType * Parser::addObjectType(std::string name)
    {

    }

    /**
    * Using previously provided definitions, parses istream using
    * definitions. Use
    */
    bool Parser::Parse(std::istream &in)
    {

    }

  }
}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=87
