/* bzflag
* Copyright (c) 1993 - 2004 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* abastracted URL class
*/

#ifndef URL_MANAGER_H
#define URL_MANAGER_H

// system headers
#include <string>
#include <map>
#include <vector>

// local implementation headers
#include "Singleton.h"

class URLManager : public Singleton<URLManager> {
public:
	bool getURL ( const std::string URL, std::string &data );
	bool getURL ( const std::string URL, void **data, unsigned int& size );
	void freeURLData ( void *data );

	void collectData(char* ptr, int len);

protected:
	friend class Singleton<URLManager>;
	URLManager();
	~URLManager();

private:
	// these are CURL specific
	// should probably put them in a pimple
	void *easyHandle;
	void *theData;
	unsigned int theLen;
};


#endif // URL_MANAGER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

