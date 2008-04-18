// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "reportTemplates.h"

class WebReport : public BZFSHTTPServer
{
public:
  WebReport( const char * plugInName ): BZFSHTTPServer(plugInName){};

  virtual bool acceptURL ( const char *url ){return true;}
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );
};

WebReport webReport("report");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
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

void WebReport::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  bool evenLine = false;

  std::string page;
  page = getFileHeader();

  bz_APIStringList *reports = bz_getReports();
  if (reports)
  {
    for (size_t i = 0; i < reports->size(); i++ )
    {
      std::string report = reports->get((unsigned int)i).c_str();
      page += report;
      page += "<br>";
    }
    bz_deleteStringList(reports);
  }

  // TODO, do the team scores, flag stats, do the last chat lines, etc..

  // finish the document
  page += getFileFooter();

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
