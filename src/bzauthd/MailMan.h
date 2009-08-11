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

#ifndef __BZAUTHD_MAILMAN_H__
#define __BZAUTHD_MAILMAN_H__

#include "Thread.h"
#include <map>
#include <string>

class Mail
{
public:
  Mail();
  Mail(const Mail &mail);

  bool load(const std::string &path);

  bool send(const std::string &dest);
  void replace(const std::string &keyword, const std::string &subst);
private:
  std::string subject;
  std::string body;
};

class MailMan : public GuardedSingleton<MailMan>
{
public:
  MailMan();
  ~MailMan();

  bool initialize();

  Mail newMail(const std::string &_template);

private:
  bool addTemplate(const std::string &name);

  typedef std::map<std::string, Mail> TemplateMapType;
  TemplateMapType templates;
};

#define sMailMan MailMan::guard().instance()

#endif // __BZAUTHD_MAILMAN_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
