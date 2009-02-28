
#include "common.h"

// interface header
#include "LuaClientScripts.h"

// system headers
#include <string.h>
#include <string>
using std::string;

// common headers
#include "TextUtils.h"
#include "StateDatabase.h"

// bzflag headers
#include "../bzflag/playing.h"
#include "../bzflag/Downloads.h"

// local headers
#include "LuaOpenGL.h"
#include "LuaURL.h"

// LuaHandle headers
#include "LuaUser.h"
#include "LuaBzOrg.h"
#include "LuaWorld.h"


/******************************************************************************/
/******************************************************************************/

void LuaClientScripts::Init()
{
	LuaOpenGL::Init();
	LuaURLMgr::SetAccessList(Downloads::instance().getAccessList());
}


void LuaClientScripts::Free()
{
	LuaOpenGL::Free();
}


/******************************************************************************/
/******************************************************************************/

void LuaClientScripts::LuaUserFreeHandler()  { LuaUser::FreeHandler();  }
void LuaClientScripts::LuaBzOrgFreeHandler() { LuaBzOrg::FreeHandler(); }
void LuaClientScripts::LuaWorldFreeHandler() { LuaWorld::FreeHandler(); }

void LuaClientScripts::LuaUserLoadHandler()  { LuaUser::LoadHandler();  }
void LuaClientScripts::LuaBzOrgLoadHandler() { LuaBzOrg::LoadHandler(); }
void LuaClientScripts::LuaWorldLoadHandler() { LuaWorld::LoadHandler(); }

bool LuaClientScripts::LuaUserIsActive()  { return (luaUser  != NULL);   }
bool LuaClientScripts::LuaBzOrgIsActive() { return LuaBzOrg::IsActive(); }
bool LuaClientScripts::LuaWorldIsActive() { return (luaWorld != NULL);   }

bool LuaClientScripts::GetDevMode() { return LuaHandle::GetDevMode(); }
void LuaClientScripts::SetDevMode(bool value) { LuaHandle::SetDevMode(value); }


/******************************************************************************/
/******************************************************************************/

void LuaClientScripts::LuaUserUpdate()
{
	if (luaUser == NULL) {
		return;
	}
	else if (luaUser->RequestReload()) {
		string reason = luaUser->RequestMessage();
		if (!reason.empty()) { reason = ": " + reason; }

		LuaUser::FreeHandler();
		LuaUser::LoadHandler();

		if (luaUser != NULL) {
			addMessage(NULL, "LuaUser reloaded" + reason);
		} else {
			addMessage(NULL, "LuaUser reload failed" + reason);
		}
	}
	else if (luaUser->RequestDisable()) {
		string reason = luaUser->RequestMessage();
		if (!reason.empty()) { reason = ": " + reason; }

		LuaUser::FreeHandler();

		addMessage(NULL, "LuaUser disabled" + reason);
	}
}


void LuaClientScripts::LuaBzOrgUpdate()
{
	if (luaBzOrg == NULL) {
		return;
	}
	else if (luaBzOrg->RequestReload()) {
		string reason = luaBzOrg->RequestMessage();
		if (!reason.empty()) { reason = ": " + reason; }

		LuaBzOrg::FreeHandler();
		LuaBzOrg::LoadHandler();

		if (luaBzOrg != NULL) {
			addMessage(NULL, "LuaBzOrg reloaded" + reason);
		} else {
			addMessage(NULL, "LuaBzOrg reload failed" + reason);
		}
	}
	else if (luaBzOrg->RequestDisable()) {
		string reason = luaBzOrg->RequestMessage();
		if (!reason.empty()) { reason = ": " + reason; }

		LuaBzOrg::FreeHandler();

		addMessage(NULL, "LuaBzOrg disabled" + reason);
	}
}


void LuaClientScripts::LuaWorldUpdate()
{
	if (luaWorld == NULL) {
		return;
	}
	else if (luaWorld->RequestReload()) {
		string reason = luaWorld->RequestMessage();
		if (!reason.empty()) { reason = ": " + reason; }

		LuaWorld::FreeHandler();
		LuaWorld::LoadHandler();

		if (luaWorld != NULL) {
			addMessage(NULL, "LuaWorld reloaded" + reason);
		} else {
			addMessage(NULL, "LuaWorld reload failed" + reason);
		}
	}
	else if (luaWorld->RequestDisable()) {
		string reason = luaWorld->RequestMessage();
		if (!reason.empty()) { reason = ": " + reason; }

		LuaWorld::FreeHandler();

		addMessage(NULL, "LuaWorld disabled" + reason);
	}
}


/******************************************************************************/
/******************************************************************************/

bool LuaClientScripts::LuaUserCommand(const std::string& cmdLine)
{
	const string prefix = "luauser";
	const char* c = cmdLine.c_str();
	if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
		return false;
	}
	c = TextUtils::skipWhitespace(c + prefix.size());

	const string cmd = c;
	if (cmd == "reload") {
		LuaUser::FreeHandler();
		if (BZDB.isTrue("_forbidLuaUser")) {
			addMessage(NULL, "This server forbids LuaUser scripts");
			return false;
		} else {
			LuaUser::LoadHandler();
		}
	}
	else if (cmd == "disable") {
		const bool active = (luaUser != NULL);
		LuaUser::FreeHandler();
		if (active) {
			addMessage(NULL, "LuaUser disabled");
		}
	}
	else if (cmd == "status") {
		if (luaUser != NULL) {
			addMessage(NULL, "LuaUser is enabled");
		} else {
			addMessage(NULL, "LuaUser is disabled");
		}
	}
	else if (cmd == "") {
		addMessage(NULL,
		           "/luauser < status | reload | disable | custom_command ... >");
	}
	else if (luaUser != NULL) {
		return luaUser->RecvCommand(c);
	}
	else {
		return false;
	}

	return true;
}


bool LuaClientScripts::LuaBzOrgCommand(const std::string& cmdLine)
{
	const string prefix = "luabzorg";
	const char* c = cmdLine.c_str();
	if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
		return false;
	}
	c = TextUtils::skipWhitespace(c + prefix.size());

	const string cmd = c;
	if (cmd == "reload") {
		LuaBzOrg::FreeHandler();
		if (BZDB.isTrue("_forbidLuaBzOrg")) {
			addMessage(NULL, "This server forbids LuaBzOrg scripts");
			return false;
		} else {
			LuaBzOrg::LoadHandler();
		}
	}
	else if (cmd == "disable") {
		const bool active = LuaBzOrg::IsActive();
		LuaBzOrg::FreeHandler();
		if (active) {
			addMessage(NULL, "LuaBzOrg disabled");
		}
	}
	else if (cmd == "status") {
		if (luaBzOrg != NULL) {
			addMessage(NULL, "LuaBzOrg is enabled");
		}
		else if (LuaBzOrg::IsActive()) {
			addMessage(NULL, "LuaBzOrg is loading");
		}
		else {
			addMessage(NULL, "LuaBzOrg is disabled");
		}
	}
	else if (cmd == "") {
		addMessage(NULL,
		           "/luabzorg < status | reload | disable | custom_command ... >");
	}
	else if (luaBzOrg != NULL) {
		return luaBzOrg->RecvCommand(c);
	}
	else {
		return false;
	}

	return true;
}


bool LuaClientScripts::LuaWorldCommand(const std::string& cmdLine)
{
	const string prefix = "luaworld";
	const char* c = cmdLine.c_str();
	if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
		return false;
	}
	c = TextUtils::skipWhitespace(c + prefix.size());

	const string cmd = c;
	if (cmd == "reload") {
		LuaWorld::FreeHandler();
		LuaWorld::LoadHandler();
	}
	else if (cmd == "disable") {
		if (BZDB.isTrue("_forceLuaWorld")) {
			return false;
		}
		const bool active = (luaWorld != NULL);
		LuaWorld::FreeHandler();
		if (active) {
			addMessage(NULL, "LuaWorld disabled");
		}
	}
	else if (cmd == "status") {
		if (luaWorld != NULL) {
			addMessage(NULL, "LuaWorld is enabled");
		} else {
			addMessage(NULL, "LuaWorld is disabled");
		}
	}
	else if (cmd == "") {
		addMessage(NULL,
		           "/luaworld < status | reload | disable | custom_command ... >");
	}
	else if (luaWorld != NULL) {
		return luaWorld->RecvCommand(c);
	}
	else {
		return false;
	}

	return true; 
}


/******************************************************************************/
/******************************************************************************/

void LuaClientScripts::LuaUserUpdateForbidden()
{
	if (luaUser == NULL) {
		return;
	}
	if (BZDB.isTrue("_forbidLuaUser")) {
		LuaUser::FreeHandler();
		addMessage(NULL, "This server forbids LuaUser scripts");
		return;
	}
}


void LuaClientScripts::LuaBzOrgUpdateForbidden()
{
	if (!LuaBzOrg::IsActive()) {
		return;
	}
	if (BZDB.isTrue("_forbidLuaBzOrg")) {
		LuaBzOrg::FreeHandler();
		addMessage(NULL, "This server forbids LuaBzOrg scripts");
		return;
	}
}


/******************************************************************************/
/******************************************************************************/
