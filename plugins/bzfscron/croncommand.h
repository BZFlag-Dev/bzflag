/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfsAPI.h"

class CronCommand : public bz_CustomSlashCommandHandler
{
public:
  bool handle(int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params);
};

int registerCronCommand();
