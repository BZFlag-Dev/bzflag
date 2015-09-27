================================================================================
  BZFlag Server Plugin :: SAMPLE_PLUGIN
================================================================================

This sample plugin provides a starting point for new plugins.

On most non-Windows platforms, you can use the 'newplug.sh' script to
automatically copy, rename, and edit the files from this sample to a new plugin.
For instance, if you are making a plugin called AwesomeSauce, you would change
directory to the plugins directory and run:
  ./newplug.sh AwesomeSauce

If you are on Windows, or would prefer to do it manually, you can copy the
SAMPLE_PLUGIN directory to a new name, and then manually replace SAMPLE_PLUGIN
in the filenames and within the files themself with your new plugin name.

The sample plugin is licensed differently than the rest of the project.
It is released under the BSD 2 clause license so that plug-in developer can
use whatever license they want instead of being forced to use LGPL.

The remainder of this README file will provide some other sections that new
plugins could include in their README file. It *WILL NOT* reflect the actual
functionality of the sample plugin.

---------------------------------------
  Loading The Plugin
---------------------------------------

To load the plugin with default settings, use:
  -loadplugin SAMPLE_PLUGIN

To load the plugin with specific settings, use the format:
  -loadplugin SAMPLE_PLUGIN,<powerlevel>,<meaning>,<leet>
For example:
  -loadplugin SAMPLE_PLUGIN,1006,42,1337

To load the plugin and specify a configuration file, use the format:
  -loadplugin SAMPLE_PLUGIN,<configfilename>
For example:
  -loadplugin SAMPLE_PLUGIN,myconfig.cfg

---------------------------------------
  Slash Command Usage
---------------------------------------

/sample on
  Turns the sample on

/sample off
  Turns the sample off

/sample powerlevel <powerlevel>
  Sets the power level to <powerlevel>, where <powerlevel> is a positive integer

---------------------------------------
  Custom Map Objects
---------------------------------------

sample # Start of the sample object

  # Define the position of the sample
  pos <x> <y> <z>

  # Make it awesome
  awesome

end # End of the sample object
