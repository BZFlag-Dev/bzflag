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
#include <algorithm>

BZFSHTTPServer::BZFSHTTPServer( const char * plugInName )
{
  if (plugInName)
    name = plugInName;
  listening = false;
}

BZFSHTTPServer::~BZFSHTTPServer()
{
  shutdownHTTP();
  bz_clearMaxWaitTime(vdir.c_str());
}

void BZFSHTTPServer::startupHTTP ( void )
{
  if(listening)
    shutdownHTTP();

  vdir = format("%d/",rand());
  std::string clipField;
  std::vector<std::string> dirs;

  if (bz_clipFieldExists ( "BZFS_HTTPD_VDIRS" ) && bz_getclipFieldString("BZFS_HTTPD_VDIRS")[0] != 0) {
    clipField = bz_getclipFieldString("BZFS_HTTPD_VDIRS");
    dirs = tokenize(clipField,std::string(","),0,false);

    if (name.size() && tolower(name) != "bzfs") {
      if (std::find(dirs.begin(),dirs.end(),name) == dirs.end())
	vdir = name;
    }
  } else {
    if (name.size() && tolower(name) != "bzfs")
      vdir = name;
  }

  clipField += "," + vdir;

  bz_setclipFieldString ( "BZFS_HTTPD_VDIRS", clipField.c_str() );

  // see if I am the indexer or not
  if (!bz_clipFieldExists ( "BZFS_HTTPD_INDEXER" ) || bz_getclipFieldString("BZFS_HTTPD_INDEXER")[0] == 0)
    bz_setclipFieldString("BZFS_HTTPD_INDEXER",format("%p",this).c_str());

  bz_registerEvent (bz_eTickEvent,this);
  bz_registerEvent (bz_eNewNonPlayerConnection,this);

  baseURL = "http://";
  std::string host = "localhost:5154";
  if (bz_getPublicAddr().size())
    host = bz_getPublicAddr().c_str();

  // make sure it has the port
  if ( strrchr(host.c_str(),':') == NULL )
    host += format(":%d",bz_getPublicPort());

  baseURL += host;
  baseURL += "/" + vdir + "/";

  listening = true;
}

void BZFSHTTPServer::shutdownHTTP ( void )
{
  // clear out the 
  if (bz_clipFieldExists ( "BZFS_HTTPD_INDEXER" )) {
    std::string myThis = format("%p",this);
    if (bz_getclipFieldString("BZFS_HTTPD_INDEXER") == myThis)
      bz_setclipFieldString("BZFS_HTTPD_INDEXER","");
  }

  if (bz_clipFieldExists ( "BZFS_HTTPD_VDIRS" ) && bz_getclipFieldString("BZFS_HTTPD_VDIRS")[0] != 0) {
    std::string clipField = bz_getclipFieldString("BZFS_HTTPD_VDIRS");
    std::vector<std::string> dirs = tokenize(clipField,std::string(","),0,false);

    clipField = "";
    for ( int i = 0; i < (int)dirs.size(); i++ ) {
      if ( dirs[i] != dirs[i] )
	clipField += vdir + ",";
    }
    bz_setclipFieldString("BZFS_HTTPD_VDIRS",clipField.c_str());
  }

  if (!listening)
    return;

  bz_removeEvent (bz_eTickEvent,this);
  bz_removeEvent (bz_eNewNonPlayerConnection,this);

  // kill the users;
  std::map<int,HTTPConnectedUsers*>::iterator itr = users.begin();
  while ( itr != users.end() ) {
    bz_removeNonPlayerConnectionHandler (itr->first, this );
    delete(itr->second);
    itr++;
  }
  users.clear();
}

std::string BZFSHTTPServer::getParam ( const URLParams &params, const char* param )
{
  if ( params.find(tolower(std::string(param))) != params.end() )
    return params.find(tolower(std::string(param)))->second;

  return std::string("");
}

void BZFSHTTPServer::process ( bz_EventData *eventData )
{
  // check if I'm the indexer
  if (!bz_clipFieldExists ( "BZFS_HTTPD_INDEXER" ) || bz_getclipFieldString("BZFS_HTTPD_INDEXER")[0] == 0)
    bz_setclipFieldString("BZFS_HTTPD_INDEXER",format("%p",this).c_str());

  std::string myThis = format("%p",this);
  indexer = bz_getclipFieldString("BZFS_HTTPD_INDEXER") == myThis;
  
  if ( eventData->eventType == bz_eTickEvent) {
    if ( listening )
      update();
  } else if ( eventData->eventType == bz_eNewNonPlayerConnection ) {
    if ( listening ) {
      bz_NewNonPlayerConnectionEventData_V1 *connData = (bz_NewNonPlayerConnectionEventData_V1*)eventData;
      char *temp = (char*)malloc(connData->size+1);
      memcpy(temp,connData->data,connData->size);
      temp[connData->size] = 0;
      bz_debugMessagef(4,"Plugin HTTP Base: Non ProtoConnection connection from %d with %s",connData->connectionID,temp);
      free(temp);

      if(bz_registerNonPlayerConnectionHandler ( connData->connectionID, this ) ) {

	HTTPConnectedUsers *user = new HTTPConnectedUsers(connData->connectionID);

	users[connData->connectionID] = user;
	pending ( connData->connectionID, (char*)connData->data, connData->size );
      }
    }
  }
}

void BZFSHTTPServer::update ( void )
{
  double now = bz_getCurrentTime();
  double timeout = 20;

  std::vector<int>  killList;

  std::map<int,HTTPConnectedUsers*>::iterator itr = users.begin();
  while ( itr != users.end() ) {
    double deadTime = now - itr->second->aliveTime;
    if ( itr->second->transferring() || deadTime < timeout ) {
      itr->second->update();
    } else {
      killList.push_back(itr->first);
    }
    itr++;
  }

  for ( int i = 0; i < (int)killList.size(); i++ ) {
    bz_removeNonPlayerConnectionHandler (i, this );
    delete(users[i]);
    users.erase(users.find(i));
  }

  if ( users.size())
    bz_setMaxWaitTime(0.01f,vdir.c_str());
  else
    bz_clearMaxWaitTime(vdir.c_str());
}

void BZFSHTTPServer::generateIndex ( HTTPConnectedUsers *user, int requestID )
{

  theCurrentCommand = new HTTPCommand;
  theCurrentCommand->request = eGet;
  theCurrentCommand->FullURL = "/";
  theCurrentCommand->data = NULL;
  theCurrentCommand->size = 0;
  theCurrentCommand->docType = eHTML;
  theCurrentCommand->returnCode = e200OK;
  theCurrentCommand->URL = "/";

  // dump out an index of the vdirs
  std::string indexPage = "<html><head></head><body>Index for ";
  indexPage += bz_getPublicAddr().c_str();
  indexPage += "<br>";
  std::string clipField;
  std::vector<std::string> dirs;

  if (bz_clipFieldExists ( "BZFS_HTTPD_VDIRS" ) && bz_getclipFieldString("BZFS_HTTPD_VDIRS")[0] != 0) {
    clipField = bz_getclipFieldString("BZFS_HTTPD_VDIRS");
    dirs = tokenize(clipField,std::string(","),0,false);

    indexPage += "<table border=\"0\">";
    for (size_t i = 0; i < dirs.size(); i++ ) {
      indexPage += "<tr><td><a href=\"/";
      indexPage += dirs[i];
      indexPage += "/\">";
      indexPage += dirs[i];
      indexPage += "/</a></td><td>";

      std::string vdirDescripption = dirs[i] + "_index_description";
      if (bz_clipFieldExists ( vdirDescripption.c_str() ) && bz_getclipFieldString(vdirDescripption.c_str())[0] != 0)
	indexPage += bz_getclipFieldString(vdirDescripption.c_str());
      else
	indexPage += "&nbsp;";
 
      indexPage += "</td></tr>";
    }
    indexPage += "</table>";

  }
  indexPage += "</body></html>";

  setURLDataSize ( (unsigned int)indexPage.size(), requestID );
  setURLData ( indexPage.c_str(), requestID );

  if (theCurrentCommand->data) {
    if (theCurrentCommand->size)
      user->startTransfer ( theCurrentCommand );
    else
      free(theCurrentCommand->data);
  }

  theCurrentCommand = NULL;
}

void BZFSHTTPServer::processTheCommand ( HTTPConnectedUsers *user, int /* requestID */, const URLParams &params )
{
  if (acceptURL(theCurrentCommand->URL.c_str())) {
    int requestID = user->connection * 100 + (int)user->pendingCommands.size();

    getURLData ( theCurrentCommand->URL.c_str(), requestID, params, theCurrentCommand->request == eGet );

    if (theCurrentCommand->data) {
      if (theCurrentCommand->size)
	user->startTransfer ( theCurrentCommand );
      else
	free(theCurrentCommand->data);
    }
  } else {
    delete(theCurrentCommand);
  }

  theCurrentCommand = NULL;
}

void BZFSHTTPServer::paramsFromString ( const std::string &paramBlock, URLParams &params )
{
  std::vector<std::string> rawParams = tokenize(paramBlock,"&",0,false);

  for (int i =0; i < (int)rawParams.size(); i++ ) {
    std::string paramItem = rawParams[i];
    if (paramItem.size()) {
      std::vector<std::string> paramChunks = tokenize(paramItem,"=",2,false);
      if (paramChunks.size() > 1 ) {
	std::string key = url_decode(replace_all(paramChunks[0],"+"," "));
	std::string value =  url_decode(replace_all(paramChunks[1],"+"," "));
	params[tolower(key)] = value;
      }
    }
  }
}

std::string BZFSHTTPServer::parseURLParams ( const std::string &FullURL, URLParams &params )
{
  std::string URL = FullURL;

  char *paramStart = (char*)strchr(URL.c_str(),'?');
  if (  paramStart != NULL ) {
    std::string paramBlock = paramStart+1;
    // CHEAP but it works, and dosn't walk over memory
    *paramStart = 0;
    std::string temp = URL.c_str();
    URL = temp.c_str();

    paramsFromString(paramBlock,params);
  }

 return url_decode(URL);
}

void BZFSHTTPServer::pending ( int connectionID, void *d, unsigned int s )
{
  std::map<int,HTTPConnectedUsers*>::iterator itr = users.find(connectionID);
  if (itr == users.end())
    return;

  HTTPConnectedUsers* user = itr->second;

  char *temp = (char*)malloc(s+1);
  memcpy(temp,d,s);
  temp[s] = 0;
  user->commandData += temp;
  free(temp);

  int requestID = connectionID*100 + (int)user->pendingCommands.size();

  if (strstr(user->commandData.c_str(),"\r\n") != NULL ) {
    // we have enough to parse the HTTP command
    std::vector<std::string>  commands = tokenize(user->commandData,std::string("\r\n"),0,false);
    for ( int i = 0; i < (int)commands.size(); i++) {
      if (strstr(user->commandData.c_str(),"\r\n") != NULL ) {
	user->commandData.erase(user->commandData.begin(),user->commandData.begin()+commands[i].size()+2);
	std::vector<std::string> params = tokenize(commands[i],std::string(" "),0,false);
	if (params.size() > 1) {
	  std::string httpCommandString = params[0];

	  if (httpCommandString == "GET") {
	    std::string url = params[1];
	    // make sure it's in our vdir
	    if ( strncmp(tolower(url).c_str()+1,tolower(vdir).c_str(),vdir.size()) == 0) {
	      theCurrentCommand = new HTTPCommand;
	      theCurrentCommand->request = eGet;
	      theCurrentCommand->FullURL = params[1].c_str()+1+vdir.size();
	      theCurrentCommand->data = NULL;
	      theCurrentCommand->size = 0;
	      theCurrentCommand->docType = eText;
	      theCurrentCommand->returnCode = e200OK;

	      URLParams urlparams;
	      theCurrentCommand->URL = parseURLParams ( theCurrentCommand->FullURL, urlparams );

	      processTheCommand(user,requestID,urlparams);
	    } else if (indexer) {
	      generateIndex(user,requestID);
	    }
	  } else if (httpCommandString == "POST") {
	    std::string url = params[1];
	    std::string paramData;

	    // make sure it's in our vdir
	    if ( strncmp(tolower(url).c_str()+1,tolower(vdir).c_str(),vdir.size()) == 0) {
	      int j = (int)commands.size();

	      for ( int c = 1; c  < j; c++ ) {
		std::string line = commands[c];
		if ( line.size() && strchr(line.c_str(),':') == NULL) // it's the post params
		  paramData += line;
	      }

	      theCurrentCommand = new HTTPCommand;
	      theCurrentCommand->request = ePost;
	      theCurrentCommand->FullURL = params[1].c_str()+1+vdir.size();
	      theCurrentCommand->data = NULL;
	      theCurrentCommand->size = 0;
	      theCurrentCommand->docType = eText;
	      theCurrentCommand->returnCode = e200OK;

	      theCurrentCommand->URL = url_decode( theCurrentCommand->FullURL);
	      URLParams urlparams;
	      paramsFromString ( paramData, urlparams );

	      processTheCommand(user,requestID,urlparams);
	    } else if (indexer) {
	      generateIndex(user,requestID);
	    }
	  }
	}
      }
    }
  } else if ( user->commandData.size() > 4 ) {
    if ( !user->transferring() && !user->pendingCommands.size() ) { // it's not sending anything, and it has not goten any commands.
      if ( strncmp(user->commandData.c_str(),"GET",3) != 0) { // it is not a get
	//if ( strncmp(user->commandData.c_str(),"POST",4) != 0) // it is not a post
	user->aliveTime = 99999999999.0;  // then it's somethign we don't support, don't handle it
      }
    }
  }
}

void BZFSHTTPServer::disconnect ( int connectionID )
{
  bz_removeNonPlayerConnectionHandler (connectionID, this );
  delete(users[connectionID]);
  users.erase(users.find(connectionID));
}

void BZFSHTTPServer::setURLDataSize ( size_t size, int /* requestID */)
{
  if (theCurrentCommand->data)
    free(theCurrentCommand->data);

  theCurrentCommand->data = NULL;
  theCurrentCommand->size = size;
}


void BZFSHTTPServer::setURLData ( const char * data, int /* requestID */ )
{
  if (theCurrentCommand->data) {
    free(theCurrentCommand->data);
    theCurrentCommand->data = NULL;
  }

  if (theCurrentCommand->size) {
    theCurrentCommand->data = (char*)malloc(theCurrentCommand->size);
    memcpy(theCurrentCommand->data,data,theCurrentCommand->size);
  }
}

void BZFSHTTPServer::setURLDocType ( HTTPDocumentType docType, int /* requestID */)
{
  theCurrentCommand->docType = docType;
}

void BZFSHTTPServer::setURLReturnCode ( HTTPReturnCode code, int /* requestID */ )
{
  theCurrentCommand->returnCode = code;
}

void BZFSHTTPServer::setURLRedirectLocation ( const char* location, int /* requestID */)
{
  if (!location)
    theCurrentCommand->redirectLocation = "";
  else
    theCurrentCommand->redirectLocation = location;
}


const char * BZFSHTTPServer::getBaseServerURL ( void )
{
  return baseURL.c_str();
}

BZFSHTTPServer::HTTPConnectedUsers::HTTPConnectedUsers(int connectionID )
{
  connection = connectionID;
  pos = 0;
}

BZFSHTTPServer::HTTPConnectedUsers::~HTTPConnectedUsers()
{
  killMe();
}

void BZFSHTTPServer::HTTPConnectedUsers::killMe ( void )
{
  for (int i = 0; i < (int)pendingCommands.size(); i++) {
    free(pendingCommands[i]->data);
    delete(pendingCommands[i]);
  }
  pendingCommands.clear();
  aliveTime -= 10000.0;
}

bool BZFSHTTPServer::HTTPConnectedUsers::transferring ( void )
{
  if (!pendingCommands.size())
    return false;

  HTTPCommand *command = *pendingCommands.begin();
  return pos < command->size;
}

void BZFSHTTPServer::HTTPConnectedUsers::startTransfer ( HTTPCommand *command )
{
  aliveTime = bz_getCurrentTime();
  pendingCommands.push_back(command);
  update();
}
std::string BZFSHTTPServer::HTTPConnectedUsers::getMimeType ( HTTPDocumentType docType )
{
  std::string type = "text/plain";
  switch(docType) {
  case eOctetStream:
    type = "application/octet-stream";
    break;

  case eBinary:
    type = "application/binary";
    break;

  case eHTML:
    type = "text/html";
    break;

  default:
    break;
  }
  return type;
}

std::string BZFSHTTPServer::HTTPConnectedUsers::getReturnCode ( HTTPReturnCode returnCode )
{
  std::string code = "200";
  switch(returnCode) {
  case e404NotFound:
    code = "404";
    break;

  case e403Forbiden:
    code = "403";
    break;

  case e301Redirect:
    code = "301";
    break;

  case e500ServerError:
    code = "500";
    break;

  default:
    break;
  }

  return code;
}

void BZFSHTTPServer::HTTPConnectedUsers::update ( void )
{
  if (!pendingCommands.size())
    return;

  HTTPCommand *currentCommand = pendingCommands[0];
  if ( pos == 0 ) {
    // start a new one
    std::string httpHeaders;
    httpHeaders += "HTTP/1.1 200 OK\n";
    httpHeaders += format("Content-Length: %d\n", (int)currentCommand->size);
    httpHeaders += "Connection: close\n";
    httpHeaders += "Content-Type: " + getMimeType(currentCommand->docType) + "\n";
    if (currentCommand->returnCode != e301Redirect) {
      httpHeaders += "Status-Code: " + getReturnCode(currentCommand->returnCode) + "\n";
    } else {
      if ( currentCommand->redirectLocation.size() ) {
	httpHeaders += "Status-Code: " + getReturnCode(currentCommand->returnCode) + "\n";
	httpHeaders += "Location: " + currentCommand->redirectLocation + "\n";
      } else {  // yeah WTF, you want a redirect but don't have a URL, dude
	httpHeaders += "Status-Code: "  + getReturnCode(e500ServerError) + "\n";
      }
    }
    httpHeaders += "\n";

    if ( !bz_sendNonPlayerData ( connection, httpHeaders.c_str(), (unsigned int)httpHeaders.size()) ) {
      killMe();
      return;
    }
  }
  
  // keep it going
  // wait till the current data is sent
  if (bz_getNonPlayerConnectionOutboundPacketCount(connection) == 0)
  {
    size_t chunkToSend = 1000;

    if ( pos + chunkToSend > currentCommand->size)
      chunkToSend = currentCommand->size-pos;

    bz_sendNonPlayerData ( connection, currentCommand->data+pos, (unsigned int)chunkToSend );

    pos += chunkToSend;
    if (pos >= currentCommand->size) { // if we are done, close this sucker
      pos = 0;
      free(currentCommand->data);
      delete(currentCommand);
      pendingCommands.erase(pendingCommands.begin());
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
