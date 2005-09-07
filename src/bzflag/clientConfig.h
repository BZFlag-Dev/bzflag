/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* clientConfig.h:
*	setup and load the client side prefrences
*/

#ifndef	__CLIENT_CONFIG_H__
#define	__CLIENT_CONFIG_H__

#include <vector>
#include <string>

extern std::vector<std::string>	configQualityValues;
extern std::vector<std::string>	configViewValues;

void initConfigData ( void );

std::string	getOldConfigFileName(void);
std::string getCurrentConfigFileName(void);
void findConfigFile(void);
void updateConfigFile(void);

#endif // __CLIENT_CONFIG_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
