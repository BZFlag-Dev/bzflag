========================================================================
    DYNAMIC LINK LIBRARY : webstats Project Overview
========================================================================

This is the webstats plugin. It generates web pages based on live stat info.

Template features.
2 templates are used.

stats.tmpl
	This template is the default page for the template loop and will provide the Player loop, and all the player data items for each player
	
player.tmpl
	This template is called if the action paramater is set to player, and the PlayerID paramater is a valid player id.
	It provides all the player data templates for the one player refrenced in PlayerID

Template Tokens

Keys
	Player Loop
	 PlayerCount
		
	Player Data (per player)
	 TeamName
	 Callsign
	 Wins
	 Losses
	 TeamKills
	 Status
	 PlayerID
	 PlayerFlag
	 BZID
	 
IFs 
	Player Loop
		NewTeam
		Players
	Player Data (per player)
	  Spawned
	  Verified
	  Global
	  Admin
	  Op
	  CanSpawn

Loops
	Player Loop
	Players
