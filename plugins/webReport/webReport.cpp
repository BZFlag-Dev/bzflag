// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "reportTemplates.h"


std::string templatesDir;

class WebReport : public BZFSHTTPServer
{
public:
  WebReport( const char * plugInName ): BZFSHTTPServer(plugInName){};

  virtual bool acceptURL ( const char *url ){return true;}
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );

  std::string getURL ( void ) { return std::string(getBaseServerURL());}
};

WebReport webReport("report");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
  if (commandLine)
    templatesDir = commandLine;
  else
    templatesDir = "./";

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
bool evenLine = false;
double pageStartTime = 0;
bool valid = false;
bz_APIStringList *reports;
int report;

void StaticTemplates ( std::string &data, const std::string &key )
{
  if (key == "evenodd")
    data = evenLine ? "even" : "odd";
  if (key == "servername")
  {
    data = bz_getPublicAddr().c_str();
    if (!data.size())
      data = "Local Host";
  }
  else if (key =="pagetime")
    data = format("%f",bz_getCurrentTime()-pageStartTime);
  else if (key =="pluginname")
    data = "WebReport";
  else if (key =="serverurl")
    data = webReport.getURL();
  else if (key =="reporturl")
    data = "report";
  else if (key =="passwordfield")
    data = "pass";
  else if (key =="report")
  {
    if (reports && report > 0 && report < (int)reports->size())
      data = reports->get(report).c_str();
    else
      data = "";
  }
}

bool ReportsLoop ( std::string &key )
{
  if (!reports || !reports->size())
    return false;

  report++;
  if ( report >= (int)reports->size())
    return false;

  evenLine = !evenLine;

  return true;
}

bool ValidIF ( std::string &key )
{
  return valid;
}

std::string loadTemplate ( const char* file )
{
  std::string path = templatesDir + file;
  FILE *fp = fopen(path.c_str(),"rb");
  if (!fp)
    return std::string("");
  fseek(fp,0,SEEK_END);
  size_t pos = ftell(fp);
  fseek(fp,0,SEEK_SET);
  char *temp = (char*)malloc(pos+1);
  fread(temp,pos,1,fp);
  temp[pos] = 0;

  std::string val(temp);
  free(temp);
  fclose(fp);
  return val;
}

void WebReport::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  evenLine = false;
  valid = false;
  pageStartTime = bz_getCurrentTime();
  reports = NULL;

  addTemplateCall ( "evenodd", &StaticTemplates );
  addTemplateCall ( "PluginName", &StaticTemplates );
  addTemplateCall ( "ServerName", &StaticTemplates );
  addTemplateCall ( "PageTime", &StaticTemplates );
  addTemplateCall ( "ServerURL", &StaticTemplates );
  addTemplateCall ( "ReportURL", &StaticTemplates );
  addTemplateCall ( "PasswordField", &StaticTemplates );
  addTemplateCall ( "Report", &StaticTemplates );

  addTemplateLoop ( "Reports", &ReportsLoop );
  addTemplateIF ( "Valid", ValidIF );

  setTemplateDir(templatesDir);

  std::string page;
 // page = getFileHeader();

  std::string action = getParam(paramaters,"action");
  if (!action.size())
    processTemplate(page,loadTemplate("login.tmpl"));
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
    processTemplate(page,loadTemplate("report.tmpl"));

    if (reports)
      bz_deleteStringList(reports);
  }
  setURLDocType(eHTML,requestID);
  setURLDataSize ( (unsigned int)page.size(), requestID );
  setURLData ( page.c_str(), requestID );
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
