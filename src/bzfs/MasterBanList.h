/* bzflag
* Copyright (c) 1993 - 2004 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef __MASTER_BAN_LIST_H__
#define __MASTER_BAN_LIST_H__

// bzflag global header
#include "global.h"

// system headers
#include <string>
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

class MasterBanList {
public:
	MasterBanList();
	~MasterBanList();

	const std::string& get ( const std::string URL );
protected:
	std::string	data;
 private:
#ifdef HAVE_CURL
	void collectData(char* ptr, int len);
	static size_t writeFunction(void *ptr, size_t size, size_t nmemb,
				    void *stream);
        CURL *easyHandle;
#endif
};
#endif //__MASTER_BAN_LIST_H__

