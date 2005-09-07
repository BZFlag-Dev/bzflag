/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfsAPI.h"
#include "PyTeam.h"

namespace Python
{

Team::Team ()
{
	module = Py_InitModule3 ("BZFlag.Team", NULL, NULL);

	// This module just contains team IDs
	PyModule_AddIntConstant (module, "Server",   BZ_SERVER);
	PyModule_AddIntConstant (module, "All",      BZ_ALLUSERS);
	/*PyModule_AddIntConstant (module, "Red",      BZ_RED_TEAM);
	PyModule_AddIntConstant (module, "Green",    BZ_GREEN_TEAM);
	PyModule_AddIntConstant (module, "Blue",     BZ_BLUE_TEAM);
	PyModule_AddIntConstant (module, "Purple",   BZ_PURPLE_TEAM);
	PyModule_AddIntConstant (module, "Rogue",    BZ_ROGUE_TEAM);
	PyModule_AddIntConstant (module, "Rabbit",   BZ_RABBIT_TEAM);
	PyModule_AddIntConstant (module, "Hunter",   BZ_HUNTER_TEAM);
	PyModule_AddIntConstant (module, "Observer", BZ_OBSERVERs); */
}

PyObject *
Team::GetSubModule ()
{
	return module;
}

}
