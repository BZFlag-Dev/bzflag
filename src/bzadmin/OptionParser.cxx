/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "OptionParser.h"


OptionParser::~OptionParser() {
  std::map<string, Parser*>::iterator iter;
  for (iter = parsers.begin(); iter != parsers.end(); ++iter)
    delete iter->second;
}


const string& OptionParser::getError() const {
  return error;
}


const vector<string>& OptionParser::getParameters() const {
  return parameters;
}


bool OptionParser::parse(int argc, char** argv) {
  parameters.clear();
  error = "";
  map<string, Parser*>::iterator iter;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] != '-')
      parameters.push_back(argv[i]);
    else {
      iter = parsers.find(&argv[i][1]);
      if (iter == parsers.end()) {
	error = error + "Unknown option \"" + argv[i] + "\"";
	return false;
      }
      i += iter->second->parse(&argv[i+1]);
    }
  }
  return true;
}



