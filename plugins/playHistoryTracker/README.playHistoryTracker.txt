BZFlag Server Plugin: playHistoryTracker
================================================================================

This plugin tracks kills and shows messages for killing sprees (multiple kills
without dying).  At 5 kills in a row, it shows a "Rampage!" message.  At 10, it
shows a "Killing Spree!" message.  At 20, it shows "Unstoppable!!", and for
every 5 after that it shows "continues to rage on".  If a player kills another
player that had reached one of the spree thresholds, it will show a special
message indicating who stopped their spree.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:
  -loadplugin playHistoryTracker
