/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

/* system headers */
#include <string>
#include <string.h>

/* interface header */
#include "ServerItem.h"

/* common implementation headers */
#include "TextUtils.h"
#include "AnsiCodes.h"

/* local implementation headers */
#include "ServerListCache.h"


ServerItem::ServerItem() :  randomSortWeight((int)(bzfrand()*500)), updateTime(0), cached(false), favorite(false)
{
}

void ServerItem::writeToFile(std::ostream& out) const
{
  char buffer[ServerListCache::max_string+1]; // ServerListCache::max_string is inherited from ServerListCache.h

  // write out desc.
  memset(buffer,0,sizeof(buffer));
  int copyLength = int(description.size() < ServerListCache::max_string ? description.size(): ServerListCache::max_string);
  strncpy(&buffer[0],description.c_str(),copyLength);
  out.write(buffer,sizeof(buffer));

  // write out name
  memset(buffer,0,sizeof(buffer));
  copyLength = int(name.size() < ServerListCache::max_string ? name.size(): ServerListCache::max_string);
  strncpy(&buffer[0],name.c_str(),copyLength);
  out.write(buffer,sizeof(buffer));

  // write out pingpacket
  ping.writeToFile(out);

  nboPackUByte(buffer, favorite);
  out.write(buffer, 1);

  // write out current time
  memset(buffer,0,sizeof(buffer));
  nboPackInt(buffer,(int32_t)updateTime);
  out.write(&buffer[0], 4);
}

bool ServerItem::readFromFile(std::istream& in, int subrevision)
{
  char buffer[ServerListCache::max_string+1];

  //read description
  memset(buffer,0,sizeof(buffer));
  in.read(buffer,sizeof(buffer));
  if ((size_t)in.gcount() < sizeof(buffer)) return false; // failed to read entire string
  description = buffer;

  //read name
  memset(buffer,0,sizeof(buffer));
  in.read(buffer,sizeof(buffer));
  if ((size_t)in.gcount() < sizeof(buffer)) return false; // failed to read entire string
  name = buffer;

  bool pingWorked = ping.readFromFile(in);
  if (!pingWorked) return false; // pingpacket failed to read

  // favorites introduced in subrevision 1
  if (subrevision >=1) {
    uint8_t fav;
    in.read(buffer, 1);
    nboUnpackUByte(buffer, fav);
    favorite = (fav != 0);
  }

  // read in time
  in.read(&buffer[0],4);
  if (in.gcount() < 4) return false;
  int32_t theTime;
  nboUnpackInt(&buffer[0],theTime);
  updateTime = (time_t) theTime;
  cached = true;
  return true;
}

// set the last updated time to now
void ServerItem::setUpdateTime()
{
  updateTime = getNow();
}

// get current age in minutes
time_t ServerItem::getAgeMinutes() const
{
  time_t time = (getNow() - updateTime)/(time_t)60;
  return time;
}

// get current age in seconds
time_t ServerItem::getAgeSeconds() const
{
  time_t time = (getNow() - updateTime);
  return time;
}

// get a simple string which describes the age of item
std::string ServerItem::getAgeString() const
{
  std::string returnMe;
  char buffer [80];
  time_t age = getAgeMinutes();
  if (age < 60) { // < 1 hr
    if (age < 1) {
      time_t ageSecs = getAgeSeconds();
      sprintf(buffer,"%-3ld secs",(long)ageSecs);
    } else {
      sprintf(buffer,"%-3ld mins",(long)age);
    }
  } else { // >= 60 minutes
    float fAge;
    if (age < (24*60)) { // < 24 hours & > 1 hr
      fAge = ((float)age / 60.0f);
      sprintf(buffer, "%-2.1f hrs", fAge);
    } else  { // > 24 hrs
      if (age < (24*60*99)) {  // > 1 day & < 99 days
	fAge = ((float) age / (60.0f*24.0f));
	sprintf(buffer, "%-2.1f days", fAge);
      } else { // over 99 days
	fAge = ((float) age / (60.0f*24.0f));
	sprintf(buffer, "%-3f days", fAge);  //should not happen
      }
    }
  }
  returnMe = buffer;
  return returnMe;
}

// get the current time
time_t ServerItem::getNow() const
{
#if defined(_WIN32)
  return time(NULL);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
#endif
}

bool ServerItem::operator<(const ServerItem &right)
{
  const ServerItem & left = *this;
  if (left.cached && right.cached) {
    if (left.getSortFactor() < right.getSortFactor()) {
      return true;
    }
    else if (left.getSortFactor() == right.getSortFactor()) {
      if (left.getAgeMinutes() > right.getAgeMinutes()) {
	return true;
      }
      else {
	return false;
      }
    }
    else {
      return false;
    }
  }
  else if (!left.cached && !right.cached) {
    if (left.getSortFactor() < right.getSortFactor()) {
      return true;
    }
    else {
      return false;
    }
  }
  else if (!left.cached && right.cached) {
    return false;
  }
  else {
    // left.cached && !right.cached // always less
    return true;
  }
}

int ServerItem::getPlayerCount() const
{
  // if null ping we return a 0 player count
  int curPlayer = 0;
  int maxPlayer = ping.maxPlayers;
  curPlayer = ping.rogueCount + ping.redCount + ping.greenCount +
    ping.blueCount + ping.purpleCount + ping.observerCount;
  if (curPlayer > maxPlayer)
    curPlayer = maxPlayer;
  return curPlayer;
}

std::string ServerItem::getAddrName() const
{
  return TextUtils::format("%s:%d", name.c_str(), ntohs(ping.serverId.port));
}

unsigned int ServerItem::getSortFactor() const
{
  // if null ping we return a 0 player count
  unsigned int value = 0;
  // real players are worth a 1000
  value = ping.rogueCount + ping.redCount + ping.greenCount +
	  ping.blueCount + ping.purpleCount;
  value *= 1000;

  // The constructor sets this to a random value from roughly 0 to 500 (up
  // to half the value of a player). This will be used to randomize the
  // order of servers with the same number of players. No longer will
  // servers starting with an 'a' or a number show higher on the list just
  // because of their hostname.
  value += randomSortWeight;

  return value;
}


void ServerItem::splitAddrTitle(std::string& addr, std::string& title) const
{
  addr = stripAnsiCodes(description);
  title = "";
  const std::string::size_type pos = addr.find_first_of(';');
  if (pos == std::string::npos) {
    return;
  }
  const std::string::size_type tpos = pos + 2; // skip the ';' and ' '
  if (addr.size() > tpos) {
    title = addr.substr(tpos);
  }
  addr.resize(pos);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
