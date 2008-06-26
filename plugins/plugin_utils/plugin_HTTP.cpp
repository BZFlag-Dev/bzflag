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

#include "plugin_HTTP.h"
#include "plugin_utils.h"
#include "plugin_groups.h"
#include "plugin_files.h"

#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include <algorithm>

BZFSHTTP::BZFSHTTP()
{
  bz_loadPlugin("HTTPServer",NULL);
}

void BZFSHTTP::registerVDir ( void )
{
  bz_callCallback("RegisterHTTPDVDir",this);
}

BZFSHTTP::~BZFSHTTP()
{
  bz_callCallback("RemoveHTTPDVDir",this);
}
std::string BZFSHTTP::getBaseURL ( void )
{
  std::string URL = "http://";
  std::string host = "localhost";
  if (bz_getPublicAddr().size())
    host = bz_getPublicAddr().c_str();

  // make sure it has the port
  if ( strrchr(host.c_str(),':') == NULL )
    host += format(":%d",bz_getPublicPort());

  URL += host +"/";
  URL += getVDir();
  URL += "/";

  return URL;
}

bool HTTPRequest::getParam ( const char* param, std::string &val ) const
{
  val = "";
  if (!param)
    return false;

  std::string p;
  tolower(param,p);

  std::map<std::string, std::string>::const_iterator itr = paramaters.find(p);
  if ( itr != paramaters.end() )
  {
    val = itr->second;
    return true;
  } 
  return false;
}

bool HTTPRequest::getParam ( const std::string &param, std::string &val ) const
{
  val = "";

  std::string p;
  tolower(param,p);

  std::map<std::string, std::string>::const_iterator itr = paramaters.find(p);
  if ( itr != paramaters.end() )
  {
    val = itr->second;
    return true;
  } 
  return false;
}

class PendingTokenTask : public bz_BaseURLHandler
{
public:
  PendingTokenTask():requestID(-1){};

  virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
  virtual void timeout ( const char* /*URL*/, int /*errorCode*/ );
  virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ );

  int requestID;
  std::string URL;
  std::vector<std::string> groups;

  std::string data;
};

std::map<int,PendingTokenTask*> pendingTokenTasks;

void PendingTokenTask::done ( const char* /*URL*/, void * inData, unsigned int size, bool complete )
{
  char *t = (char*)malloc(size+1);
  memcpy(t,inData,size);
  t[size] = 0;
  data += t;
  free(t);

  if (complete)
  {
    // parse out the info

    data = replace_all(data,std::string("\r\n"),"\n");
    data = replace_all(data,std::string("\r"),"\n");

    std::vector<std::string> lines = tokenize(data,std::string("\n"),0,false);

    groups.clear();

    for (size_t i = 0; i < lines.size(); i++)
    {
      std::string &line = lines[i];

      if (line.size())
      {
	if (strstr(line.c_str(),"TOKGOOD") != NULL)
	{
	  groups = tokenize(line,std::string(":"),0,false);

	  if(groups.size() > 1)	// yank the first 2 since that is the sign and the name
	    groups.erase(groups.begin(),groups.begin()+1);
	}
      }
    }
    pendingTokenTasks[requestID] = this;
  }
}

void PendingTokenTask::timeout ( const char* /*URL*/, int /*errorCode*/ )
{
  groups.clear();
  pendingTokenTasks[requestID] = this;
}

void PendingTokenTask::error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ )
{
  groups.clear();
  pendingTokenTasks[requestID] = this;
}

BZFSHTTPAuth::BZFSHTTPAuth():BZFSHTTP(),sessionTimeout(300)
{
}

void BZFSHTTPAuth::setupAuth ( void )
{
  authPage = "<html><body><h4><a href=\"[$WebAuthURL]\">Please Login</a></h4></body></html>";

  std::string returnURL;
  if (getBaseURL().size())
    returnURL += format("http://localhost:%d/%s/",bz_getPublicPort(),getVDir());
  else
    returnURL += getBaseURL();
  returnURL += "?action=login&token=%TOKEN%&user=%USERNAME%";

  tsCallback.URL = "http://my.bzflag.org/weblogin.php?action=weblogin&url=";
  tsCallback.URL += url_encode(returnURL);

  templateSystem.addKey("WebAuthURL",&tsCallback);
}

void BZFSHTTPAuth::TSURLCallback::keyCallback(std::string &data, const std::string &key)
{
  if(compare_nocase(key,"WebAuthURL") == 0)
    data += URL;
}

bool BZFSHTTPAuth::verifyToken ( const HTTPRequest &request, HTTPReply &reply )
{
  // build up the groups list
  std::string token,user;
  request.getParam("token",token);
  request.getParam("user",user);

  std::vector<std::string> groups;

  std::map<int, std::vector<std::string> >::iterator itr = authLevels.begin();

  while (itr != authLevels.end())
  {
    for (size_t i = 0; i < itr->second.size();i++)
    {
      std::string &perm = itr->second[i];

      std::vector<std::string> groupsWithPerm;

      if (compare_nocase(perm,"ADMIN")==0)
	groupsWithPerm = findGroupsWithAdmin();
      else
	groupsWithPerm = findGroupsWithPerm(perm);

      groups.insert(groups.end(),groupsWithPerm.begin(),groupsWithPerm.end());
    }
    itr++;
  }

  PendingTokenTask *task = new PendingTokenTask;

  if (!user.size() || !token.size())
  {
    reply.body += "Invalid response";
    reply.docType = HTTPReply::eText;
    return true;
  }
  task->requestID = request.requestID;
  task->URL = "http://my.bzflag.org/db/";
  task->URL += "?action=CHECKTOKENS&checktokens=" + url_encode(user) + "@";
  task->URL += request.ip;
  task->URL += "%3D" + token;

  task->URL += "&groups=";
  for (size_t g = 0; g < groups.size(); g++)
  {
    task->URL += groups[g];
    if ( g+1 < groups.size())
      task->URL += "%0D%0A";
  }

  // give the task to bzfs and let it do it, when it's done it'll be processed
  bz_addURLJob(task->URL.c_str(),task);

  return false;
}

bool BZFSHTTPAuth::handleRequest ( const HTTPRequest &request, HTTPReply &reply )
{
  flushTasks();
  int sessionID = request.sessionID;
  reply.docType = HTTPReply::eHTML;

  std::map<int,AuthInfo>::iterator authItr = authedSessions.find(sessionID);
  if ( authItr != authedSessions.end() )  // it is one of our authorized users, be nice and forward the request to our child
  {
    // put this back to the default
    reply.docType = HTTPReply::eText;

    authItr->second.time = bz_getCurrentTime();
    bool complete = handleAuthedRequest(authItr->second.level,request,reply);

    if (!complete)
      defferedAuthedRequests.push_back(request.requestID);

    return complete;
  }
  else	// they are not fully authorized yet
  {
    std::map<int,PendingTokenTask*>::iterator pendingItr = pendingTokenTasks.find(request.requestID);
    if (pendingItr != pendingTokenTasks.end())	// they have a token, check it
    {
      AuthInfo info;
      info.time = bz_getCurrentTime();

      if (!pendingItr->second->groups.size())
	info.level = -1;
      else
      {
	if (pendingItr->second->groups.size() == 1)
	  info.level = 0; // just authed, no levels
	else
	  info.level = getLevelFromGroups(pendingItr->second->groups);
	if (info.level >= 0)
	  authedSessions[request.sessionID] = info;
      }

      delete(pendingItr->second);
      pendingTokenTasks.erase(pendingItr);

      // put this back to the default
      reply.docType = HTTPReply::eText;

      bool complete = handleAuthedRequest(info.level,request,reply);

      if (!complete)
	defferedAuthedRequests.push_back(request.requestID);

      return complete;
    }
    else
    {
      std::string action,user,token;
      request.getParam("action",action);
      request.getParam("user",user);
      request.getParam("token",token);
      if (compare_nocase(action,"login") == 0 && user.size() && token.size()) // it's a response from weblogin
	return verifyToken(request,reply);
      else
      {
	// it's someone we know NOTHING about, send them the login
	templateSystem.startTimer();
	size_t s = authPage.find_last_of('.');
	if (s != std::string::npos)
	{
	  if (compare_nocase(authPage.c_str()+s,".tmpl") == 0)
	  {
	    if(!templateSystem.processTemplateFile(reply.body,authPage.c_str()))
	      templateSystem.processTemplate(reply.body,authPage);
	  }
	  else
	    templateSystem.processTemplate(reply.body,authPage);
	}
	else
	  templateSystem.processTemplate(reply.body,authPage);
      }
    }
  }

  return true;
}

bool BZFSHTTPAuth::resumeTask (  int requestID )
{
  // it's token job that is done, go for it
  if(pendingTokenTasks.find(requestID) != pendingTokenTasks.end())
    return true;

  // our child differed
  std::vector<int>::iterator itr = std::find(defferedAuthedRequests.begin(),defferedAuthedRequests.end(),requestID);
  if (itr != defferedAuthedRequests.end())
  {
    if(resumeAuthedTask(requestID)) // they are done, move on
    {
      defferedAuthedRequests.erase(itr);
      return true;
    }
  }
  return false;
}

bool stringInList ( const std::string &str, const std::vector<std::string> stringList )
{
  for (std::vector<std::string>::const_iterator itr = stringList.begin(); itr != stringList.end(); itr++)
  {
    if (compare_nocase(str,*itr) == 0)
      return true;
  }
  return false;
}

void BZFSHTTPAuth::addPermToLevel ( int level, const std::string &perm )
{
  if (level <= 0)
    return;

   if(authLevels.find(level)== authLevels.end())
   {
     std::vector<std::string> t;
     authLevels[level] = t;
   }
   authLevels[level].push_back(perm);
}


int BZFSHTTPAuth::getLevelFromGroups (const std::vector<std::string> &groups )
{
  std::map<int,std::vector<std::string> >::reverse_iterator itr = authLevels.rbegin();

  while (itr != authLevels.rend())
  {
    for (size_t s = 0; s < itr->second.size(); s++ )
    {
      std::string &perm = itr->second[s];

      std::vector<std::string> groupsWithPerm;

      if (compare_nocase(perm,"ADMIN")==0)
	groupsWithPerm = findGroupsWithAdmin();
      else
	groupsWithPerm = findGroupsWithPerm(perm);

      // check the input groups, and see if any of the them are in the groups with this perm
      for (size_t i = 1; i < groups.size(); i++ )
      {  
	if (stringInList(groups[i],groupsWithPerm))
	  return itr->first;
      }
    }
    itr++;
  }
  return 0;
}

void BZFSHTTPAuth::flushTasks ( void )
{
  std::map<int,AuthInfo>::iterator authItr = authedSessions.begin();

  double now = bz_getCurrentTime();
  while (authedSessions.size() && authItr != authedSessions.end())
  {
    if (sessionTimeout + authItr->second.time < now)
    {
      std::map<int,AuthInfo>::iterator t = authItr;
      t++;
      authedSessions.erase(authItr);
      authItr = t;
    }
  }
}


Templateiser::Templateiser()
{
  startTimer();
  setDefaultTokens();
}

Templateiser::~Templateiser()
{

}

void Templateiser::startTimer ( void )
{
  startTime = bz_getCurrentTime();
}

void Templateiser::addKey ( const char *key, TemplateKeyCallback callback )
{
  if (!key || !callback)
    return;

  std::string k;
  tolower(key,k);

  keyFuncCallbacks[k] = callback;
  ClassMap::iterator itr = keyClassCallbacks.find(k);
  if (itr != keyClassCallbacks.end())
    keyClassCallbacks.erase(itr);
}

void Templateiser::addKey ( const char *key, TemplateCallbackClass *callback )
{
  if (!key || !callback)
    return;

  std::string k;
  tolower(key,k);

  keyClassCallbacks[k] = callback;
  KeyMap::iterator itr = keyFuncCallbacks.find(k);
  if (itr != keyFuncCallbacks.end())
    keyFuncCallbacks.erase(itr);
}

void Templateiser::clearKey ( const char *key )
{
  std::string k;
  tolower(key,k);

  ClassMap::iterator itr = keyClassCallbacks.find(k);
  if (itr != keyClassCallbacks.end())
    keyClassCallbacks.erase(itr);

  KeyMap::iterator itr2 = keyFuncCallbacks.find(k);
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

  std::string l;
  tolower(loop,l);

  loopFuncCallbacks[l] = callback;
  ClassMap::iterator itr = loopClassCallbacks.find(l);
  if (itr != loopClassCallbacks.end())
    loopClassCallbacks.erase(itr);
}

void Templateiser::addLoop ( const char *loop, TemplateCallbackClass *callback )
{
  if (!loop || !callback)
    return;

  std::string l;
  tolower(loop,l);

  loopClassCallbacks[l] = callback;
  TestMap::iterator itr = loopFuncCallbacks.find(l);
  if (itr != loopFuncCallbacks.end())
    loopFuncCallbacks.erase(itr);
}

void Templateiser::clearLoop ( const char *loop )
{
  if (!loop)
    return;

  std::string l;
  tolower(loop,l);

  TestMap::iterator itr = loopFuncCallbacks.find(l);
  if (itr != loopFuncCallbacks.end())
    loopFuncCallbacks.erase(itr);

  ClassMap::iterator itr2 = loopClassCallbacks.find(l);
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

  std::string n;
  tolower(name,n);

  ifFuncCallbacks[n] = callback;
  ClassMap::iterator itr = ifClassCallbacks.find(n);
  if (itr != ifClassCallbacks.end())
    ifClassCallbacks.erase(itr);
}

void Templateiser::addIF ( const char *name, TemplateCallbackClass *callback )
{
  if (!name || !callback)
    return;

  std::string n;
  tolower(name,n);

  ifClassCallbacks[n] = callback;
  TestMap::iterator itr = ifFuncCallbacks.find(n);
  if (itr != ifFuncCallbacks.end())
    ifFuncCallbacks.erase(itr);
}

void Templateiser::clearIF ( const char *name )
{
  if(!name)
    return;

  std::string n;
  tolower(name,n);

  ClassMap::iterator itr = ifClassCallbacks.find(n);
  if (itr != ifClassCallbacks.end())
    ifClassCallbacks.erase(itr);

  TestMap::iterator itr2 = ifFuncCallbacks.find(n);
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
  std::string lowerKey;
  tolower(key,lowerKey);

  data.clear();

  ClassMap::iterator itr = keyClassCallbacks.find(lowerKey);
  if (itr != keyClassCallbacks.end()) {
    itr->second->keyCallback(data,key);
    return true;
  }

  KeyMap::iterator itr2 = keyFuncCallbacks.find(lowerKey);
  if (itr2 != keyFuncCallbacks.end()) {
    (itr2->second)(data,key);
    return true;
  }

  return false;
}

bool Templateiser::callLoop ( const std::string &key )
{
  std::string lowerKey;
  tolower(key,lowerKey);

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
  std::string lowerKey;
  tolower(key,lowerKey);

  ClassMap::iterator itr = ifClassCallbacks.find(lowerKey);
  if (itr != ifClassCallbacks.end())
    return itr->second->ifCallback(key);

  TestMap::iterator itr2 = ifFuncCallbacks.find(lowerKey);
  if (itr2 != ifFuncCallbacks.end())
    return (itr2->second)(key);

  return false;
}

bool Templateiser::processTemplateFile ( std::string &code, const char *file )
{
  if (!file)
    return false;

  // find the file
  for (size_t i = 0; i < filePaths.size(); i++ ) {
    std::string path = filePaths[i] + file;
    FILE *fp = fopen(getPathForOS(path).c_str(),"rb");
    if (fp) {
      fseek(fp,0,SEEK_END);
      size_t pos = ftell(fp);
      fseek(fp,0,SEEK_SET);
      char *temp = (char*)malloc(pos+1);
      fread(temp,pos,1,fp);
      temp[pos] = 0;

      std::string val(temp);
      free(temp);
      fclose(fp);

      processTemplate(code,val);
      return true;
    }
  }
  return false;
}


void Templateiser::processTemplate ( std::string &code, const std::string &templateText )
{
  std::string::const_iterator templateItr = templateText.begin();

  while ( templateItr != templateText.end() ) {
    if ( *templateItr != '[' ) {
      code += *templateItr;
      templateItr++;
    } else {
      templateItr++;

      if (templateItr == templateText.end()) {
	code += '[';
      } else {
	switch(*templateItr) {
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
	case '!':
	  processInclude(code,++templateItr,templateText);
	  break;
	}
      }
    }
  }
}

void Templateiser::setPluginName ( const char* name, const char* URL )
{
  if (name)
    pluginName = name;

  if (URL)
    baseURL = URL;
}

void Templateiser::addSearchPath ( const char* path )
{
  if (path)
    filePaths.push_back(std::string(path));
}

void Templateiser::flushSearchPaths ( void )
{
  filePaths.clear();
}

void Templateiser::setDefaultTokens ( void )
{
  addKey("Date",this);
  addKey("Time",this);
  addKey("HostName",this);
  addKey("PageTime",this);
  addKey("BaseURL",this);
  addKey("PluginName",this);
  addIF("Public",this);
}

void Templateiser::keyCallback ( std::string &data, const std::string &key )
{
  if (key == "date") {
    bz_Time time;
    bz_getLocaltime(&time);
    data = format("%d/%d/%d",time.month,time.day,time.year);
  } else if (key == "time") {
    bz_Time time;
    bz_getLocaltime(&time);
    data = format("%d:%d:%d",time.hour,time.minute,time.second);
  } else if (key == "hostname") {
    data = bz_getPublicAddr().c_str();
    if (!data.size())
      data = format("localhost:%d",bz_getPublicPort());;
  } else if (key == "pagetime") {
    data = format("%.3f",bz_getCurrentTime()-startTime);
  } else if (key == "baseurl") {
    data =baseURL;
  } else if (key == "pluginname") {
    data = pluginName;
  }
}

bool Templateiser::loopCallback ( const std::string & /* key */ )
{
  return false;
}

bool Templateiser::ifCallback ( const std::string &key )
{
  if (key == "public")
    return bz_getPublic();

  return false;
}

// processing helpers

std::string::const_iterator Templateiser::readKey ( std::string &key, std::string::const_iterator inItr, const std::string &str )
{
  std::string::const_iterator itr = inItr;

  while ( itr != str.end() ) {
    if (*itr != ']') {
      key += *itr;
      itr++;
    } else {
      // go past the code
      itr++;
      makelower(key);
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

  while (1) {
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

    for ( size_t i = 0; i < keys.size(); i++ ) {
      if ( key == keys[i]) {
	endKey = key;
	code.resize(keyStartItr - inItr);
	std::copy(inItr,keyStartItr,code.begin());
	return itr;
      }
    }
  }
  return itr;
}

void Templateiser::processComment ( std::string & /* code */, std::string::const_iterator &inItr, const std::string &str )
{
  std::string key;
  inItr = readKey(key,inItr,str);
}

void Templateiser::processInclude ( std::string &code, std::string::const_iterator &inItr, const std::string &str )
{
  std::string key;
  inItr = readKey(key,inItr,str);

  // check the search paths for the include file
  if (!processTemplateFile(code,key.c_str()))
    code += "[!" + key + "]";
}

void Templateiser::replaceVar ( std::string &code, std::string::const_iterator &itr, const std::string &str )
{
  // find the end of the ]]
  std::string key;

  itr = readKey(key,itr,str);

  if (itr != str.end()) {
    std::string lowerKey;
    std::string val;

    tolower(key,lowerKey);

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
  std::string::const_iterator itr = readKey(key,inItr,str);

  std::vector<std::string> commandParts = tokenize(key,std::string(" "),0,0);
  if (commandParts.size() < 2) {
    inItr = itr;
    return;
  }

  // check the params
  makelower(commandParts[0]);
  makelower(commandParts[1]);

  if ( commandParts[0] != "start" ) {
    inItr = itr;
    return;
  }

  // now get the code for the loop section section
  std::string loopSection,emptySection;

  std::vector<std::string> checkKeys;
  checkKeys.push_back(format("*end %s",commandParts[1].c_str()));

  std::string keyFound;
  itr = findNextTag(checkKeys,keyFound,loopSection,itr,str);

  if (itr == str.end()) {
    inItr = itr;
    return;
  }

  // do the empty section
  // loops have to have both
  checkKeys.clear();
  checkKeys.push_back(format("*empty %s",commandParts[1].c_str()));
  itr = findNextTag(checkKeys,keyFound,emptySection,itr,str);

  if (callLoop(commandParts[1])) {
    std::string newCode;
    processTemplate(newCode,loopSection);
    code += newCode;

    while(callLoop(commandParts[1])) {
      newCode = "";
      processTemplate(newCode,loopSection);
      code += newCode;
    }
  } else {
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
  std::string::const_iterator itr = readKey(key,inItr,str);

  std::vector<std::string> commandParts = tokenize(key,std::string(" "),0,0);
  if (commandParts.size() < 2) {
    inItr = itr;
    return;
  }

  // check the params
  makelower(commandParts[0]);
  makelower(commandParts[1]);

  if ( commandParts[0] != "if" ) {
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

  if (keyFound == checkKeys[0]) { // we hit an else, so we need to check for it
    // it was the else, so go and find the end too
    if (itr == str.end()) {
      inItr = itr;
      return;
    }

    checkKeys.erase(checkKeys.begin());// kill the else, find the end
    itr = findNextTag(checkKeys,keyFound,elseSection,itr,str);
  }

  // test the if, stuff that dosn't exist is false
  if (callIF(commandParts[1])) {
    std::string newCode;
    processTemplate(newCode,trueSection);
    code += newCode;
  } else {
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
