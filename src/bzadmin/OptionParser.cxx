/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

/* interface header */
#include "OptionParser.h"


OptionParser::OptionParser(const std::string& helpPrefix,
			   const std::string& usageSuffix)
  : helpPre(helpPrefix), usageSuf(usageSuffix) {

}

OptionParser::~OptionParser() {
  ParserMap::iterator iter;
  for (iter = parsers.begin(); iter != parsers.end(); ++iter)
    delete iter->second;
}


const std::string& OptionParser::getError() const {
  return error;
}


const std::vector<std::string>& OptionParser::getParameters() const {
  return parameters;
}


bool OptionParser::parse(int argc, char** argv) {
  parameters.clear();
  error = "";
  ParserMap::iterator iter;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-help")) {
      printHelp(std::cout, argv[0]);
      return false;
    }
    if (argv[i][0] != '-') {
      parameters.push_back(argv[i]);
      // should just blank out the password, but we don't really parse it here
      memset(argv[i], ' ', strlen(argv[i]));
    } else {
      iter = parsers.find(&argv[i][1]);
      if (iter == parsers.end()) {
	error = error + "Unknown option \"" + argv[i] + "\"";
	break;
      }
      i += iter->second->parse(&argv[i+1]);
    }
  }
  if (error.size() > 0) {
    std::cerr<<error<<std::endl;
    printUsage(std::cerr, argv[0]);
    std::cerr<<std::endl;
    return false;
  }
  return true;
}


void OptionParser::printHelp(std::ostream& os, const std::string& progName) const {
  os<<helpPre<<std::endl<<std::endl;
  printUsage(os, progName);
  os<<std::endl<<std::endl;
  ParserMap::const_iterator iter;
  os<<"   -help: print this help message"<<std::endl;
  for (iter = parsers.begin(); iter != parsers.end(); ++iter)
    os<<"   -"<<iter->first<<": "<<iter->second->help<<std::endl;
}


void OptionParser::printUsage(std::ostream& os, const std::string& progName) const {
  ParserMap::const_iterator iter;
  os<<"Usage: "<<progName<<" [-help] ";
  for (iter = parsers.begin(); iter != parsers.end(); ++iter)
    os<<iter->second->usage<<" ";
  os<<usageSuf;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
