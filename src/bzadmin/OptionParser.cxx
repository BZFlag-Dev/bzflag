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



