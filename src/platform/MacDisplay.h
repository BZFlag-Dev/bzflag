
#ifndef BZF_MACDISPLAY_H
#define BZF_MACDISPLAY_H

#include "common.h"
#include "BzfDisplay.h"
#include "BzfEvent.h"
#include "AList.h"
#include "BzfString.h"
#include "bzfgl.h"

#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#define MAC_FG_SLEEP 3 /* ticks to sleep when in foreground */
#define MAC_BG_SLEEP 4 /* ticks to sleep when in background */

class MacDisplay : public BzfDisplay {
  public:
    MacDisplay();
    MacDisplay (const char *name, const char *videoFormat);
    ~MacDisplay() {};

    boolean isValid() const { return is_valid;   }
    boolean isEventPending() const {
      EventRecord eventRec;
      return EventAvail(everyEvent, &eventRec);
    }

    boolean getEvent(BzfEvent&) const;
    //void    setPending (boolean val) const { pending = val; }

    int getWidth() const;
    int getHeight() const;

    int getPassthroughWidth() const;
    int getPassthroughHeight() const;

    //GDHandle getDevice () const { return screen_device; }
    //CGrafPtr getWindow () const { return window; }
    void setWindow(CGrafPtr win) const { window = win; }

    //AGLContext getContext () const { return context; }
    void setContext(CGLContextObj ctx) const { context = ctx;  }

  private:

    boolean doSetResolution(int) { return false; }

    void getKey(BzfKeyEvent &bzf_event, EventRecord &event_rec) const;

    //ResInfo *res_info;
    int screen_width;
    int screen_height;
    //int     screen_depth;

    boolean is_valid;

    static boolean pending;

    static CGLContextObj  context;
    static CGrafPtr    window;

    RgnHandle   cursor_region;

    //GDHandle    screen_device;
};

#endif // BZF_MACDISPLAY_H
//BZF_DEFINE_ALIST(MacDisplayResList, MacDisplayRes);
