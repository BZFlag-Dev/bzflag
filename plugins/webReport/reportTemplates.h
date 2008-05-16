// statTemplates.h : Defines the entry point for the DLL application.
//

#ifndef _STAT_TEMPLATE_H_
#define _STAT_TEMPLATE_H_
#include "bzfsAPI.h"

#include <string>

std::string getFileHeader(void);
std::string getFileFooter(void);

std::string getPlayersHeader(void);
std::string getPlayersLineItem(bz_BasePlayerRecord * rec, bool evenLine);
std::string getPlayersNoPlayers(void);
std::string getPlayersFooter(void);

std::string getTeamHeader(bz_eTeamType team);
std::string getTeamFooter(bz_eTeamType team);

std::string getTeamFontCode(bz_eTeamType team);
std::string getTeamTextName(bz_eTeamType team);

// called to get the code for a template variable
typedef void (*TemplateCallback) ( std::string &data, const std::string &key );

// called for logic statements, loops and if tests
// called repeatedly for each loop to know if it should be done.
// return true to do an instance of the loop
// return false to stop and continue the template
// for if test called to determine true or false
typedef bool (*TemplateTestCallback) ( std::string &key );

void addTemplateCall ( const char *key, TemplateCallback callback );
void clearTemplateCall ( const char *key );
void flushTemplateCalls ( void );

void addTemplateLoop ( const char *loop, TemplateTestCallback callback );
void clearTemplateLoop ( const char *loop );
void flushTemplateLoops ( void );

void addTemplateIF ( const char *name, TemplateTestCallback callback );
void clearTemplateIF ( const char *name );
void flushTemplateIFs ( void );

void processTemplate ( std::string &code, const std::string &templateText );
void setTemplateDir ( const std::string &dir );

#endif	//_STAT_TEMPLATE_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
