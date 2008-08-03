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
#include <map>
#include <vector>
#include <iostream>

/* boost headers */
#include <boost/spirit/core.hpp>
#include <boost/spirit/iterator/multi_pass.hpp>

namespace BZW
{
  class Parser
  {
    public:

    private:
      /*******************
       * Spirit Grammars *
       *******************/
      struct bzw_grammar : boost::spirit::grammar<bzw_grammar>
      {
        bzw_grammar(Parser& _parser)
          : parser(_parser) {}

        template <typename ScannerT>
          struct definition
          {
            rule<ScannerT> identifier;
            rule<ScannerT> string_literal;
            rule<ScannerT> line_end;
            rule<ScannerT> block;
            rule<ScannerT> block_end;
            rule<ScannerT> block_list;

            definition(bzw_grammar const& self)
            {
            }
          };
      };
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

