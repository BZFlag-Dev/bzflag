/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// webReport.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "bzfsHTTPAPI.h"

std::string templatesDir;

class WebReport : public bz_Plugin, public bzhttp_VDir, public bzhttp_TemplateCallback
{
public:
  WebReport();

  virtual const char* Name (){return "Web Reports";}
  virtual void Init(const char* config);
  virtual void Cleanup();

  void loadDefaultTemplates(void);

  virtual const char* GetTemplateKey(const char* /* key */);
  virtual bool GetTemplateLoop(const char* /* key */, const char* /*param*/);
  virtual bool GetTemplateIF(const char* /* key */, const char* /*param*/);

  std::string noAuthDefaultTeplate,reportDefaultTemplate;
  bool evenLine;
  bool valid;
  bz_APIStringList *reports;
  int report;

  virtual const char* VDirName(){return "WebReport";}
  virtual const char* VDirDescription(){return "View Reports On-line";}

  virtual bzhttp_ePageGenStatus GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce );
  virtual bool GenerateNoAuthPage ( const bzhttp_Request& request, bzhttp_Responce &responce);

  virtual bool AllowResourceDownloads ( void );

  protected:
    std::string resourceDir;
};

BZ_PLUGIN(WebReport)

WebReport::WebReport() :bz_Plugin(), bzhttp_VDir(),bzhttp_TemplateCallback()
{
  evenLine = false;
  valid = false;
}

void WebReport::Init(const char *commandLine)
{
  bzhttp_RegisterVDir(this,this);
  this->RequiredAuthentiction = eBZID;
  this->CacheAuthentication = true;

  loadDefaultTemplates();

  if (commandLine)
    resourceDir = commandLine;
  if (resourceDir.size())
  {
    bzhttp_AddSearchPath("ReportTemplates",commandLine);
    this->ResourceDirs.push_back(std::string(commandLine));
  }
}

bool WebReport::AllowResourceDownloads ( void )
{
  return resourceDir.size() > 0;
}


void WebReport::Cleanup()
{
  bzhttp_RemoveAllVdirs(this);
  Flush();
}

const char* WebReport::GetTemplateKey(const char* _key)
{
  std::string key = _key;
  if (key == "evenodd")
    return evenLine ? "even" : "odd";
  else if (key =="report") {
    if (reports && report > 0 && report < (int)reports->size())
      return reports->get(report).c_str();
  }
  return "";
}

bool WebReport::GetTemplateLoop(const char* _key, const char* /*_param*/)
{
  std::string key = _key;
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

bool WebReport::GetTemplateIF(const char* _key, const char* /*_param*/)
{
  std::string key = _key;
  if (key == "valid")
    return valid;
  return false;
}

 bzhttp_ePageGenStatus WebReport::GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce )
{
  evenLine = false;
  reports = NULL;

  if (!request.UserHasPerm(bz_perm_viewReports))
    return GenerateNoAuthPage(request,responce) ? ePageDone : eNoPage;

  responce.ReturnCode = e200OK;
  responce.DocumentType = eHTML;
 // unsigned int sessionID = request.Session->SessionID;

  std::string action = request.GetParamater("action");

  reports = bz_getReports();
  report = -1;

  // find the report file
  const char* file = bzhttp_FindFile("ReportTemplates","reports.tmpl");
  if (file)
    responce.AddBodyData(bzhttp_RenderTemplate(file,this).c_str());
  else
    responce.AddBodyData(bzhttp_RenderTemplateFromText(reportDefaultTemplate.c_str(),this).c_str());

  if (reports)
    bz_deleteStringList(reports);

  return ePageDone;
}

bool WebReport::GenerateNoAuthPage ( const bzhttp_Request& /*request*/, bzhttp_Responce &responce)
{
  const char* file = bzhttp_FindFile("ReportTemplates","report_noAuth.tmpl");
  if (file)
    responce.AddBodyData(bzhttp_RenderTemplate(file,this).c_str());
  else
  {
    file = bzhttp_FindFile("ReportTemplates","unauthorized.tmpl");
    if (file)
      responce.AddBodyData(bzhttp_RenderTemplate(file,this).c_str());
    else
      responce.AddBodyData(bzhttp_RenderTemplateFromText(noAuthDefaultTeplate.c_str(),this).c_str());
  }

  return true;
}

void WebReport::loadDefaultTemplates(void)
{
  reportDefaultTemplate = "<html><body>[?IF Valid][*START Reports][$Report]<br>[*END Reports]There are no reports, sorry[*EMPTY Reports][?ELSE Valid]Invalid Login, sorry[?END Valid]</body></html>";
  noAuthDefaultTeplate = "<html><body>Unauthorized User</body></html>";
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
