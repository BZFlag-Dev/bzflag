// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_HTTPTemplates.h"

//#include "reportTemplates.h"

std::string templatesDir;

std::string loginDefaultTemplate,reportDefaultTemplate;
void loadDefaultTemplates(void);

class WebReport : public BZFSHTTPServer, TemplateCallbackClass
{
public:
  WebReport( const char * plugInName );

  void init ( std::string &tDir );

  virtual bool acceptURL ( const char *url ){return true;}
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );

  virtual void keyCallback ( std::string &data, const std::string &key );
  virtual bool loopCallback ( const std::string &key );
  virtual bool ifCallback ( const std::string &key );

  bool evenLine;
  bool valid;
  bz_APIStringList *reports;
  int report;

  Templateiser	templateSystem;
};

WebReport webReport("report");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
  if (commandLine)
    templatesDir = commandLine;
  else
    templatesDir = "./";

  loadDefaultTemplates();
  webReport.init(templatesDir);

  bz_setclipFieldString("report_index_description","View reports on-line");

  bz_debugMessage(4,"webReport plugin loaded");
  webReport.startupHTTP();

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  webReport.shutdownHTTP();
  bz_debugMessage(4,"webReport plugin unloaded");
  return 0;
}

// template stuff
// globals

WebReport::WebReport( const char * plugInName ): BZFSHTTPServer(plugInName)
{
  evenLine = false;
  valid = false;
};

void WebReport::init ( std::string &tDir )
{
  templateSystem.addSearchPath(tDir.c_str());
  
  templateSystem.addKey("evenodd", this);
  templateSystem.addKey("Report", this);
  templateSystem.addLoop("Reports", this);
  templateSystem.addIF("Valid", this);

 templateSystem.setPluginName("Web Report",getBaseServerURL());
}

void WebReport::keyCallback ( std::string &data, const std::string &key )
{
  if (key == "evenodd")
    data = evenLine ? "even" : "odd";
  else if (key =="report")
  {
    if (reports && report > 0 && report < (int)reports->size())
      data = reports->get(report).c_str();
    else
      data = "";
  }
}

bool WebReport::loopCallback ( const std::string &key )
{
  if (key != "report")
    return false;

  if (!reports || !reports->size())
    return false;

  report++;
  if ( report >= (int)reports->size())
    return false;

  evenLine = !evenLine;

  return true;
}

bool WebReport::ifCallback ( const std::string &key )
{
  if (key == "valid")
    return valid;
  return false;
}

void WebReport::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  evenLine = false;
  valid = false;
  reports = NULL;

  std::string page;
  templateSystem.startTimer();

  std::string action = getParam(paramaters,"action");
  if (!action.size())
  {
    if (!templateSystem.processTemplateFile(page,"login.tmpl"))
      templateSystem.processTemplate(page,loginDefaultTemplate);
  }
  else
  {
    valid = false;

    if (tolower(action) == "report")
    {
      if (bz_validAdminPassword(getParam(paramaters,"pass").c_str()))
	valid = true;
  
      reports = bz_getReports();
      report = -1;
    }
    if (!templateSystem.processTemplateFile(page,"report.tmpl"))
      templateSystem.processTemplate(page,reportDefaultTemplate);

    if (reports)
      bz_deleteStringList(reports);
  }
  setURLDocType(eHTML,requestID);
  setURLDataSize ( (unsigned int)page.size(), requestID );
  setURLData ( page.c_str(), requestID );
}

void loadDefaultTemplates(void)
{
  loginDefaultTemplate = "<html><body><h4>Please Enter the Server Password</h3><form name=\"input\" action=\"[$BaseURL]\" method=\"put\">";
  loginDefaultTemplate += "<input type=\"hidden\" name=\"action\" value=\"report\"><input type=\"text\" name=\"pass\"><input type=\"submit\" value =\"submit\"></form></body></html>";

  reportDefaultTemplate = "<html><body>[?IF Valid][*START Reports][$Report]<br>[*END Reports]There are no reports, sorry[*EMPTY Reports][?ELSE Valid]Invalid Login, sorry[?END Valid]</body></html>";
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
