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

#ifdef _WIN32
#pragma warning( 4: 4786)
#endif

#include "OptionParser.h"


OptionParser::OptionParser(const string& helpPrefix, 
			   const string& usageSuffix)
  : helpPre(helpPrefix), usageSuf(usageSuffix) {

}

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
    if (!strcmp(argv[i], "-help")) {
      printHelp(cout, argv[0]);
      return false;
    }
    if (argv[i][0] != '-')
      parameters.push_back(argv[i]);
    else {
      iter = parsers.find(&argv[i][1]);
      if (iter == parsers.end()) {
	error = error + "Unknown option \"" + argv[i] + "\"";
	break;
      }
      i += iter->second->parse(&argv[i+1]);
    }
  }
  if (error.size() > 0) {
    cerr<<error<<endl;
    printUsage(cerr, argv[0]);
    cerr<<endl;
    return false;
  }
  return true;
}


void OptionParser::printHelp(ostream& os, const string& progName) const {
  os<<helpPre<<endl<<endl;
  printUsage(os, progName);
  os<<endl<<endl;
  map<string, Parser*>::const_iterator iter;
  os<<"   -help: print this help message"<<endl;
  for (iter = parsers.begin(); iter != parsers.end(); ++iter)
    os<<"   -"<<iter->first<<": "<<iter->second->help<<endl;
}


void OptionParser::printUsage(ostream& os, const string& progName) const {
  map<string, Parser*>::const_iterator iter;
  os<<"Usage: "<<progName<<" [-help] ";
  for (iter = parsers.begin(); iter != parsers.end(); ++iter)
    os<<iter->second->usage<<" ";
  os<<usageSuf;
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
