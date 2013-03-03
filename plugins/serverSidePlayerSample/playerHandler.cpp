#include "playerHandler.h"
#include "plugin_utils.h"

void PlayerHandler::added(int playerIndex)
{
	bz_debugMessage(3, "PlayerHandler::added");
	std::string name = format("Sample %d", playerIndex);
	setPlayerData(name.c_str(), NULL, "bot sample", eObservers);
	joinGame();
}

void PlayerHandler::textMessage(int dest, int source, const char *text)
{
	if (dest == getPlayerID())
	{
		sendChatMessage(text,source);
	}
}

void PlayerHandler::playerSpawned(int player, const float pos[3], float rot)
{
	std::string playerName = bz_getPlayerCallsign(player);
	std::string msg = "Oh look, " + playerName + " decided to join us!";
	sendChatMessage(msg.c_str());
}

void PlayerHandler::shotFired(int player, unsigned short shotID)
{
	std::string playerName = bz_getPlayerCallsign(player);
	std::string msg = "Hey, " + playerName + " I bet you think you are special now!";
	sendChatMessage(msg.c_str());
}