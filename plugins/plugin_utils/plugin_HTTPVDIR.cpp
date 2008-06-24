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

#include "plugin_HTTPVDIR.h"
#include "plugin_utils.h"
#include "plugin_groups.h"
#include <algorithm>

BZFSHTTPVDir::BZFSHTTPVDir()
{
  bz_loadPlugin("HTTPServer",NULL);
}

void BZFSHTTPVDir::registerVDir ( void )
{
  bz_callCallback("RegisterHTTPDVDir",this);
}

BZFSHTTPVDir::~BZFSHTTPVDir()
{
  bz_callCallback("RemoveHTTPDVDir",this);
}
std::string BZFSHTTPVDir::getBaseURL ( void )
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

  std::map<std::string, std::string>::const_iterator itr = paramaters.find(tolower(std::string(param)));
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

  std::map<std::string, std::string>::const_iterator itr = paramaters.find(tolower(param));
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

BZFSHTTPVDirAuth::BZFSHTTPVDirAuth():BZFSHTTPVDir(),sessionTimeout(300)
{
}

void BZFSHTTPVDirAuth::setupAuth ( void )
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

void BZFSHTTPVDirAuth::TSURLCallback::keyCallback(std::string &data, const std::string &key)
{
  if(compare_nocase(key,"WebAuthURL") == 0)
    data += URL;
}

bool BZFSHTTPVDirAuth::verifyToken ( const HTTPRequest &request, HTTPReply &reply )
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

bool BZFSHTTPVDirAuth::handleRequest ( const HTTPRequest &request, HTTPReply &reply )
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
      info.level = getLevelFromGroups(pendingItr->second->groups);
      if (info.level >= 0)
	authedSessions[request.sessionID] = info;

      delete(pendingItr->second);
      pendingTokenTasks.erase(pendingItr);
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

bool BZFSHTTPVDirAuth::resumeTask (  int requestID )
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

void BZFSHTTPVDirAuth::addPermToLevel ( int level, const std::string &perm )
{
  if (level < 0)
    return;

   if(authLevels.find(level)== authLevels.end())
   {
     std::vector<std::string> t;
     authLevels[level] = t;
   }
   authLevels[level].push_back(perm);
}


int BZFSHTTPVDirAuth::getLevelFromGroups (const std::vector<std::string> &groups )
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
      for (std::vector<std::string>::const_iterator group = groups.begin(); group != groups.end(); group++)
      {  
	if (stringInList(*group,groupsWithPerm))
	  return itr->first;
      }
    }
    itr++;
  }
  return -1;
}

void BZFSHTTPVDirAuth::flushTasks ( void )
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
