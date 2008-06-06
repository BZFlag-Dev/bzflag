========================================================================
    DYNAMIC LINK LIBRARY : webstats Project Overview
========================================================================

This is the webstats plugin. It generates web pages based on live stat info.

Template features.
2 templates are used.

stats.tmpl
	This template is the default page for the template loop and will provide the Player loop (team sorted), and all the player data items for each player
	
player.tmpl
	This template is called if the action paramater is set to player, and the PlayerID paramater is a valid player id.
	It provides all the player data templates for the one player refrenced in PlayerID
	
flag.tmpl
	This template is called if the action paramater is set to flag, and the FlagID paramater is a valid flag id.
	It provides all the flag data templates for the one flag refrenced in FlagID
	
CUSTOM tmpl files
	If the action paramater is any other word, a tmpl file of that name will be loaded and run. If that fails then stats.tmpl will be used

Template Tokens

Keys
	Flag Info ( per player and in flag loop)
		FlagCount
		FlagName
		FlagPlayer
		FlagPlayerID
		FlagX
		FlagY
		FlagZ
		
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
	 Lag
	 Jitter
	 PacketLoss
	 IPAddress
	 
	 Group Info
		GroupCount
		GroupName
		
	 Flag History
		FlagHistoryCount
		FlagHistoryFlag
		
		Team Info
		  TeamCount
		  TeamScore
		  TeamWins
		  TeamLosses

	Global Team Info
		RedTeamCount
		GreenTeamCount
		BlueTeamCount
		PurpleTeamCount
		ObserverTeamCount
		RogueTeamCount
		HunterTeamCount
		RabbitTeamCount
		RedTeamScore
		GreenTeamScore
		BlueTeamScore
		PurpleTeamScore
		RedTeamWins
		GreenTeamWins
		BlueTeamWins
		PurpleTeamWins
		RedTeamLosses
		GreenTeamLosses
		BlueTeamLosses
		PurpleTeamLosses
		
	Global Server Info
		GameType

IFs 
	Flag Loop
		Flags
		
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
	  
	 Group Info
		Groups
	Flag History
		FlagHistory
		
	Global Team Info
		RedTeam
		GreenTeam
		BlueTeam
		PurpleTeam
		ObserverTeam
		RogueTeam
		HunterTeam
		RabbitTeam
	
	Global Server Info
	  TeamFFA
	  OpenFFA
	  CTF
	  RabbitChase

Loops
	Player Loop
		Players
			Group Info
				Groups
			Flag History
				FlagHistory
				
	Flag Loop
		Flags