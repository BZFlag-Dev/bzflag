// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include "plugin_HTTPTemplates.h"
#include "plugin_groups.h"

//#include "reportTemplates.h"

std::string templatesDir;

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

  virtual const char * getVDir ( void ){return "WebReport";}
  virtual const char * getDescription ( void ){return "View Reports On-line(Requires Login)";}

  virtual bool handleAuthedRequest ( int level, const HTTPRequest &request, HTTPReply &reply );
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
