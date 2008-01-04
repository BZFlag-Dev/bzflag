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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

// interface header
#include "MasterBanList.h"

const std::string& MasterBanList::get ( const std::string URL )
{
  data = "";
  // get all up on the internet and go get the thing
  setURL(URL);
  performWait();
  return data;
}

void MasterBanList::finalization(char *cURLdata, unsigned int length,
				 bool good)
{
  if (good)
    data = std::string(cURLdata, length);
}
