========================================================================
    RogueGenocide : Plugin Overview
========================================================================

Concept: In place of a separate flag, this flag makes genocide act on rogues
in much the same way it does on a team.  A better solution is a separate flag.

Details:

	handled in the plugin:
		1. if a tank(non-rogue) kills a rouge all rogues die.
		2. if a tank(rogue) kills a rogue, only the killed tank dies.**
		2a. a possible modification above would be to have the killer tank
		die as well.  makes the flag require more skill.
	handled by the client/server default:
		1. if a tank kills a member of a team, all the members of that 
		team die.
		2. if a tank kills a member of same team, all the members of that
		team die and the killing tank gets a tk count increase.



Notes:

**This was done for the following reason:
	1. since tk counters are not tracked for the rogue, if the tk'er killed
	all rogue tanks, it could create another nuisance area.

