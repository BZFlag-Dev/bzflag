// SAMPLE_PLUGIN.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
	bz_debugMessage(4,"SAMPLE_PLUGIN plugin loaded");
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_debugMessage(4,"SAMPLE_PLUGIN plugin unloaded");
	return 0;
}
