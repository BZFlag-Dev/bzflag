// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
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

class WebReport : public BZFSHTTPServer, TemplateCallbackClass
{
public:
  WebReport(const char * plugInName);
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

protected:
  virtual bool acceptURL(const char *url) { return true; }
  virtual void getURLData(const char* url, int requestID, const URLParams &parameters, bool get = true);
};

WebReport webReport("report");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char *commandLine)
{
  if (commandLine && strlen(commandLine))
    webReport.init(commandLine);
  else
    webReport.init("./");

  bz_setclipFieldString("report_index_description", "View reports on-line");

  bz_debugMessage(4, "webReport plugin loaded");
  webReport.startupHTTP();

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  webReport.shutdownHTTP();
  bz_debugMessage(4, "webReport plugin unloaded");
  return 0;
}

// template stuff
// globals

WebReport::WebReport(const char *plugInName)
  : BZFSHTTPServer(plugInName)
{
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

  templateSystem.setPluginName("Web Report", getBaseServerURL());
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

void WebReport::getURLData(const char* url, int requestID, const URLParams &parameters, bool get)
{
  flushTasks();
  evenLine = false;
  valid = false;
  reports = NULL;

  std::string page;
  templateSystem.startTimer();

  // check to see if this is a pending report
  if (pendingAuths.find(requestID) != pendingAuths.end())
  {
    valid = pendingAuths[requestID];
    if ( valid)
    {
      reports = bz_getReports();
      report = -1;
    }

    if (!templateSystem.processTemplateFile(page, "report.tmpl"))
      templateSystem.processTemplate(page, reportDefaultTemplate);

    if (reports)
      bz_deleteStringList(reports);

    pendingAuths.erase(pendingAuths.find(requestID));
  }
  else
  {
    std::string action = getParam(parameters, "action");
   
    if (tolower(action) == "report")
    {
      valid = false;
      std::string token = getParam(parameters,"token");
      if (token.size())
      {
	  // start a token job
	std::string user = getParam(parameters,"user");
	if (user.size())
	{
	  // ok now we have to fire off a token job and defer this task

	  std::vector<std::string> reportGroups = findGroupsWithPerm(bz_perm_viewReports);
	  if (!reportGroups.size())
	    reportGroups = findGroupsWithAdmin();

	  if (reportGroups.size())
	  {
	    TokenTask *task = new TokenTask(this);

	    task->groups = reportGroups;
	    task->requestID = requestID;
	    task->URL = "http://my.bzflag.org/db/";
	    task->URL += "?action=CHECKTOKENS&checktokens=" + url_encode(user) + "@";
	    task->URL += getIP(requestID);
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

	    deferRequest(requestID);
	  }
	  else
	     page = "reports are inaccessible";
	}
	else
	  page = "Invalid response, set the username";
      }
      else
      {
	if (bz_validAdminPassword(getParam(parameters, "pass").c_str()))
	  valid = true;

	reports = bz_getReports();
	report = -1;

	if (!templateSystem.processTemplateFile(page, "report.tmpl"))
	  templateSystem.processTemplate(page, reportDefaultTemplate);

	if (reports)
	  bz_deleteStringList(reports);
      }
    }
    else
    {
      if (!templateSystem.processTemplateFile(page, "login.tmpl"))
	  templateSystem.processTemplate(page, loginDefaultTemplate);
    }
  }
 
  setURLDocType(eHTML, requestID);
  setURLDataSize((unsigned int)page.size(), requestID);
  setURLData(page.c_str(), requestID);
}

void WebReport::loadDefaultTemplates(void)
{
  loginDefaultTemplate = "<html><body><h4><a href=\"";
  loginDefaultTemplate += "http://my.bzflag.org/weblogin.php?action=weblogin&url=";

  std::string returnURL;
  if (!getBaseServerURL() || !strlen(getBaseServerURL()))
    returnURL += "http://localhost:5154/report/";
  else
   returnURL += getBaseServerURL();
  returnURL += "?action=report&token=%TOKEN%&user=%USERNAME%";

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
      if ( tasks[s] = task)
      {
	tasks.erase(tasks.begin()+s);
	s = tasks.size();
      }
    }
    delete(task);
  }

  tasksToFlush.clear();
}

TokenTask::TokenTask(WebReport *r)
{
  report = r;
  requestID = -1;
 // std::string token;
  //std::string user;
 // std::string ip;
}

void TokenTask::done ( const char* /*URL*/, void * inData, unsigned int size, bool complete )
{
  char *t = (char*)malloc(size+1);
  memcpy(t,inData,size);
  t[size] = 0;
  data += t;

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

    report->resumeRequest(requestID);
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
