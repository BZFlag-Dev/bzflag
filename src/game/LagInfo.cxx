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

/* interface header */
#include "LagInfo.h"

float LagInfo::threshold = 0.0;
float LagInfo::max       = 0.0;

LagInfo::LagInfo(PlayerInfo *_info)
  : info(_info), lagavg(0), jitteravg(0), lostavg(0), lagalpha(1),
    jitteralpha(1), lostalpha(1), lagcount(0), laglastwarn(0), lagwarncount(0),
    pingpending(false), pingseqno(0), pingssent(0), lasttimestamp(0.0f) {
}

void LagInfo::reset()
{
  nextping       = info->now;
  nextping      += 10.0;
  lastupdate     = info->now;
}

int LagInfo::getLag()
{
  return int(lagavg * 1000);
}

void LagInfo::getLagStats(char* msg) {
  msg[0] = 0;
  if (!info->isPlaying() || !info->isHuman())
    return;

  // don't wait for ping to come back
  int lag = int(lagavg * 1000);
  if (pingpending) {
    float timepassed = info->now - lastping;
    int lastLag = int((lagavg * (1 - lagalpha) + lagalpha * timepassed) * 1000);
    if (lastLag > lag)
      lag = lastLag;
  }
  sprintf(msg,"%s \t: %3d +- %2dms", info->getCallSign(),
	  lag, int(jitteravg * 1000));
  if (lostavg >= 0.01f)
    sprintf(msg + strlen(msg), " %d%% lost/ooo", int(lostavg * 100));
}

// update absolute latency based on LagPing messages
int LagInfo::updatePingLag(void *buf, bool &warn, bool &kick) {
  uint16_t _pingseqno;
  int lag = 0;
  nboUnpackUShort(buf, _pingseqno);
  if (pingseqno == _pingseqno) {
    float timepassed = info->now - lastping;
    // time is smoothed exponentially using a dynamic smoothing factor
    lagavg   = lagavg * (1 - lagalpha) + lagalpha * timepassed;
    lagalpha = lagalpha / (0.9f + lagalpha);
    lag      = int(lagavg * 1000);
    lagcount++;

    // warn players from time to time whose lag is > threshold (-lagwarn)
    if (!info->isObserver() && (threshold > 0) && lagavg > threshold
	&& lagcount - laglastwarn > 2 * lagwarncount) {
      laglastwarn = lagcount;
      lagwarncount++;
      warn = true;
      kick = (lagwarncount++ > max);
    } else {
      warn = false;
      kick = false;
    }
    lostavg     = lostavg * (1 - lostalpha);
    lostalpha   = lostalpha / (0.99f + lostalpha);
    pingpending = false;
  } else {
    warn = false;
    kick = false;
  }
  return lag;
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
    const float jitter = fabs(info->now - lastupdate
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

  if (nextping - info->now >= 0)
    // no time for pinging
    return -1;

  pingseqno = (pingseqno + 1) % 10000;
  if (pingpending) {
    // ping lost
    lostavg   = lostavg * (1 - lostalpha) + lostalpha;
    lostalpha = lostalpha / (0.99f + lostalpha);
    if (!info->isObserver() && (threshold > 0)
	&& lagcount - laglastwarn > 2 * lagwarncount) {
      laglastwarn = lagcount;
      lagwarncount++;
      warn = true;
      kick = (lagwarncount++ > max);
    }
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
  float delta = nextping - info->now;
  if (delta < waitTime)
    waitTime  = delta;
}

void LagInfo::setThreshold(float _threshold, float _max) {
  threshold = _threshold;
  max       = _max;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
