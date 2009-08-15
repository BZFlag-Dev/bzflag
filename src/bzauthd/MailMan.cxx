/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "MailMan.h"
#include "Log.h"
#include <TextUtils.h>
#include <assert.h>

INSTANTIATE_GUARDED_SINGLETON(MailMan)

MailMan::MailMan()
{

}

MailMan::~MailMan()
{

}

bool MailMan::initialize()
{ 
  if(!addTemplate("user_welcome_inactive")) return false;
  if(!addTemplate("user_resend_inactive")) return false;
  if(!addTemplate("user_activate_passwd")) return false;

  for(TemplateMapType::iterator itr = templates.begin(); itr != templates.end(); ++itr) {
    itr->second.replace("{SITENAME}", "Official forums for BZFlag.org");
    itr->second.replace("{WELCOME_MSG}", "Welcome to Official forums for BZFlag.org");
    itr->second.replace("{U_BOARD}", "http://my.bzflag.org/bb");
    itr->second.replace("{EMAIL_SIG}", "--\nThank you.\n\n-The BZFlag Team");
  }

  sLog.outLog("MailMan: initialized");;

  return true;
}

bool MailMan::addTemplate(const std::string &name)
{
  Mail &mail = templates[name];
  return mail.load(name + ".txt");
}

Mail MailMan::newMail(const std::string &_template)
{
  TemplateMapType::iterator itr = templates.find(_template);
  assert(itr != templates.end());
  return itr->second;
}

Mail::Mail()
{

}

Mail::Mail(const Mail &mail)
{
  subject = mail.subject;
  body = mail.body;
}

bool Mail::load(const std::string &path)
{
  FILE *fin = fopen(path.c_str(), "r");
  if(!fin) {
    sLog.outError("MailMan: could not load template from %s", path.c_str());
    return false;
  }

  // get size and reserve
  fseek(fin, 0, SEEK_END);
  long size = ftell(fin);
  std::string file;
  file.reserve(size + 1);

  // read the file in large chunks
  // ok.. maybe reading mail templates doesn't need this kind of efficiency but .. :)
  const int READ_BUF_SIZE = 200000;
  fseek(fin, 0, SEEK_SET);
  char buf[READ_BUF_SIZE];
  while(1) {
    size_t bytes = fread(buf, 1, READ_BUF_SIZE,  fin);
    if(bytes <= 0) break;
    file.append(buf, bytes);
  }

  // separate subject and body
  size_t off = (size_t)strlen("Subject: ");
  size_t pos = file.find('\n', off);
  if(pos == file.npos) return false;
  subject = file.substr(off, pos - off + 1);
  if(file.size() > pos + 1)
    body = file.substr(pos+1);

  return true;
}

bool Mail::send(const std::string &dest)
{
  FILE *cmd_pipe;
  // escape this to prevent executing other commands
  subject = TextUtils::replace_all(subject, "\\", "\\\\");
  subject = TextUtils::replace_all(subject, "\"", "\\\"");
  std::string escaped_dest = TextUtils::replace_all(dest, "\"", "\\\"");
  escaped_dest = TextUtils::replace_all(escaped_dest, "\\", "\\\\");

  std::string cmd = "mail -s \"" + subject + "\" \"" + escaped_dest + "\"";
  if( (cmd_pipe = popen( cmd.c_str(), "w" )) != NULL ) {
    int ret = fprintf(cmd_pipe, "%s", body.c_str());
    pclose(cmd_pipe);
    if(ret != body.size()) {
      sLog.outError("MailMan: could not write body of mail sent to %s", dest.c_str());
      return false;
    } else
      return true;
  } else {
    sLog.outError("MailMan: could not send mail, failed to open pipe");  
    return false;
  }

  return true;
}

void Mail::replace(const std::string &keyword, const std::string &subst)
{
  subject = TextUtils::replace_all(subject, keyword, subst);
  body = TextUtils::replace_all(body, keyword, subst);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
