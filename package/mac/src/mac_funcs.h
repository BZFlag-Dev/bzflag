
#define MAC_FG_SLEEP 3 /* ticks to sleep when in foreground */
#define MAC_BG_SLEEP 4 /* ticks to sleep when in background */

extern "C" {

  void MacLaunchServer (int argc, const char **argv);
  void MacOneEvent ();
}
