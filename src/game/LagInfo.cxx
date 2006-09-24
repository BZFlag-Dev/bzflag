/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "LagInfo.h"

float LagInfo::threshold = 0.0;
float LagInfo::jitterthreshold = 0.0;
float LagInfo::max       = 0.0;
float LagInfo::jittermax       = 0.0;

LagInfo::LagInfo(PlayerInfo *_info)
  : info(_info), lagavg(0), jitteravg(0), lostavg(0), lagalpha(1),
    jitteralpha(1), lostalpha(1), lagcount(0), laglastwarn(0), lagwarncount(0),
    jittercount(0), jitterlastwarn(0), jitterwarncount(0), pingpending(false),
    pingseqno(0), pingssent(0), lasttimestamp(0.0f) {
}

void LagInfo::reset()
{
  nextping       = info->now;
  nextping      += 10.0;
  lastupdate     = info->now;
}

int LagInfo::getLag() const
{
  return int(lagavg * 1000);
}

int LagInfo::getJitter() const
{
  return int(jitteravg * 1000);
}

float LagInfo::getLagAvg() const
{
  return lagavg;
}

void LagInfo::getLagStats(char* msg, bool isAdmin) const
{
  msg[0] = 0;
  if (!info->isPlaying() || !info->isHuman())
    return;

  // don't wait for ping to come back
  int lag = int(lagavg * 1000);
  if (pingpending) {
    float timepassed = (float)(info->now - lastping);
    int lastLag = int((lagavg * (1 - lagalpha) + lagalpha * timepassed) * 1000);
    if (lastLag > lag)
      lag = lastLag;
  }

  int numchars;
  if (isAdmin)
    numchars = sprintf(msg, "[%3d] %-24.24s: %3d", info->getPlayerIndex(), 
          TextUtils::str_trunc_continued (info->getCallSign(), 22).c_str(), lag);
  else
    numchars = sprintf(msg, "%-24.24s: %3d", 
          TextUtils::str_trunc_continued (info->getCallSign(), 22).c_str(), lag);

  if (info->isObserver()) {
    sprintf(msg+numchars, "ms");
  } else {
    numchars += sprintf(msg + numchars, " +- %2dms", int(jitteravg * 1000));
    if (lostavg >= 0.01f)
      sprintf(msg + numchars, " %d%% lost/ooo", int(lostavg * 100));
  }
}

// update absolute latency based on LagPing messages
void LagInfo::updatePingLag(void *buf, bool &warn, bool &kick, bool &jittwarn,
			    bool &jittkick) {
  uint16_t _pingseqno;
  nboUnpackUShort(buf, _pingseqno);
  if (pingseqno == _pingseqno) {
    float timepassed = float(info->now - lastping);
    // time is smoothed exponentially using a dynamic smoothing factor
    lagavg   = lagavg * (1 - lagalpha) + lagalpha * timepassed;
    lagalpha = lagalpha / (0.9f + lagalpha);
    lagcount++;
    jittercount++;

    // if lag has been good for some time, forget old warnings
    if (lagavg < threshold && lagcount - laglastwarn >= 20)
      lagwarncount = 0;

    // if jitter has been good for some time, forget old warnings
    if (jitteravg < jitterthreshold && jittercount - jitterlastwarn >= 20)
      jitterwarncount = 0;

    // warn players from time to time whose lag is > threshold (-lagwarn)
    if (!info->isObserver() && (threshold > 0) && lagavg > threshold
	&& lagcount - laglastwarn > 2 * lagwarncount) {
      laglastwarn = lagcount;
      warn = true;
      kick = (++lagwarncount > max);
    } else {
      warn = false;
      kick = false;
    }

    // warn players from time to time whose jitter is > jitterthreshold (-jitterwarn)
    if (!info->isObserver() && (jitterthreshold > 0)
	&& jitteravg > jitterthreshold
        && jittercount - jitterlastwarn > 2 * jitterwarncount) {
      jitterlastwarn = jittercount;
      jittwarn = true;
      jittkick = (++jitterwarncount > jittermax);
    } else {
      jittwarn = false;
      jittkick = false;
    }
    lostavg     = lostavg * (1 - lostalpha);
    lostalpha   = lostalpha / (0.99f + lostalpha);
    pingpending = false;
  } else {
    warn = false;
    kick = false;
    jittwarn = false;
    jittkick = false;
  }
  return;
}

void LagInfo::updateLag(float timestamp, bool ooo) {
  if (!info->isPlaying())
    return;
  if (ooo) {
    lostavg   = lostavg * (1 - lostalpha) + lostalpha;
    lostalpha = lostalpha / (0.99f + lostalpha);
  }
  // don't calc jitter if more than 2 seconds between packets
  if (lasttimestamp > 0.0f && timestamp - lasttimestamp < 2.0f) {
    const float jitter = fabs((float)(info->now - lastupdate)
			      - (timestamp - lasttimestamp));
    // time is smoothed exponentially using a dynamic smoothing factor
    jitteravg   = jitteravg * (1 - jitteralpha) + jitteralpha * fabs(jitter);
    jitteralpha = jitteralpha / (0.99f + jitteralpha);
    lostavg     = lostavg * (1 - lostalpha);
    lostalpha   = lostalpha / (0.99f + lostalpha);
  }
  lasttimestamp = timestamp;
  lastupdate    = info->now;
}

int LagInfo::getNextPingSeqno(bool &warn, bool &kick) {

  warn = false;
  kick = false;

  if (!info->isPlaying() || !info->isHuman())
    return -1;

  if (info->now <= nextping)
    // no time for pinging
    return -1;

  pingseqno = (pingseqno + 1) % 10000;
  if (pingpending) {
    // ping lost
    lostavg   = lostavg * (1 - lostalpha) + lostalpha;
    lostalpha = lostalpha / (0.99f + lostalpha);
  }

  pingpending = true;
  lastping    = info->now;
  nextping    = info->now;
  nextping   += 10.0f;
  pingssent++;
  return pingseqno;
}

// update absolute latency based on LagPing messages
void LagInfo::updateLatency(float &waitTime) {
  if (!info->isPlaying() || !info->isHuman())
    return;
  float delta = (float)(nextping - info->now);
  if (delta < waitTime)
    waitTime  = delta;
}

void LagInfo::setThreshold(float _threshold, float _max) {
  threshold = _threshold;
  max       = _max;
}

void LagInfo::setJitterThreshold(float _jitterthreshold, float _jittermax) {
  jitterthreshold = _jitterthreshold;
  jittermax       = _jittermax;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
