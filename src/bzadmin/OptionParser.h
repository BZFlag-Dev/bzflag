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

#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


/** This class handles all the command line parsing for bzadmin. */
class OptionParser {
public:
  /** Since the individual parsers are allocated dynamically we need a
      destructor that deletes them. */
  ~OptionParser();

  /** This function returns the latest error message. */
  const string& getError() const;
  /** This function returns a vector of unknown parameters in the parsed
      command line. */
  const vector<string>& getParameters() const;
  /** This function parses the given command line. */
  bool parse(int argc, char** argv);
  /** This function prints the help text to the stream @c out. */
  void printHelp(ostream& os) const;
  /** This function prints the usage text to the stream @c out. */
  void printUsage(ostream& os, const string& progName) const;
  /** This template function connects the variable @c variable to the command
      line option @c option. This means that if you call this function like
      this: <code>registerVariable("name", myName)</code>, and then gives
      the parameters <code>-name "Lars Luthman"</code> on the command line,
      the variable @c myName will get the value <code>"Lars Luthman"</code>
      when the command line is parsed. */
  template <class T>
  bool registerVariable(const string& option, T& variable, 
			const string& usage = "", const string& help = "");

protected:

  /** This is an abstract base class for all different option types. */
  class Parser {
  public:
    Parser(const string& usageText, const string& helpText)
      : usage(usageText), help(helpText) { }
    virtual ~Parser() { }
    /** This function is called when the option that this parser is mapped
	to is given on the command line. */
    virtual int parse(char** argv) = 0;
    const string usage;
    const string help;
  };

  /** This is a template class for the variable parser. */
  template <class T>
  class VariableParser : public Parser {
  public:
    VariableParser(T& variable, const string& usageText, 
		   const string& helpText) 
      : Parser(usageText, helpText), var(variable) { }
    virtual int parse(char** argv) {
      istringstream iss(argv[0]);
      iss>>var;
      return 1;
    }
  protected:
    T& var;
  };

  map<string, Parser*> parsers;
  vector<string> parameters;
  string error;
};


// implement the template functions here
template <class T>
bool OptionParser::registerVariable(const string& option, T& variable,
				    const string& usage, const string& help) {
  VariableParser<T>* parser = new VariableParser<T>(variable, usage, help);
  parsers[option] = parser;
  return true;
}


template<>
int OptionParser::VariableParser<string>::parse(char** argv) {
  var = argv[0];
  return 1;
}


template<>
int OptionParser::VariableParser<bool>::parse(char**) {
  var = true;
  return 0;
}


#endif
