// webReport.cpp : Defines the entry point for the DLL application.
//

#include "plugin_authHTTP.h"


AuthTokenTask::AuthTokenTask(BZFSAUTHHTTPServer *s)
{
  server = s;
  requestID = -1;
}

void AuthTokenTask::done ( const char* /*URL*/, void * inData, unsigned int size, bool complete )
{
  char *t = (char*)malloc(size+1);
  memcpy(t,inData,size);
  t[size] = 0;
  data += t;
  free(t);

  if (complete)
  {
    // parse out the info

    bool valid = false;

    data = replace_all(data,std::string("\r\n"),"\n");
    data = replace_all(data,std::string("\r"),"\n");

    std::vector<std::string> lines = tokenize(data,std::string("\n"),0,false);

    bool tokenOk = false;
    bool oneGroupIsIn = false;

    for (size_t i = 0; i < lines.size(); i++)
    {
      std::string &line = lines[i];

      if (line.size())
      {
	if (strstr(line.c_str(),"TOKGOOD") != NULL)
	{
	  tokenOk = true;

	  // the first 2 will be TOKGOOD, and the callsign
	  std::vector<std::string> validGroups = tokenize(line,std::string(":"),0,false);
	  if (validGroups.size() > 2)
	  {
	    for (size_t vg = 2; vg < validGroups.size(); vg++)
	    {
	      for(size_t g = 0; g < groups.size(); g++ )
	      {
		if (compare_nocase(groups[g],validGroups[vg]) == 0)
		  oneGroupIsIn = true;
	      }
	    }
	  }
	}
      }
    }

    if (tokenOk)
      valid = oneGroupIsIn;

    server->resumeRequest(requestID);
    server->pendingAuths[requestID] = valid;
    server->tasksToFlush.push_back(this);
  }
}

void AuthTokenTask::timeout ( const char* /*URL*/, int /*errorCode*/ )
{
  if (!server)
    return; // fucked

  server->pendingAuths[requestID] = false;
  server->tasksToFlush.push_back(this);
}

void AuthTokenTask::error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ )
{
  if (!server)
    return; // fucked

  server->pendingAuths[requestID] = false;
  server->tasksToFlush.push_back(this);
}


BZFSAUTHHTTPServer::BZFSAUTHHTTPServer(const char *p)
  : BZFSHTTPServer(p)
{
  if (p)
    plugInName = p;
}

BZFSAUTHHTTPServer::~BZFSAUTHHTTPServer()
{
  flushTasks();

  for (size_t s = 0; s < tasks.size(); s++ )
  {
    bz_removeURLJob(tasks[s]->URL.c_str());
    delete(tasks[s]);
  }
}

void BZFSAUTHHTTPServer::init( const char* searchDir, const char *displayName, const char* /*description*/ )
{
  if (searchDir)
    templateSystem.addSearchPath(searchDir);
  else
    templateSystem.addSearchPath("./");

  templateSystem.setPluginName(displayName, getBaseServerURL());

  startupHTTP();
}

void BZFSAUTHHTTPServer::stop( void )
{
  shutdownHTTP();
}

void BZFSAUTHHTTPServer::setLoginMessage ( const char* page )
{
  if (page)
    loginMessage = page;
  else
    loginMessage = "";
}

void BZFSAUTHHTTPServer::setLoginMessage ( const std::string& page )
{
  loginMessage = page;
}

void BZFSAUTHHTTPServer::getURLData(const char* /*url*/, int requestID, const URLParams &/*parameters*/, bool /*get*/)
{
  flushTasks();

  std::string page;

  // check the headers and see what we have to do 

  setURLDocType(eHTML, requestID);
  setURLDataSize((unsigned int)page.size(), requestID);
  setURLData(page.c_str(), requestID);
}

void BZFSAUTHHTTPServer::flushTasks ( void )
{
  for (size_t i = 0; i < tasksToFlush.size(); i++)
  {
    AuthTokenTask *task = tasksToFlush[i];

    for (size_t s = 0; s < tasks.size(); s++ )
    {
      if ( tasks[s]== task)
      {
	tasks.erase(tasks.begin()+s);
	s = tasks.size();
      }
    }
    delete(task);
  }

  tasksToFlush.clear();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
