
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

/* interface header */
#include "messages.h"

/* system implementation headers */
#include <sstream>
#include <string>

#include <stdio.h>
#include <string.h>

/* common implementation headers */
#include "Pack.h"


PlayerAddMessage::PlayerAddMessage()
{
	playerID = -1;
	team = -1;
	type = -1;
	wins = 0;
	losses = 0;
	tks = 0;
}

bool PlayerAddMessage::unpack ( void* buf )
{
	unsigned char id;
	uint16_t _team, _type, _wins, _losses, _tks;
	char _callsign[_CallSignLen] = {0};
	char _email[_EmailLen] = {0};

	buf = nboUnpackUByte(buf, id);
	buf = nboUnpackUShort(buf, _type);
	buf = nboUnpackUShort(buf, _team);
	buf = nboUnpackUShort(buf, _wins);
	buf = nboUnpackUShort(buf, _losses);
	buf = nboUnpackUShort(buf, _tks);
	buf = nboUnpackString(buf, _callsign, _CallSignLen);
	buf = nboUnpackString(buf, _email, _EmailLen);

	playerID = id;
	team = _team;
	type = _type;
	wins = _wins;
	losses = _losses;
	tks = _tks;

	callsign = _callsign;
	email = _email;

	return true;
}

void*  PlayerAddMessage::pack ( void* buf )
{
	uint16_t _team, _type, _wins, _losses, _tks;
	char _callsign[_CallSignLen] = {0};
	char _email[_EmailLen] = {0};

	unsigned char id;

	id = playerID;
	_team = team;
	_type = type;
	_wins = wins;
	_losses = losses;
	_tks = tks;

	strncpy(_callsign,callsign.c_str(),callsign.size() > _CallSignLen-1 ? _CallSignLen-1 :  callsign.size());
	strncpy(_email,email.c_str(),email.size() > _EmailLen-1 ? _EmailLen-1 :  email.size());

	buf = nboPackUByte(buf, id);
	buf = nboPackUShort(buf, _type);
	buf = nboPackUShort(buf, _type);
	buf = nboPackUShort(buf, _team);
	buf = nboPackUShort(buf, _wins);
	buf = nboPackUShort(buf, _losses);
	buf = nboPackUShort(buf, _tks);
	buf = nboPackString(buf, _callsign, _CallSignLen);
	buf = nboPackString(buf, _email, _EmailLen);

	return buf;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
