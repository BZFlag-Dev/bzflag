
#ifndef BZF_MACDISPLAY_H
#define	BZF_MACDISPLAY_H

#include "common.h"
#include "BzfDisplay.h"
#include "BzfEvent.h"
#include "BzfString.h"
#include "bzfgl.h"

#include <QuickDraw.h>
#include <LowMem.h>
#include <agl.h>

class MacDisplay : public BzfDisplay {


	public:
		MacDisplay();
		MacDisplay (const char *name, const char *videoFormat);
		~MacDisplay () {};

		boolean isValid	   () const { return is_valid;   }
		boolean isEventPending    () const {

		    EventRecord eventRec;
		    return EventAvail(everyEvent, &eventRec);
		}

		boolean getEvent (BzfEvent&) const;
	//	void    setPending (boolean val) const { pending = val; }

		int getWidth  () const { return screen_width;  }
		int getHeight () const { return screen_height; }

	    // GDHandle getDevice () const { return screen_device; }
	    //CGrafPtr getWindow () const { return window; }
	    void     setWindow (CGrafPtr win) const { window = win; }

	    //AGLContext getContext () const { return context; }
	    void       setContext (AGLContext ctx) const { context = ctx;  }
	private:

		boolean	doSetResolution(int) { return false; }

		void getKey (BzfKeyEvent &bzf_event, EventRecord &event_rec) const;

	  //ResInfo *res_info;
		int     screen_width;
	    int     screen_height;
	  // int     screen_depth;


	boolean is_valid;

	static boolean pending;

	    static CGrafPtr    window;
	    static AGLContext  context;
	    RgnHandle   cursor_region;


	   // GDHandle    screen_device;
	   //
};

#endif // BZF_MACDISPLAY_H
//BZF_DEFINE_ALIST(MacDisplayResList, MacDisplayRes);

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

