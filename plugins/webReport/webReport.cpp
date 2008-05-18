// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_HTTPTemplates.h"

//#include "reportTemplates.h"

std::string templatesDir;

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
  templateSystem.setTemplateDir(tDir);
  
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
  reports = NULL;

  std::string page;
  templateSystem.startTimer();

  std::string action = getParam(paramaters,"action");
  if (!action.size())
    templateSystem.processTemplate(page,loadTemplate("login.tmpl"));
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
    templateSystem.processTemplate(page,loadTemplate("report.tmpl"));

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
