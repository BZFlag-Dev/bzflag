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

#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include "common.h"

/* system interface headers */
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/* common interface headers */
#include "TextUtils.h"

/** This is an abstract base class for all different option parsers.
    The idea is that you register a Parser object for each command line option,
    and the subclass of the object depends on what the program should
    do when it sees that option. If it should change a variable you could
    use a VariableParser. Right now VariableParser is the only subclass.
    This class and its subclasses should only be used by OptionParser.
    @see VariableParser
    @see OptionParser
*/
class Parser {
public:
  /** This constructor should be called by all subclasses.
      @param usageText A text string that describes how this command line
		       option should be used, e.g. "[-ui curses|stdout]".
      @param helpText A short description of what this option does.
  */
  Parser(const std::string& usageText, const std::string& helpText)
    : usage(usageText), help(helpText) { }
  virtual ~Parser() { }
  /** This function is called by OptionParser when the option that this
      parser is mapped to is given on the command line. It will return
      the number of parameters that this option takes, so that the
      OptionParser knows where to look for next option.
  */
  virtual int parse(char** argv) = 0;
  const std::string usage;
  const std::string help;
};

/** This is a template class for the variable parser. It should be used
    when you want a command line option to change the value of a variable.
    The template parameter T must be such that there is a function
    <code>istream& operator>>(istream&, T&)</code>.
*/
template <class T>
class VariableParser : public Parser {
public:
  /** This constructor stores a reference to @c variable, and when parse() is
      called it will read the value of the next parameter on the command line
      into @c variable. This should only be used by OptionParser, see
      OptionParser::registerVariable() for more info.
  */
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

// true ugliness to fool VC5
typedef std::string STRING;

/** This is a specialization for @c string variables. It copies the
    entire parameter instead of just the first word (which the
    stream operator would have done). */
template<>
class VariableParser<STRING> : public Parser {
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
template<>
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

/** This is a parser for @c std::vector<T> variables.
    It splits the parameter at ',' characters and puts the tokens in the
    vector. The type @c T must have a stream operator. */
template<class T>
class VectorParser : public Parser {
public:
  VectorParser(std::vector<T>& variable,
		 const std::string& usageText,
		 const std::string& helpText)
    : Parser(usageText, helpText), var(variable) { }
  virtual int parse(char** argv) {
    std::vector<std::string> tmpVector = TextUtils::tokenize(argv[0], ",");
    T t;
    for (unsigned i = 0; i < tmpVector.size(); ++i) {
      std::istringstream iss(tmpVector[i]);
      iss>>t;
      var.push_back(t);
    }
    return 1;
  }
protected:
  std::vector<T>& var;
};


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
      line option @c option. This means that if you have a @c std::string
      variable called @c myName and call this function like
      this: <code>registerVariable("name", myName)</code>, and then give
      the parameters <code>-name "Lars Luthman"</code> on the command line,
      the variable @c myName will get the value <code>"Lars Luthman"</code>
      when the command line is parsed. This is a template function, so it
      will work for variables of other types too, such as @c float or
      @c int. The only condition on the variable type is that it can be
      read from an @c istream.
  */
  template <class T>
  bool registerVariable(const std::string& option, T& variable,
			const std::string& usage = "",
			const std::string& help = "")
  { //VC doesn't support out of class definition of template functions
    VariableParser<T>* parser = new VariableParser<T>(variable, usage, help);
    parsers[option] = parser;
    return true;
  }
  /** Same as registerVariable(), but for @c vector<T> variables instead.
      The parameter will be tokenized with ',' as delimiter and the tokens
      will be parsed and placed in the vector. */
  template <class T>
  bool registerVector(const std::string& option, std::vector<T>& variable,
		      const std::string& usage = "",
		      const std::string& help = "")
  { //VC doesn't support out of class definition of template functions
    VectorParser<T>* parser = new VectorParser<T>(variable, usage, help);
    parsers[option] = parser;
    return true;
  }


protected:
  typedef std::map<std::string, Parser*> ParserMap;

  ParserMap parsers;
  std::vector<std::string> parameters;
  std::string error;
  std::string helpPre;
  std::string usageSuf;
};

#endif


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
