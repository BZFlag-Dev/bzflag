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

// a series of utilities for bzfs plugins to use when doing HTTP templates

#ifndef _PLUGIN_HTTPTEMPLATES_H_
#define _PLUGIN_HTTPTEMPLATES_H_

#include <string>
#include <map>
#include <vector>

// called to get the code for a template variable
typedef void (*TemplateKeyCallback) ( std::string &data, const std::string &key );

// called for logic statements, loops and if tests
// called repeatedly for each loop to know if it should be done.
// return true to do an instance of the loop
// return false to stop and continue the template
// for if test called to determine true or false
typedef bool (*TemplateTestCallback) ( const std::string &key );

class TemplateCallbackClass
{
public:
  virtual ~TemplateCallbackClass(){};
  virtual void keyCallback ( std::string &data, const std::string &key ){};
  virtual bool loopCallback ( const std::string &key ){return false;}
  virtual bool ifCallback ( const std::string &key ){return false;}
};

class Templateiser : public TemplateCallbackClass
{
public:
  Templateiser();
  virtual ~Templateiser();

  void addKey ( const char *key, TemplateKeyCallback callback );
  void addKey ( const char *key, TemplateCallbackClass *callback );
  void clearKey ( const char *key );
  void flushKeys ( void );

  void addLoop ( const char *loop, TemplateTestCallback callback );
  void addLoop ( const char *loop, TemplateCallbackClass *callback );
  void clearLoop ( const char *loop );
  void flushLoops ( void );

  void addIF ( const char *name, TemplateTestCallback callback );
  void addIF ( const char *name, TemplateCallbackClass *callback );
  void clearIF ( const char *name );
  void flushIFs ( void );

  void processTemplate ( std::string &code, const std::string &templateText );
  
  void setTemplateDir ( const std::string &dir );

  // for the default template tokens
  virtual void keyCallback ( std::string &data, const std::string &key );
  virtual bool loopCallback ( const std::string &key );
  virtual bool ifCallback ( const std::string &key );

protected:
  typedef std::map<std::string,TemplateKeyCallback> KeyMap;
  typedef std::map<std::string,TemplateTestCallback> TestMap;
  typedef std::map<std::string,TemplateCallbackClass*> ClassMap;

  KeyMap keyFuncCallbacks;
  TestMap loopFuncCallbacks;
  TestMap ifFuncCallbacks;
  ClassMap keyClassCallbacks;
  ClassMap loopClassCallbacks;
  ClassMap ifClassCallbacks;

  std::string templateDir; // for includes

  bool callKey ( std::string &data, const std::string &key );
  bool callLoop ( const std::string &key );
  bool callIF ( const std::string &key );

  void setDefaultTokens ( void );

private:
  std::string::const_iterator readKey ( std::string &key, std::string::const_iterator inItr, const std::string &str );
  std::string::const_iterator findNextTag ( const std::vector<std::string> &keys, std::string &endKey, std::string &code, std::string::const_iterator inItr, const std::string &str );

  void processComment ( std::string &code, std::string::const_iterator &inItr, const std::string &str );
  void replaceVar ( std::string &code, std::string::const_iterator &itr, const std::string &str );
  void processIF ( std::string &code, std::string::const_iterator &inItr, const std::string &str );
  void processLoop ( std::string &code, std::string::const_iterator &inItr, const std::string &str );

};

#endif //_PLUGIN_HTTPTEMPLATES_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
