// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include "plugin_HTTPTemplates.h"
#include "plugin_groups.h"

//#include "reportTemplates.h"

std::string templatesDir;

// class WebReport;
// 
// class TokenTask : public bz_BaseURLHandler
// {
// public:
//   TokenTask(WebReport *r);
// 
//   virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
//   virtual void timeout ( const char* /*URL*/, int /*errorCode*/ );
//   virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ );
// 
//   WebReport *report;
//   int requestID;
//   std::string URL;
//   std::vector<std::string> groups;
// 
//   std::string data;
// }; 

class WebReport : public BZFSHTTPVDirAuth, public TemplateCallbackClass
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

 // std::vector<TokenTask*> tasksToFlush;
  //std::vector<TokenTask*> tasks;

 // void flushTasks ( void );

 // Templateiser templateSystem;

  virtual const char * getVDir ( void ){return "WebReport";}
  virtual const char * getDescription ( void ){return "View Reports On-line(Requires Login)";}

  virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );

//  virtual bool handleRequest ( const HTTPRequest &request, HTTPReply &reply );
 // virtual bool resumeTask (  int requestID );

 // bool verifyToken ( const HTTPRequest &request, HTTPReply &reply );

//  std::map<int,double>	authedSessionIDs;
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
  : BZFSHTTPVDirAuth()
{
  registerVDir();
  setupAuth();
  evenLine = false;
  valid = false;
}

WebReport::~WebReport()
{
}

void WebReport::init(const char *tDir)
{
  // find any groups that have viewreport or admin, let them fly
  addPermToLevel(1,bz_perm_viewReports);
  addPermToLevel(1,"ADMIN");

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

bool WebReport::handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply )
{
  evenLine = false;
  valid = level == 1;
  reports = NULL;

  std::string &page = reply.body;
  int sessionID = request.sessionID;

  std::string action;
  request.getParam("action",action);

  reply.docType = HTTPReply::eHTML;

  reports = bz_getReports();
  report = -1;

  if (!templateSystem.processTemplateFile(page, "report.tmpl"))
    templateSystem.processTemplate(page, reportDefaultTemplate);

  if (reports)
    bz_deleteStringList(reports);

  return true;
}

void WebReport::loadDefaultTemplates(void)
{
  reportDefaultTemplate = "<html><body>[?IF Valid][*START Reports][$Report]<br>[*END Reports]There are no reports, sorry[*EMPTY Reports][?ELSE Valid]Invalid Login, sorry[?END Valid]</body></html>";
}

// void WebReport::flushTasks ( void )
// {
//   for (size_t i = 0; i < tasksToFlush.size(); i++)
//   {
//     TokenTask *task = tasksToFlush[i];
// 
//     for (size_t s = 0; s < tasks.size(); s++ )
//     {
//       if ( tasks[s] == task)
//       {
// 	tasks.erase(tasks.begin()+s);
// 	s = tasks.size();
//       }
//     }
//     delete(task);
//   }
// 
//   tasksToFlush.clear();
// 
//   // check for long dead sessions
// 
//   double now = bz_getCurrentTime();
//   double timeout = 300; // 5 min
// 
//   std::map<int,double>::iterator authItr = authedSessionIDs.begin();
//   
//   while (authItr != authedSessionIDs.end())
//   {
//     if (authItr->second+timeout < now)
//     {
//       std::map<int,double>::iterator itr = authItr;
//       authItr++;
//       authedSessionIDs.erase(itr);
//     }
//     else
//       authItr++;
//   }
// }
// 
// TokenTask::TokenTask(WebReport *r)
// {
//   report = r;
//   requestID = -1;
// }
// 
// void TokenTask::done ( const char* /*URL*/, void * inData, unsigned int size, bool complete )
// {
//   char *t = (char*)malloc(size+1);
//   memcpy(t,inData,size);
//   t[size] = 0;
//   data += t;
//   free(t);
// 
//   if (complete)
//   {
//     // parse out the info
// 
//     bool valid = false;
// 
//     data = replace_all(data,std::string("\r\n"),"\n");
//     data = replace_all(data,std::string("\r"),"\n");
// 
//     std::vector<std::string> lines = tokenize(data,std::string("\n"),0,false);
// 
//     bool tokenOk = false;
//     bool oneGroupIsIn = false;
// 
//     for (size_t i = 0; i < lines.size(); i++)
//     {
//       std::string &line = lines[i];
// 
//       if (line.size())
//       {
// 	if (strstr(line.c_str(),"TOKGOOD") != NULL)
// 	{
// 	  tokenOk = true;
// 
// 	  // the first 2 will be TOKGOOD, and the callsign
// 	  std::vector<std::string> validGroups = tokenize(line,std::string(":"),0,false);
// 	  if (validGroups.size() > 2)
// 	  {
// 	    for (size_t vg = 2; vg < validGroups.size(); vg++)
// 	    {
// 	      for(size_t g = 0; g < groups.size(); g++ )
// 	      {
// 		if (compare_nocase(groups[g],validGroups[vg]) == 0)
// 		  oneGroupIsIn = true;
// 	      }
// 	    }
// 	  }
// 	}
//       }
//     }
// 
//     if (tokenOk)
//       valid = oneGroupIsIn;
// 
//     report->pendingAuths[requestID] = valid;
//     report->tasksToFlush.push_back(this);
//   }
// }
// 
// void TokenTask::timeout ( const char* /*URL*/, int /*errorCode*/ )
// {
//   if (!report)
//     return; // fucked
// 
//   report->pendingAuths[requestID] = false;
//   report->tasksToFlush.push_back(this);
// }
// 
// void TokenTask::error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ )
// {
//   if (!report)
//     return; // fucked
// 
//   report->pendingAuths[requestID] = false;
//   report->tasksToFlush.push_back(this);
// }

// bool WebReport::verifyToken ( const HTTPRequest &request, HTTPReply &reply )
// {
//   std::vector<std::string> reportGroups = findGroupsWithPerm(bz_perm_viewReports);
//   if (!reportGroups.size())
//     reportGroups = findGroupsWithAdmin();
// 
//   bool validGroups = false;
// 
//   if (reportGroups.size())
//   {
//     if ( reportGroups.size() == 1 && compare_nocase(reportGroups[0],"local.admin") == 0)
//       validGroups = false;
//     else
//       validGroups = true;
//   }
// 
//   if (validGroups)
//   {
//     TokenTask *task = new TokenTask(this);
// 
//     std::string user,token;
//     request.getParam("user",user);
//     request.getParam("token",token);
// 
//     if (!user.size() || !token.size())
//     {
//       reply.body += "Invalid response";
//       reply.docType = HTTPReply::eText;
//       return true;
//     }
// 
//     task->groups = reportGroups;
//     task->requestID = request.requestID;
//     task->URL = "http://my.bzflag.org/db/";
//     task->URL += "?action=CHECKTOKENS&checktokens=" + url_encode(user) + "@";
//     task->URL += request.ip;
//     task->URL += "%3D" + token;
// 
//     task->URL += "&groups=";
//     for (size_t g = 0; g < reportGroups.size(); g++)
//     {
//       task->URL += reportGroups[g];
//       if ( g+1 < reportGroups.size())
// 	task->URL += "%0D%0A";
//     }
// 
//     // give the task to bzfs and let it do it, when it's done it'll be processed
//     bz_addURLJob(task->URL.c_str(),task);
//     tasks.push_back(task);
//   }
//   else
//   {
//     reply.body += "Reports are inaccessible";
//     reply.docType = HTTPReply::eText;
//     return true;
//   }
//   return false;
// }



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
