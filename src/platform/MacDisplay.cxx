#include "MacDisplay.h"
#include "BzfEvent.h"
#include <CoreFoundation/CoreFoundation.h>

boolean     MacDisplay::pending;
CGrafPtr    MacDisplay::window;
CGLContextObj  MacDisplay::context;

//extern bool gInBackground;
//extern int  gSleepTime;
//extern bool gMouseGrabbed;

MacDisplay::MacDisplay() {
  pending = true;
  
  this->setPassthroughSize(CGDisplayPixelsWide(kCGDirectMainDisplay),
      CGDisplayPixelsHigh(kCGDirectMainDisplay));
}

MacDisplay::MacDisplay(const char *name, const char *videoFormat) {
#pragma unused (name, videoFormat)
  is_valid      = true;
  pending       = false;

  cursor_region = NewRgn();
  ResInfo** resInfo = NULL;
  resInfo = new ResInfo*[1];
  resInfo[0] = new ResInfo("default",
  CGDisplayPixelsWide(kCGDirectMainDisplay),
  CGDisplayPixelsHigh(kCGDirectMainDisplay), 0);
  //numModes = 1;
  //currentMode = 0;

  // register modes
  initResolutions(resInfo, 1, 0);
}

static int           event_cnt = 0;

boolean MacDisplay::getEvent (BzfEvent &bzf_event) const {

  EventRecord eventRec;
  int         gotEvent = false;

  bzf_event.type = (BzfEvent::Type)-1;

  gotEvent = WaitNextEvent(everyEvent, &eventRec, 0, cursor_region);

  if (gotEvent) {
    switch (eventRec.what) {
      case mouseDown:
        bzf_event.type = BzfEvent::KeyDown;
        bzf_event.keyDown.ascii = 0;
        bzf_event.keyDown.shift = 0;
            
        // Emulate a 3-button mouse with modifier keys
                                
        if (eventRec.modifiers      & controlKey)
          bzf_event.keyDown.button = BzfKeyEvent::RightMouse;
        else if (eventRec.modifiers & optionKey)
          bzf_event.keyDown.button = BzfKeyEvent::RightMouse;
        else
          bzf_event.keyDown.button = BzfKeyEvent::LeftMouse;

        break;

      case mouseUp:
        bzf_event.type = BzfEvent::KeyUp;
        bzf_event.keyUp.ascii = 0;
        bzf_event.keyUp.shift = 0;

        if (eventRec.modifiers      & controlKey)
          bzf_event.keyUp.button = BzfKeyEvent::MiddleMouse;
        else if (eventRec.modifiers & optionKey)
          bzf_event.keyUp.button = BzfKeyEvent::RightMouse;
        else
          bzf_event.keyUp.button = BzfKeyEvent::LeftMouse;
        break;

      case keyDown
        bzf_event.type = BzfEvent::KeyDown;
        getKey (bzf_event.keyDown, eventRec);
        break;

      case keyUp:
        bzf_event.type = BzfEvent::KeyUp;
        getKey (bzf_event.keyUp, eventRec);
        break;

      case autoKey:
        bzf_event.type = BzfEvent::KeyDown;
        getKey (bzf_event.keyDown, eventRec);
        break;

      case updateEvt:
        bzf_event.type = BzfEvent::Redraw;
        BeginUpdate((WindowPtr)eventRec.message);
        EndUpdate((WindowPtr)eventRec.message);
        break;

      //case diskEvt:
      //  break;
      //case activateEvt:
      //  break;


      case osEvt:
        switch( (eventRec.message >> 24) & 0x000000FF) {
          case suspendResumeMessage:
//             gInBackground = (eventRec.message & resumeFlag) != 1;
//             gSleepTime = gInBackground ? MAC_BG_SLEEP : MAC_FG_SLEEP;
//             gMouseGrabbed = !gInBackground;
//             if (!gInBackground)
//              SetCursor (&qd.arrow);
            break;
        }
        break;

      case kHighLevelEvent:
        AEProcessAppleEvent(&eventRec);
        break;
    }

    //SIOUXHandleOneEvent (&eventRec);
    return true;
  }

  return false;
}

void MacDisplay::getKey (BzfKeyEvent &bzf_key, EventRecord &event_rec) const {
  bzf_key.ascii = 0;
  char char_code = event_rec.message & charCodeMask;
  switch (char_code) {
    case kUpArrowCharCode   : bzf_key.button = BzfKeyEvent::Up;       break;
    case kDownArrowCharCode : bzf_key.button = BzfKeyEvent::Down;     break;
    case kLeftArrowCharCode : bzf_key.button = BzfKeyEvent::Left;     break;
    case kRightArrowCharCode: bzf_key.button = BzfKeyEvent::Right;    break;
    case kHomeCharCode      : bzf_key.button = BzfKeyEvent::Home;     break;
    case kEndCharCode       : bzf_key.button = BzfKeyEvent::End;      break;
    case kPageUpCharCode    : bzf_key.button = BzfKeyEvent::PageUp;   break;
    case kPageDownCharCode  : bzf_key.button = BzfKeyEvent::PageDown; break;
    case kHelpCharCode      : bzf_key.button = BzfKeyEvent::Insert;   break;
    case kDeleteCharCode    : bzf_key.button = BzfKeyEvent::Delete;   break;
    case kFunctionKeyCharCode  :

    switch ( (event_rec.message << 16) >> 24 ) {
      // These are the f-key codes on my apple extended keyboard
      case 113: // F15
        bzf_key.button = BzfKeyEvent::Pause;
        break;
      //case 107: // F14
      //case 105: // F13
      case 111: // F12
        bzf_key.button = BzfKeyEvent::F12;
        break;
      case 103: // F11
        bzf_key.button = BzfKeyEvent::F11;
        break;
      case 109: // F10
        bzf_key.button = BzfKeyEvent::F10;
        break;
      case 101: // F9
        bzf_key.button = BzfKeyEvent::F9;
        break;
      case 100: // F8
        bzf_key.button = BzfKeyEvent::F8;
        break;
      case 98: // F7
        bzf_key.button = BzfKeyEvent::F7;
        break;
      case 97: // F6
        bzf_key.button = BzfKeyEvent::F6;
        break;
      case 96: // F5
        bzf_key.button = BzfKeyEvent::F5;
        break;
      case 118: // F4
        bzf_key.button = BzfKeyEvent::F4;
        break;
      case 99: // F3
        bzf_key.button = BzfKeyEvent::F3;
        break;
      case 120: // F2
        bzf_key.button = BzfKeyEvent::F2;
        break;
      case 122: // F1
        bzf_key.button = BzfKeyEvent::F1;
        break;
    }
    break;

    default:
      bzf_key.ascii  = char_code;
      bzf_key.button = BzfKeyEvent::NoButton;
  }
}


// if -directory is not used, this function is used to get the default path
// to the data directory which is located in the same directory as the
// application bundle
char *GetMacOSXDataPath(void)
{
  CFStringRef CFstring;
  const char *string;
  char basePath[2048] = {NULL};
  Boolean bah = false;
  OSStatus err = fnfErr;
  CFURLRef pathURL;

  CFBundleRef myAppsBundle = CFBundleGetMainBundle();
  if (myAppsBundle == NULL)
    return NULL;

  CFURLRef myBundleURL = CFBundleCopyBundleURL(myAppsBundle);
  if (myBundleURL == NULL)
    return NULL;
        
  pathURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, myBundleURL);
  CFstring = CFURLCopyStrictPath(pathURL, &bah);
  string = CFStringGetCStringPtr(CFstring, CFStringGetSystemEncoding());
  sprintf(basePath,"/%sdata/", string);

  CFRelease(myBundleURL);
  CFRelease(pathURL);

  return basePath;
}
