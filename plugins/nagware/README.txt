========================================================================
      NAGWARE, a bzfs plugin                    v 1.00.00  (menotume)
========================================================================

The nagware pluging is designed to encourage players to register their
callsigns at my.bzflag.org/bb.

Nagware can send text messages to unverified players at defined
intervals, as well as automatically kick unverified players.

The plugin is customizable with a plain-text configuration file.

	      ------------------------------
	      See NAGSAMPLE.CFG for details.
	      ------------------------------

Load the plugin in bzfs with the following configuration option:
-loadplugin <path to plugin>,<path to config>

For example (on linux):
-loadplugin /home/bzfs/lib/nagware.so,/home/bzfs/configs/nagware.cfg

If there is an error in the configuration file, the plugin will not
load.  See the output of bzfs (or log file) for details.


The following commands are available fo privileged players (see the
sample config file for how to set the permission name):

/nag on: Enables the plugin for sending messages and kicking players.
	 The plugin is enabled by default.

/nag off: Stops the plugin from sending messages and kicking players.
	 Renable with '/nag on'.  NOTE that the plugin is automatically
	 disabled during a match.

/nag config: Display the current configuration options.

/nag reload: Reload the configuration file. Any config file errors will
	 be shown.

/nag list: Show all unverified players, and how long they have been
	 connected.
