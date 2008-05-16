// statTemplates.cpp : Defines the entry point for the DLL application.
//

#include "reportTemplates.h"
#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <algorithm>
#include <map>


// actual template system

std::map<std::string,TemplateCallback> keyCallbacks;
std::map<std::string,TemplateTestCallback> loopCallbacks;
std::map<std::string,TemplateTestCallback> ifCallbacks;
std::string templateDir; // for includes

void addTemplateCall ( const char *key, TemplateCallback callback )
{
  if (!key || !callback)
    return;

  keyCallbacks[tolower(key)] = callback;
}

void addTemplateLoop ( const char *loop, TemplateTestCallback callback )
{
  if (!loop || !callback)
    return;

  loopCallbacks[tolower(loop)] = callback;
}

void addTemplateIF ( const char *loop, TemplateTestCallback callback )
{
  if (!loop || !callback)
    return;

  ifCallbacks[tolower(loop)] = callback;
}

void clearTemplateCall ( const char *key )
{
  std::map<std::string,TemplateCallback>::iterator itr = keyCallbacks.find(tolower(key));
  if (itr != keyCallbacks.end())
    keyCallbacks.erase(itr);
}

void clearTemplateLoop ( const char *loop )
{
  std::map<std::string,TemplateTestCallback>::iterator itr = loopCallbacks.find(tolower(loop));
  if (itr != loopCallbacks.end())
    loopCallbacks.erase(itr);
}

void clearTemplateIF ( const char *name )
{
  std::map<std::string,TemplateTestCallback>::iterator itr = ifCallbacks.find(tolower(name));
  if (itr != ifCallbacks.end())
    ifCallbacks.erase(itr);
}

void flushTemplateCalls ( void )
{
  keyCallbacks.clear();
}

void flushTemplateLoops ( void )
{
  keyCallbacks.clear();
}

void flushTemplateIFs ( void )
{
  ifCallbacks.clear();
}

std::string::const_iterator readKey ( std::string &key, std::string::const_iterator &inItr, const std::string &str )
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

void replaceVar ( std::string &code, std::string::const_iterator &itr, const std::string &str )
{
  // find the end of the ]]
  std::string key;

  itr = readKey(key,itr,str);

  if (itr != str.end())
  {
    std::string lowerKey = tolower(key);
    std::map<std::string,TemplateCallback>::iterator cbItr = keyCallbacks.find(lowerKey);
    if (cbItr != keyCallbacks.end())
    {
      if (cbItr->second)
      {
	std::string val;
	(*cbItr->second)(val,lowerKey);
	code += val;
      }
    }
    else // it's not found so dump it
      code += "[$"+key+"]";
  }
}

std::string::const_iterator findNextTag ( const std::vector<std::string> &keys, std::string &endKey, std::string &code, std::string::const_iterator &inItr, const std::string &str )
{
  if (!keys.size())
    return inItr;

  std::string::const_iterator itr = str.end();

  while (1)
  {
    itr = std::find(inItr,str.end(),'[');
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
	endKey == key;
	code.resize(keyStartItr - inItr);
	std::copy(inItr,keyStartItr,code.begin());
	return itr;
      }
    }
  }
  return itr;
}

void processLoop ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
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
  checkKeys.push_back(format("*end %s",commandParts[1]));

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
  checkKeys.push_back(format("*empty %s",commandParts[1]));
  itr = findNextTag(checkKeys,keyFound,emptySection,itr,str);

  if (itr == str.end())
  {
    inItr = itr;
    return;
  }

  //now find out if the loop is empty or not
  std::map<std::string,TemplateTestCallback>::iterator loopItr = loopCallbacks.find(tolower(commandParts[1]));
  if (loopItr == loopCallbacks.end())  // no callback means empty loop
  {
    std::string newCode;
    processTemplate(newCode,emptySection);
    code += newCode;
  }
  else
  {
    if ((*loopItr->second)(commandParts[1]) )
    {
      std::string newCode;
      processTemplate(newCode,loopSection);
      code += newCode;

      while((*loopItr->second)(commandParts[1]))
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
  }

  inItr = itr;
}

void processIF ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
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

  // now get the code for the loop section section
  std::string trueSection,elseSection;

  std::vector<std::string> checkKeys;
  checkKeys.push_back(format("*else %s",commandParts[1]));
  checkKeys.push_back(format("*end %s",commandParts[1]));

  std::string keyFound;
  itr = findNextTag(checkKeys,keyFound,trueSection,itr,str);

  if (itr == str.end())
  {
    inItr = itr;
    return;
  }

  if (keyFound == checkKeys[0]) // we hit an else, so we need to check for it
  {
    // it was the else, so go and find the end too
    
    checkKeys.erase(checkKeys.begin());// kill the else, find the end
    itr = findNextTag(checkKeys,keyFound,elseSection,itr,str);

    if (itr == str.end())
    {
      inItr = itr;
      return;
    }
  }

  //now find out if the loop is empty or not
  std::map<std::string,TemplateTestCallback>::iterator ifItr = ifCallbacks.find(tolower(commandParts[1]));
  if (ifItr == ifCallbacks.end())  // no callback means do the false case loop
  {
    if (elseSection.size())
    {
      std::string newCode;
      processTemplate(newCode,elseSection);
      code += newCode;
    }
  }
  else
  {
    if ((*ifItr->second)(commandParts[1]) )
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
  }

  inItr = itr;
}

void processComment ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
{
  std::string key;
  inItr = readKey(key,inItr,str);
}

void processTemplate ( std::string &code, const std::string &templateText )
{
  // start dumping the code
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
	    replaceVar(code,templateItr,templateText);
	    break;

	  case '*':
	    processLoop(code,templateItr,templateText);
	    break;

	  case '?':
	    processIF(code,templateItr,templateText);
	    break;
	  case '-':
	    processComment(code,templateItr,templateText);
	    break;
	}
      }
    }
  }
}

void setTemplateDir ( const std::string &dir )
{
  templateDir = dir;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
