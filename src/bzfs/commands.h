/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__


/** /poll command hook function
 */
void handlePollCmd(int t, const char *message);

/** /vote command hook function
 */
void handleVoteCmd(int t, const char *message);

/** /veto command hook function
 */
void handleVetoCmd(int t, const char * /*message*/);


#endif
// ex: shiftwidth=2 tabstop=8
