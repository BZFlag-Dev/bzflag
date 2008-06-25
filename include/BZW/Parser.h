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
    class Parser
    {
      public:
        Parser();
        ~Parser();

        Parameter * addParameter(string name);
        void Parse();

      private:

    }

    class Parameter
    {
      friend Parameter * Parser::addParameter(string name);
      public:
        enum ValueType
        {
          String,
          Real
        };
        void addValue(ValueType type, bool required);

      protected:
        Parameter(string name);
        ~Parameter();

      private:

        union ValueValue
        {
          string string_value;
          int int_value;
          float real_value;
          bool bool_value;
        };

        struct Value
        {
          ValueType type;
          bool required;
          bool set;
          ValueValue value; // I'm really sorry for this line :(
        };

      string name;
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

