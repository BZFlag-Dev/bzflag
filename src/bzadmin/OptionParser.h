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


/** This class handles all the command line parsing for bzadmin. */
class OptionParser {
public:

  /** @c helpPrefix is a line (or several lines) of text that is written
      before the usage description in the help message. It could for example
      be the program name and version. @c usageSuffix is a line (no linebreaks)
      of text that will be added to the short usage description, and it
      could describe command parameters that aren't registered with this
      parser. */
  OptionParser(const std::string& helpPrefix, const std::string& usageSuffix);

  /** Since the individual parsers are allocated dynamically we need a
      destructor that deletes them. */
  ~OptionParser();

  /** This function returns the latest error message. */
  const std::string& getError() const;
  /** This function returns a vector of unknown parameters in the parsed
      command line. */
  const std::vector<std::string>& getParameters() const;
  /** This function parses the given command line. */
  bool parse(int argc, char** argv);
  /** This function prints the help text to the stream @c out. */
  void printHelp(std::ostream& os, const std::string& progname) const;
  /** This function prints the usage text to the stream @c out. */
  void printUsage(std::ostream& os, const std::string& progName) const;
  /** This template function connects the variable @c variable to the command
      line option @c option. This means that if you call this function like
      this: <code>registerVariable("name", myName)</code>, and then gives
      the parameters <code>-name "Lars Luthman"</code> on the command line,
      the variable @c myName will get the value <code>"Lars Luthman"</code>
      when the command line is parsed. */
  template <class T>
  bool registerVariable(const std::string& option, T& variable,
			const std::string& usage = "", const std::string& help = "")
  { //VC doesn't support out of class definition of template functions
    VariableParser<T>* parser = new VariableParser<T>(variable, usage, help);
    parsers[option] = parser;
    return true;
  }


protected:

  /** This is an abstract base class for all different option types. */
  class Parser {
  public:
    Parser(const std::string& usageText, const std::string& helpText)
      : usage(usageText), help(helpText) { }
    virtual ~Parser() { }
    /** This function is called when the option that this parser is mapped
	to is given on the command line. */
    virtual int parse(char** argv) = 0;
    const std::string usage;
    const std::string help;
  };

  /** This is a template class for the variable parser. */
  template <class T>
  class VariableParser : public Parser {
  public:
    VariableParser(T& variable, const std::string& usageText,
		   const std::string& helpText)
      : Parser(usageText, helpText), var(variable) { }
    virtual int parse(char** argv) {
      std::istringstream iss(argv[0]);
      iss>>var;
      return 1;
    }
  protected:
    T& var;
  };

  /** This is a specialization for @c string variables. It copies the
      entire parameter instead of just the first word (which the
      stream operator would have done). */
  class VariableParser<std::string> : public Parser {
  public:
    VariableParser(std::string& variable, const std::string& usageText,
		   const std::string& helpText)
      : Parser(usageText, helpText), var(variable) { }
    virtual int parse(char** argv) {
      var = argv[0];
      return 1;
    }
  protected:
    std::string& var;
  };

  /** This is a specialization for @c bool variables. It does not
      take a parameter, but just sets the variable to @c true. */
  class VariableParser<bool> : public Parser {
  public:
    VariableParser(bool& variable, const std::string& usageText,
		   const std::string& helpText)
      : Parser(usageText, helpText), var(variable) { }
    virtual int parse(char**) {
      var = true;
      return 0;
    }
  protected:
    bool& var;
  };

  std::map<std::string, Parser*> parsers;
  std::vector<std::string> parameters;
  std::string error;
  std::string helpPre;
  std::string usageSuf;
};


#endif

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
