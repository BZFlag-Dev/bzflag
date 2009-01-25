
#include "common.h"

// implementation header
#include "LuaKeySyms.h"

// common headers
#include "BzfEvent.h"

// local headers
#include "LuaInclude.h"


/******************************************************************************/
/******************************************************************************/

static void PushDualPair(lua_State* L, const char* name, int code)
{
	lua_pushstring(L, name);
	lua_pushinteger(L, code);
	lua_rawset(L, -3);
	lua_pushinteger(L, code);
	lua_pushstring(L, name);
	lua_rawset(L, -3);
}


/******************************************************************************/
/******************************************************************************/

bool LuaKeySyms::PushEntries(lua_State* L)
{
	lua_pushliteral(L, "KEYSYMS");
	lua_newtable(L);

#define PUSH_KEYSYM(X) PushDualPair(L, #X, -BzfKeyEvent::X)

	PUSH_KEYSYM(Pause);
	PUSH_KEYSYM(Home);
	PUSH_KEYSYM(End);
	PUSH_KEYSYM(Left);
	PUSH_KEYSYM(Right);
	PUSH_KEYSYM(Up);
	PUSH_KEYSYM(Down);
	PUSH_KEYSYM(PageUp);
	PUSH_KEYSYM(PageDown);
	PUSH_KEYSYM(Insert);
	PUSH_KEYSYM(Backspace);
	PUSH_KEYSYM(Delete);
	PUSH_KEYSYM(Kp0);
	PUSH_KEYSYM(Kp1);
	PUSH_KEYSYM(Kp2);
	PUSH_KEYSYM(Kp3);
	PUSH_KEYSYM(Kp4);
	PUSH_KEYSYM(Kp5);
	PUSH_KEYSYM(Kp6);
	PUSH_KEYSYM(Kp7);
	PUSH_KEYSYM(Kp8);
	PUSH_KEYSYM(Kp9);
	PUSH_KEYSYM(Kp_Period);
	PUSH_KEYSYM(Kp_Divide);
	PUSH_KEYSYM(Kp_Multiply);
	PUSH_KEYSYM(Kp_Minus);
	PUSH_KEYSYM(Kp_Plus);
	PUSH_KEYSYM(Kp_Enter);
	PUSH_KEYSYM(Kp_Equals);
	PUSH_KEYSYM(F1);
	PUSH_KEYSYM(F2);
	PUSH_KEYSYM(F3);
	PUSH_KEYSYM(F4);
	PUSH_KEYSYM(F5);
	PUSH_KEYSYM(F6);
	PUSH_KEYSYM(F7);
	PUSH_KEYSYM(F8);
	PUSH_KEYSYM(F9);
	PUSH_KEYSYM(F10);
	PUSH_KEYSYM(F11);
	PUSH_KEYSYM(F12);
	PUSH_KEYSYM(Help);
	PUSH_KEYSYM(Print);
	PUSH_KEYSYM(Sysreq);
	PUSH_KEYSYM(Break);
	PUSH_KEYSYM(Menu);
	PUSH_KEYSYM(Power);
	PUSH_KEYSYM(Euro);
	PUSH_KEYSYM(Undo);
	PUSH_KEYSYM(LeftMouse);
	PUSH_KEYSYM(MiddleMouse);
	PUSH_KEYSYM(RightMouse);
	PUSH_KEYSYM(WheelUp);
	PUSH_KEYSYM(WheelDown);
	PUSH_KEYSYM(MouseButton6);
	PUSH_KEYSYM(MouseButton7);
	PUSH_KEYSYM(MouseButton8);
	PUSH_KEYSYM(MouseButton9);
	PUSH_KEYSYM(MouseButton10);

#define PUSH_BZKEYSYM(X) PushDualPair(L, #X, -BzfKeyEvent::BZ_ ## X)

	PUSH_BZKEYSYM(Button_1);
	PUSH_BZKEYSYM(Button_2);
	PUSH_BZKEYSYM(Button_3);
	PUSH_BZKEYSYM(Button_4);
	PUSH_BZKEYSYM(Button_5);
	PUSH_BZKEYSYM(Button_6);
	PUSH_BZKEYSYM(Button_7);
	PUSH_BZKEYSYM(Button_8);
	PUSH_BZKEYSYM(Button_9);
	PUSH_BZKEYSYM(Button_10);
	PUSH_BZKEYSYM(Button_11);
	PUSH_BZKEYSYM(Button_12);
	PUSH_BZKEYSYM(Button_13);
	PUSH_BZKEYSYM(Button_14);
	PUSH_BZKEYSYM(Button_15);
	PUSH_BZKEYSYM(Button_16);
	PUSH_BZKEYSYM(Button_17);
	PUSH_BZKEYSYM(Button_18);
	PUSH_BZKEYSYM(Button_19);
	PUSH_BZKEYSYM(Button_20);
	PUSH_BZKEYSYM(Button_21);
	PUSH_BZKEYSYM(Button_22);
	PUSH_BZKEYSYM(Button_23);
	PUSH_BZKEYSYM(Button_24);
	PUSH_BZKEYSYM(Button_25);
	PUSH_BZKEYSYM(Button_26);
	PUSH_BZKEYSYM(Button_27);
	PUSH_BZKEYSYM(Button_28);
	PUSH_BZKEYSYM(Button_29);
	PUSH_BZKEYSYM(Button_30);
	PUSH_BZKEYSYM(Button_31);
	PUSH_BZKEYSYM(Button_32);
	PUSH_BZKEYSYM(Hatswitch_1_up);
	PUSH_BZKEYSYM(Hatswitch_1_right);
	PUSH_BZKEYSYM(Hatswitch_1_down);
	PUSH_BZKEYSYM(Hatswitch_1_left);
	PUSH_BZKEYSYM(Hatswitch_2_up);
	PUSH_BZKEYSYM(Hatswitch_2_right);
	PUSH_BZKEYSYM(Hatswitch_2_down);
	PUSH_BZKEYSYM(Hatswitch_2_left);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/
