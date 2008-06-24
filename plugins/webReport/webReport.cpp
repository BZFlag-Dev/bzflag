// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include "plugin_HTTPTemplates.h"
#include "plugin_groups.h"

//#include "reportTemplates.h"

std::string templatesDir;

class WebReport;

class TokenTask : public bz_BaseURLHandler
{
public:
  TokenTask(WebReport *r);

  virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
  virtual void timeout ( const char* /*URL*/, int /*errorCode*/ );
  virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ );

  WebReport *report;
  int requestID;
  std::string URL;
  std::vector<std::string> groups;

  std::string data;
};

class WebReport : public BZFSHTTPVDir, public TemplateCallbackClass
{
public:
  WebReport();
  ~WebReport();

  void init(const char* tDir);

  void loadDefaultTemplates(void);

  virtual void keyCallback(std::string &data, const std::string &key);
  virtual bool loopCallback(const std::string &key);
  virtual bool ifCallback(const std::string &key);

  std::map<int,bool> pendingAuths;

  std::string loginDefaultTemplate,reportDefaultTemplate;
  bool evenLine;
  bool valid;
  bz_APIStringList *reports;
  int report;

  std::vector<TokenTask*> tasksToFlush;
  std::vector<TokenTask*> tasks;

  void flushTasks ( void );

  Templateiser templateSystem;

  virtual const char * getVDir ( void ){return "WebReport";}
  virtual const char * getDescription ( void ){return "View Reports On-line(Requires Login)";}

  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply );
  virtual bool resumeTask (  int requestID );

  bool verifyToken ( const HTTPRequest &request, HTTPReply &reply );

  std::map<int,double>	authedSessionIDs;
};

WebReport *webReport = NULL;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char *commandLine)
{
  if(webReport)
    delete(webReport);
  webReport = new WebReport;

  if (commandLine && strlen(commandLine))
    webReport->init(commandLine);
  else
    webReport->init("./");

  bz_setclipFieldString("report_index_description", "View reports on-line");

  bz_debugMessage(4, "webReport plugin loaded");

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  if(webReport)
    delete(webReport);
  webReport = NULL;

  bz_debugMessage(4, "webReport plugin unloaded");
  return 0;
}

// template stuff
// globals

WebReport::WebReport()
  : BZFSHTTPVDir()
{
  registerVDir();
  evenLine = false;
  valid = false;
}

WebReport::~WebReport()
{
  flushTasks();

  for (size_t s = 0; s < tasks.size(); s++ )
  {
    bz_removeURLJob(tasks[s]->URL.c_str());
    delete(tasks[s]);
  }
}

void WebReport::init(const char *tDir)
{
  loadDefaultTemplates();

  templateSystem.addSearchPath(tDir);

  templateSystem.addKey("evenodd", this);
  templateSystem.addKey("Report", this);
  templateSystem.addLoop("Reports", this);
  templateSystem.addIF("Valid", this);

  templateSystem.setPluginName("Web Report", getBaseURL().c_str());
}

void WebReport::keyCallback(std::string &data, const std::string &key)
{
  if (key == "evenodd") {
    data = evenLine ? "even" : "odd";
  } else if (key =="report") {
    if (reports && report > 0 && report < (int)reports->size())
      data = reports->get(report).c_str();
    else
      data = "";
  }
}

bool WebReport::loopCallback(const std::string &key)
{
  if (key != "report")
    return false;

  if (!reports || !reports->size())
    return false;

  report++;
  if (report >= (int)reports->size())
    return false;

  evenLine = !evenLine;

  return true;
}

bool WebReport::ifCallback(const std::string &key)
{
  if (key == "valid")
    return valid;
  return false;
}

bool WebReport::resumeTask (  int requestID )
{
  return pendingAuths.find(requestID) != pendingAuths.end();
}

bool WebReport::verifyToken ( const HTTPRequest &request, HTTPReply &reply )
{
  std::vector<std::string> reportGroups = findGroupsWithPerm(bz_perm_viewReports);
  if (!reportGroups.size())
    reportGroups = findGroupsWithAdmin();

  bool validGroups = false;

  if (reportGroups.size())
  {
    if ( reportGroups.size() == 1 && compare_nocase(reportGroups[0],"local.admin") == 0)
      validGroups = false;
    else
      validGroups = true;
  }

  if (validGroups)
  {
    TokenTask *task = new TokenTask(this);

    std::string user,token;
    request.getParam("user",user);
    request.getParam("token",token);

    if (!user.size() || !token.size())
    {
      reply.body += "Invalid response";
      reply.docType = HTTPReply::eText;
      return true;
    }

    task->groups = reportGroups;
    task->requestID = request.requestID;
    task->URL = "http://my.bzflag.org/db/";
    task->URL += "?action=CHECKTOKENS&checktokens=" + url_encode(user) + "@";
    task->URL += request.ip;
    task->URL += "%3D" + token;

    task->URL += "&groups=";
    for (size_t g = 0; g < reportGroups.size(); g++)
    {
      task->URL += reportGroups[g];
      if ( g+1 < reportGroups.size())
	task->URL += "%0D%0A";
    }

    // give the task to bzfs and let it do it, when it's done it'll be processed
    bz_addURLJob(task->URL.c_str(),task);
    tasks.push_back(task);
  }
  else
  {
    reply.body += "Reports are inaccessible";
    reply.docType = HTTPReply::eText;
    return true;
  }
  return false;
}

bool WebReport::handleRequest ( const HTTPRequest &request, HTTPReply &reply )
{
  flushTasks();
  evenLine = false;
  valid = false;
  reports = NULL;

  std::string &page = reply.body;
  int sessionID = request.sessionID;

  std::string action;
  request.getParam("action",action);

  reply.docType = HTTPReply::eHTML;

  // check the sessionID to see of it's authed;
  std::map<int,double>::iterator authItr = authedSessionIDs.find(sessionID);

  if (authItr != authedSessionIDs.end())
  {
    // update the timeout
    authItr->second = bz_getCurrentTime();
    valid = true;

    reports = bz_getReports();
    report = -1;

    if (!templateSystem.processTemplateFile(page, "report.tmpl"))
      templateSystem.processTemplate(page, reportDefaultTemplate);

    if (reports)
      bz_deleteStringList(reports);
  }
  else
  {
    // check and see it's one of our pending auth tasks come back to us
    std::map<int,bool>::iterator pendingItr = pendingAuths.find(request.requestID);
    if (pendingItr != pendingAuths.end())
    {
      valid = pendingItr->second;

      if (valid)
	authedSessionIDs[request.sessionID] = bz_getCurrentTime();

      reports = bz_getReports();
      report = -1;

      if (!templateSystem.processTemplateFile(page, "report.tmpl"))
	templateSystem.processTemplate(page, reportDefaultTemplate);

      if (reports)
	bz_deleteStringList(reports);
    }
    else
    {
      // check and see if it's the return from webauth, if so fire off the web job
      if (compare_nocase(action,"login") == 0)
	return verifyToken(request,reply);
      else // it's a new connection, give it a login
      {
	if (!templateSystem.processTemplateFile(page, "login.tmpl"))
	  templateSystem.processTemplate(page, loginDefaultTemplate);
      }
    }
  }
  return true;
}

void WebReport::loadDefaultTemplates(void)
{
  loginDefaultTemplate = "<html><body><h4><a href=\"";
  loginDefaultTemplate += "http://my.bzflag.org/weblogin.php?action=weblogin&url=";

  std::string returnURL;
  if (getBaseURL().size())
    returnURL += "http://localhost:5154/WebReport/";
  else
   returnURL += getBaseURL();
  returnURL += "?action=login&token=%TOKEN%&user=%USERNAME%";

  loginDefaultTemplate += url_encode(returnURL);
  loginDefaultTemplate += "\">Please Login</a></h4></body></html>";

  reportDefaultTemplate = "<html><body>[?IF Valid][*START Reports][$Report]<br>[*END Reports]There are no reports, sorry[*EMPTY Reports][?ELSE Valid]Invalid Login, sorry[?END Valid]</body></html>";
}

void WebReport::flushTasks ( void )
{
  for (size_t i = 0; i < tasksToFlush.size(); i++)
  {
    TokenTask *task = tasksToFlush[i];

    for (size_t s = 0; s < tasks.size(); s++ )
    {
      if ( tasks[s] == task)
      {
	tasks.erase(tasks.begin()+s);
	s = tasks.size();
      }
    }
    delete(task);
  }

  tasksToFlush.clear();

  // check for long dead sessions

  double now = bz_getCurrentTime();
  double timeout = 300; // 5 min

  std::map<int,double>::iterator authItr = authedSessionIDs.begin();
  
  while (authItr != authedSessionIDs.end())
  {
    if (authItr->second+timeout < now)
    {
      std::map<int,double>::iterator itr = authItr;
      authItr++;
      authedSessionIDs.erase(itr);
    }
    else
      authItr++;
  }
}

TokenTask::TokenTask(WebReport *r)
{
  report = r;
  requestID = -1;
}

void TokenTask::done ( const char* /*URL*/, void * inData, unsigned int size, bool complete )
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

    report->pendingAuths[requestID] = valid;
    report->tasksToFlush.push_back(this);
  }
}

void TokenTask::timeout ( const char* /*URL*/, int /*errorCode*/ )
{
  if (!report)
    return; // fucked

  report->pendingAuths[requestID] = false;
  report->tasksToFlush.push_back(this);
}

void TokenTask::error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ )
{
  if (!report)
    return; // fucked

  report->pendingAuths[requestID] = false;
  report->tasksToFlush.push_back(this);
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
