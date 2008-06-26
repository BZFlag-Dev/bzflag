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
  /**
   * BZW file Parsing class. Used primarily by BZW::World.
   */
  class Parser
  {
    public:
      class Parser::Field;
      class Parser::Object;
      class Parser::Parameter;

      /// Constructor
      Parser();
      /// Destructor
      ~Parser();

      /**
       * Using previously provided definitions, parses istream using
       * definitions. Use
       */
      bool Parse(std::istream &in);

      /**
       * Handle a new object type to parse. Adds a new block-like object
       * structure.
       */
      void manageObject(string name, Object &object);

      /**
       * Retrieve the multimap of objects read via parsing.
       * FIXME: This probably shouldn't be sending back a copy of the
       * multimap, and sending a reference seems like a bad idea also.
       */
      std::multimap<string, Object> getObjects();

    private:
      std::map<string, Object> managedObjects;
      std::multimap<string, Object> readObjects;
  };


  /**
   * BZW file parsing interface for Objects, Parameters.
   */
  class Parser::Field
  {
    public:
      /// Constructor
      Field(bool repeatable);
      /// Destructor
      ~Field();

      virtual void manage(Field &field);

    private:
      bool repeatable;
  };

  /**
   * BZW file parsing class for handling Parameters. Used primarily within
   * BZW::World and Parser.
   */
  class Parser::Parameter : public Parser::Field
  {
    public:
      /// Constructor
      Parameter(bool repeatable);
      /// Destructor
      ~Parameter();

    private:
      //TODO
  };

  /**
   * BZW file Parsing Object. Used primarily by BZW::World and within Parser.
   */
  class Parser::Object : public Parser::Field
  {
    public:
      /// Constructor
      Object(bool repeatable);
      /// Destructor
      ~Object();

    private:
      /** This is the text that comes after the identifier. Used
       * primarily by define/group/matrefs I believe...
       */
      string name;
      std::map<string, Field> managedFields;
      std::multimap<string, Field> readFields;
  };

}

#endif // __BZW_PARSER_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

