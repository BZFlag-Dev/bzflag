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

#ifndef __BZW_GRAMMAR_H__
#define __BZW_GRAMMAR_H__

#include <string>
#include <map>
#include <vector>
#include <iostream>

/* boost headers */
#include <boost/spirit/core.hpp>

/* BZW */
#include "Parser.h"

using namespace boost;
using namespace boost::spirit;

namespace BZW
{
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
        /*TODO add group def grammar */
        rule<ScannerT> parameter;
        rule<ScannerT> parameter_list;
        rule<ScannerT> block;
        rule<ScannerT> block_end;
        rule<ScannerT> block_list;

        definition(bzw_grammar const& self)
        {
          block_end
            = str_p("end")
            ;

          identifier
            = lexeme_d
              [
                alpha_p >>  *(alnum_p | ch_p('_'))
              ]
            - block_end
            ;

          string_literal
            = lexeme_d
              [
                graph_p >> *( *blank_p >> (graph_p - ch_p('#') ) )
              ]
            ;

          parameter
            = !(string_literal/*[functor]*/)
            >> line_end
            ;

          parameter_list
            = *parameter
            >> block_end
            >> line_end
            ;

          block
            = identifier/*[functor]*/
            >>
              !(
                  string_literal/*[functor]*/
              )
            >> line_end
            >> parameter_list
            ;

          block_list
            = *( line_end | block );
        }

        rule<ScannerT> const& start() { return block_list; }
      };
    Parser& parser;
  };

  struct bzw_skip_grammar : grammar<bzw_skip_grammar>
  {
    template <typename ScannerT>
      struct definition
      {
        rule<ScannerT> skip;

        definition(bzw_skip_grammar const& /*self*/)
        {
          skip
            = blank_p
            | "#" >> *(anychar_p - eol_p)
            ;
        }

        rule<ScannerT> const& start() const { return skip; }
      };
  };
}

#endif // __BZW_GRAMMAR_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
