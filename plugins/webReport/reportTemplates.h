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

#endif	//_STAT_TEMPLATE_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
