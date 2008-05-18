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

#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include <algorithm>

#include "plugin_HTTPTemplates.h"
#include "plugin_utils.h"
#include "bzfsAPI.h"

Templateiser::Templateiser()
{
  setDefaultTokens();
}

Templateiser::~Templateiser()
{

}

void Templateiser::addKey ( const char *key, TemplateKeyCallback callback )
{
  if (!key || !callback)
    return;

  keyFuncCallbacks[tolower(key)] = callback;
  ClassMap::iterator itr = keyClassCallbacks.find(tolower(key));
  if (itr != keyClassCallbacks.end())
    keyClassCallbacks.erase(itr);
}

void Templateiser::addKey ( const char *key, TemplateCallbackClass *callback )
{
  if (!key || !callback)
    return;

  keyClassCallbacks[tolower(key)] = callback;
  KeyMap::iterator itr = keyFuncCallbacks.find(tolower(key));
  if (itr != keyFuncCallbacks.end())
    keyFuncCallbacks.erase(itr);
}

void Templateiser::clearKey ( const char *key )
{
  ClassMap::iterator itr = keyClassCallbacks.find(tolower(key));
  if (itr != keyClassCallbacks.end())
    keyClassCallbacks.erase(itr);

  KeyMap::iterator itr2 = keyFuncCallbacks.find(tolower(key));
  if (itr2 != keyFuncCallbacks.end())
    keyFuncCallbacks.erase(itr2);
}

void Templateiser::flushKeys ( void )
{
  keyClassCallbacks.clear();
  keyFuncCallbacks.clear();
}

void Templateiser::addLoop ( const char *loop, TemplateTestCallback callback )
{
  if (!loop || !callback)
    return;

  loopFuncCallbacks[tolower(loop)] = callback;
  ClassMap::iterator itr = loopClassCallbacks.find(tolower(loop));
  if (itr != loopClassCallbacks.end())
    loopClassCallbacks.erase(itr);
}

void Templateiser::addLoop ( const char *loop, TemplateCallbackClass *callback )
{
  if (!loop || !callback)
    return;

  loopClassCallbacks[tolower(loop)] = callback;
  TestMap::iterator itr = loopFuncCallbacks.find(tolower(loop));
  if (itr != loopFuncCallbacks.end())
    loopFuncCallbacks.erase(itr);
}

void Templateiser::clearLoop ( const char *loop )
{
  TestMap::iterator itr = loopFuncCallbacks.find(tolower(loop));
  if (itr != loopFuncCallbacks.end())
    loopFuncCallbacks.erase(itr);

  ClassMap::iterator itr2 = loopClassCallbacks.find(tolower(loop));
  if (itr2 != loopClassCallbacks.end())
    loopClassCallbacks.erase(itr2);
}

void Templateiser::flushLoops ( void )
{
  loopClassCallbacks.clear();
  loopFuncCallbacks.clear();
}

void Templateiser::addIF ( const char *name, TemplateTestCallback callback )
{
  if (!name || !callback)
    return;

  ifFuncCallbacks[tolower(name)] = callback;
  ClassMap::iterator itr = ifClassCallbacks.find(tolower(name));
  if (itr != ifClassCallbacks.end())
    ifClassCallbacks.erase(itr);
}

void Templateiser::addIF ( const char *name, TemplateCallbackClass *callback )
{
  if (!name || !callback)
    return;

  ifClassCallbacks[tolower(name)] = callback;
  TestMap::iterator itr = ifFuncCallbacks.find(tolower(name));
  if (itr != ifFuncCallbacks.end())
    ifFuncCallbacks.erase(itr);
}

void Templateiser::clearIF ( const char *name )
{
  ClassMap::iterator itr = ifClassCallbacks.find(tolower(name));
  if (itr != ifClassCallbacks.end())
    ifClassCallbacks.erase(itr);

  TestMap::iterator itr2 = ifFuncCallbacks.find(tolower(name));
  if (itr2 != ifFuncCallbacks.end())
    ifFuncCallbacks.erase(itr2);
}

void Templateiser::flushIFs ( void )
{
  ifClassCallbacks.clear();
  ifFuncCallbacks.clear();
}

bool Templateiser::callKey ( std::string &data, const std::string &key )
{
  std::string lowerKey = tolower(key);

  data.clear();

  ClassMap::iterator itr = keyClassCallbacks.find(lowerKey);
  if (itr != keyClassCallbacks.end())
  {
    itr->second->keyCallback(data,key);
    return true;
  }

  KeyMap::iterator itr2 = keyFuncCallbacks.find(lowerKey);
  if (itr2 != keyFuncCallbacks.end())
  {
    (itr2->second)(data,key);
    return true;
  }

  return false;
}

bool Templateiser::callLoop ( const std::string &key )
{
  std::string lowerKey = tolower(key);

  ClassMap::iterator itr = loopClassCallbacks.find(lowerKey);
  if (itr != loopClassCallbacks.end())
    return itr->second->loopCallback(key);

  TestMap::iterator itr2 = loopFuncCallbacks.find(lowerKey);
  if (itr2 != loopFuncCallbacks.end())
    return (itr2->second)(key);

  return false;
}

bool Templateiser::callIF ( const std::string &key )
{
  std::string lowerKey = tolower(key);

  ClassMap::iterator itr = ifClassCallbacks.find(lowerKey);
  if (itr != ifClassCallbacks.end())
    return itr->second->ifCallback(key);

  TestMap::iterator itr2 = ifFuncCallbacks.find(lowerKey);
  if (itr2 != ifFuncCallbacks.end())
    return (itr2->second)(key);

  return false;
}

void Templateiser::processTemplate ( std::string &code, const std::string &templateText )
{
  std::string::const_iterator templateItr = templateText.begin();

  while ( templateItr != templateText.end() )
  {
    if ( *templateItr != '[' )
    {
      code += *templateItr;
      templateItr++;
    }
    else
    {
      templateItr++;

      if (templateItr == templateText.end())
	code += '[';
      else
      {
	switch(*templateItr)
	{
	default: // it's not a code, so just let the next loop hit it and output it
	  break;

	case '$':
	  replaceVar(code,++templateItr,templateText);
	  break;

	case '*':
	  processLoop(code,++templateItr,templateText);
	  break;

	case '?':
	  processIF(code,++templateItr,templateText);
	  break;
	case '-':
	  processComment(code,++templateItr,templateText);
	  break;
	}
      }
    }
  }
}

void Templateiser::setTemplateDir ( const std::string &dir )
{
  templateDir = dir;
}

void Templateiser::setDefaultTokens ( void )
{
  addKey("Date",this);
  addKey("Time",this);
  addKey("HostName",this);
  addIF("Public",this);
}

void Templateiser::keyCallback ( std::string &data, const std::string &key )
{
  if (key == "date")
  {
    bz_localTime time;
    bz_getLocaltime(&time);
    data = format("%d/%d/%d %d:%d:%d",time.month,time.day,time.year);
  }
  else if (key == "time")
  {
    bz_localTime time;
    bz_getLocaltime(&time);
    data = format("d:%d:%d",time.hour,time.minute,time.second);
  }
  else if (key == "hostname")
  {
    data = bz_getPublicAddr().c_str();
    if (!data.size())
      data = format("localhost:%d",bz_getPublicPort());;
  }
}

bool Templateiser::loopCallback ( const std::string &key )
{
  return false;
}

bool Templateiser::ifCallback ( const std::string &key )
{
  if (key == "public")
  {
    return bz_getPublic();
  }
  return false;
}

// processing helpers

std::string::const_iterator Templateiser::readKey ( std::string &key, std::string::const_iterator inItr, const std::string &str )
{
  std::string::const_iterator itr = inItr;

  while ( itr != str.end() )
  {
    if (*itr != ']')
    {
      key += *itr;
      itr++;
    }
    else
    {
      // go past the code
      itr++;
      key = tolower(key);
      return itr;
    }
  }
  return itr;
}

std::string::const_iterator Templateiser::findNextTag ( const std::vector<std::string> &keys, std::string &endKey, std::string &code, std::string::const_iterator inItr, const std::string &str )
{
  if (!keys.size())
    return inItr;

  std::string::const_iterator itr = inItr;

  while (1)
  {
    itr = std::find(itr,str.end(),'[');
    if (itr == str.end())
      return itr;

    // save off the itr in case this is the one, so we can copy to this point
    std::string::const_iterator keyStartItr = itr;

    itr++;
    if (itr == str.end())
      return itr;

    std::string key;
    itr = readKey(key,itr,str);

    for ( size_t i = 0; i < keys.size(); i++ )
    {
      if ( key == keys[i])
      {
	endKey = key;
	code.resize(keyStartItr - inItr);
	std::copy(inItr,keyStartItr,code.begin());
	return itr;
      }
    }
  }
  return itr;
}

void Templateiser::processComment ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
{
  std::string key;
  inItr = readKey(key,inItr,str);
}

void Templateiser::replaceVar ( std::string &code, std::string::const_iterator &itr, const std::string &str )
{
  // find the end of the ]]
  std::string key;

  itr = readKey(key,itr,str);

  if (itr != str.end())
  {
    std::string lowerKey = tolower(key);
    std::string val;
    if (callKey(val,lowerKey))
      code += val;
    else
      code += "[$"+key+"]";
  }
}

void Templateiser::processLoop ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
{
  std::string key;

  // read the rest of the key
  std::string::const_iterator &itr = readKey(key,inItr,str);

  std::vector<std::string> commandParts = tokenize(key,std::string(" "),0,0);
  if (commandParts.size() < 2)
  {
    inItr = itr;
    return;
  }

  // check the params
  commandParts[0] = tolower(commandParts[0]);
  commandParts[1] = tolower(commandParts[1]);

  if ( commandParts[0] != "start" )
  {
    inItr = itr;
    return;
  }

  // now get the code for the loop section section
  std::string loopSection,emptySection;

  std::vector<std::string> checkKeys;
  checkKeys.push_back(format("*end %s",commandParts[1].c_str()));

  std::string keyFound;
  itr = findNextTag(checkKeys,keyFound,loopSection,itr,str);

  if (itr == str.end())
  {
    inItr = itr;
    return;
  }

  // do the empty section
  // loops have to have both
  checkKeys.clear();
  checkKeys.push_back(format("*empty %s",commandParts[1].c_str()));
  itr = findNextTag(checkKeys,keyFound,emptySection,itr,str);

  if (callLoop(commandParts[1]))
  {
    std::string newCode;
    processTemplate(newCode,loopSection);
    code += newCode;

    while(callLoop(commandParts[1]))
    {
	newCode = "";
	processTemplate(newCode,loopSection);
	code += newCode;
    }
  }
  else
  {
    std::string newCode;
    processTemplate(newCode,emptySection);
    code += newCode;
  }
  inItr = itr;
}

void Templateiser::processIF ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
{
  std::string key;

  // read the rest of the key
  std::string::const_iterator &itr = readKey(key,inItr,str);

  std::vector<std::string> commandParts = tokenize(key,std::string(" "),0,0);
  if (commandParts.size() < 2)
  {
    inItr = itr;
    return;
  }

  // check the params
  commandParts[0] = tolower(commandParts[0]);
  commandParts[1] = tolower(commandParts[1]);

  if ( commandParts[0] != "if" )
  {
    inItr = itr;
    return;
  }

  // now get the code for the next section
  std::string trueSection,elseSection;

  std::vector<std::string> checkKeys;
  checkKeys.push_back(format("?else %s",commandParts[1].c_str()));
  checkKeys.push_back(format("?end %s",commandParts[1].c_str()));

  std::string keyFound;
  itr = findNextTag(checkKeys,keyFound,trueSection,itr,str);

  if (keyFound == checkKeys[0]) // we hit an else, so we need to check for it
  {
    // it was the else, so go and find the end too
    if (itr == str.end())
    {
      inItr = itr;
      return;
    }

    checkKeys.erase(checkKeys.begin());// kill the else, find the end
    itr = findNextTag(checkKeys,keyFound,elseSection,itr,str);
  }

  // test the if, stuff that dosn't exist is false
  if (callIF(commandParts[1]))
  {
    std::string newCode;
    processTemplate(newCode,trueSection);
    code += newCode;
  }
  else
  {
    std::string newCode;
    processTemplate(newCode,elseSection);
    code += newCode;
  }
  inItr = itr;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
