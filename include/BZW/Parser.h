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

#ifndef __BZW_PARSER_H__
#define __BZW_PARSER_H__

#include <string>
#include <iostream>

/* bzflag common headers */
#include "common.h"

namespace BZW
{
  namespace Parser
  {
    /**
     * BZW file Parsing Key class. Used for interaction between BZW::World,
     * Object and the Parser class to define what sorts of objects need to,
     * and can be, read from a BZW file.
     */
    class Key
    {
      friend Key * Parser::addKey(std::string name);
      public:
        enum ParameterType
        {
          String,
          Integer,
          Real,
          Boolean
        };
        void addParameter(ParameterType type, bool optional = false);

      protected:
        Key(std::string name, bool optional);
        ~Key();

      private:

        union ParameterValue
        {
          std::string string_value;
          int int_value;
          float real_value;
          bool bool_value;
        };

        struct Parameter
        {
          ValueType type;
          bool optional;
          bool set;
          ParameterValue value;
        };

        std::string name;
        bool optional;
    }

    /**
     * BZW file ObjectType class. Used primarily by BZW::World and Parser.
     */
    class ObjectType
    {
      friend ObjectType * Parser::addObjectType(std::string name);
      public:

      protected:
        /// Constructor
        ObjectType(std::string name);
        /// Destructor
        ~ObjectType();

      private:
        std::string name;
    }

    /**
     * BZW file Parsing class. Used primarily by BZW::World.
     */
    class Parser
    {
      public:
        /// Constructor
        Parser();
        /// Destructor
        ~Parser();

        /// Adds an Object definition to the Parser
        ObjectType * addObjectType(std::string name);
        /**
         * Using previously provided definitions, parses istream using
         * definitions. Use
         */
        bool Parse(std::istream &in);

      private:
    }


  }
}

#endif // __BZW_PARSER_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

