/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
This file was originally part of the Xfire Game SDK.

The following is an excerpt from an email conversation with Chris Kirmse,
VP Engineering, Xfire, regarding the Xfire Game SDK's license.

dtremenak@users.sourceforge.net:
What is the license on the SDK materials (particularly
xfiregameclient.cpp)?  BZFlag is an LGPL'ed open-source game (license
text is http://www.gnu.org/copyleft/lesser.html), and we cannot link
with code which is "less free" (speaking of freedom of use)...in other
words, if we use the SDK materials, we need to be able to distribute
them with permission to modify them for any purpose.

chris@xfire.com:
The SDK is available for your use for any purpose and you may
redistribute as you will. Please let me know if it works well for you,
and I can try to help you if you have any questions or problems.
*/

#ifndef __XFIREGAMECLIENT_H__
#define __XFIREGAMECLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
**  XfireIsLoaded()
**
**  returns 1 if application can talk to Xfire, 0 otherwise
*/
int XfireIsLoaded();

/*
**  XfireSetCustomGameDataA()
**
**  ANSI version to tell xfire of custom game data
*/
int XfireSetCustomGameDataA(int num_keys, const char **keys, const char **values);

/*
**  XfireSetCustomGameDataA()
**
**  UNICODE version to tell xfire of custom game data
*/
int XfireSetCustomGameDataW(int num_keys, const wchar_t **keys, const wchar_t **values);

/*
**  XfireSetCustomGameDataUTF8()
**
**  UTF8 version to tell xfire of custom game data
*/
int XfireSetCustomGameDataUTF8(int num_keys, const char **keys, const char **values);

#ifdef UNICODE
#define XfireSetCustomGameData XfireSetCustomGameDataW
#else
#define XfireSetCustomGameData XfireSetCustomGameDataA
#endif


#ifdef __cplusplus
}
#endif

#endif /* __XFIREGAMECLIENT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
