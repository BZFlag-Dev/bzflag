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

/* Spirit */
#include <boost/spirit/iterator/multi_pass.hpp>

/* BZW */
#include "Parser.h"

namespace BZW
{
	void Parser::parse(std::istream& input)
  {
    istreambuf_iterator<char> stream_begin(input);
    multi_pass<istreambuf_iterator<char> > begin(make_multi_pass(stream_begin)), end;
    bzw_grammar the_grammar(*this);
    bzw_skip_grammar the_skipper;
    parse(begin, end, the_grammar, the_skipper);

	// So I don't know how this is done but this is the basic idea,
	// as you are parsing, your grammar will get you the type of the object
	while (parsing)
	{
	  std::string newObjectType = "put something real here from your grammar"; // get the new object type from the grammar 

	  current_object = world.newObject(newObjectType); //ask the world to make one for you

	  while (!shitDone)	// read untill the end of the object and pass it lines to parse
		current_object->readLine(some_line_shit);

	  // tell the object we are done with it
	  current_object->finalize();
	}

	current_object = NULL;
  }

  Parser::Parser( World &w) : current_object(NULL), : world(w)
  {

  }

  Parser::~Parser()
  {

  }


}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=87
