#include "MacDisplay.h"
#include "BzfEvent.h"

#include <CoreFoundation/CoreFoundation.h>

bool     MacDisplay::pending;
CGrafPtr    MacDisplay::window;
CGLContextObj  MacDisplay::context;

MacDisplay::MacDisplay() {
  pending = true;

  setPassthroughSize(CGDisplayPixelsWide(kCGDirectMainDisplay),
		     CGDisplayPixelsHigh(kCGDirectMainDisplay));
}

MacDisplay::MacDisplay(const char *, const char *) {
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

bool MacDisplay::isEventPending() const
{
  ::OSStatus	status		= ::noErr;
  ::EventRef	eventRef	= 0;

  status = ::ReceiveNextEvent(0, NULL, 0, false, &eventRef);
  return (status == ::noErr) ? true : false;
}

bool MacDisplay::peekEvent (BzfEvent &) const
{
  return false;
}

bool MacDisplay::getEvent (BzfEvent &bzf_event) const {
  ::EventRef		eventRef	= 0;
  ::OSStatus		status		= ::noErr;
  ::UInt32		eventClass	= 0;
  ::UInt32		eventKind	= 0;
  ::EventTime		eventTime	= 0;
  ::HIPoint		eventLocation	= {0, 0};
  ::HIPoint		eventDelta	= {0, 0};
  ::UInt32		eventModifiers	= 0;
  ::EventMouseButton	eventButtons	= 0;
  char			eventChar	= 0;
  ::UInt32		eventKeyCode	= 0;
  ::WindowRef		eventWindow	= NULL;
  const ::Boolean	removeEventFromQueue = true;

  /* initialize the event for safety */
  bzf_event.keyDown.ascii = 0;

  bzf_event.type = (BzfEvent::Type)-1;

  status = ::ReceiveNextEvent(0, NULL, 0, removeEventFromQueue, &eventRef);
  if(status != ::noErr) {
    return false;
  }

  eventTime = ::GetEventTime(eventRef);
  eventClass = ::GetEventClass(eventRef);
  eventKind = ::GetEventKind(eventRef);

  // make note of any modifiers being pressed
  bzf_event.keyDown.shift = 0;
  status = GetEventParameter(eventRef,
			     ::kEventParamKeyModifiers,
			     ::typeUInt32,
			     NULL,
			     sizeof(::UInt32),
			     NULL,
			     &eventModifiers);
  if (eventModifiers & cmdKey) {
    // command and option both serve as bzflag alt even
    bzf_event.keyDown.shift = BzfKeyEvent::AltKey;
  }
  if (eventModifiers & shiftKey) bzf_event.keyDown.shift = BzfKeyEvent::ShiftKey;
  if (eventModifiers & optionKey) bzf_event.keyDown.shift = BzfKeyEvent::AltKey;
  if (eventModifiers & controlKey) bzf_event.keyDown.shift = BzfKeyEvent::ControlKey;

  switch(eventClass) {
  case ::kEventClassMouse:
    status = GetEventParameter(eventRef,
			       ::kEventParamWindowMouseLocation,
			       ::typeHIPoint,
			       NULL,
			       sizeof(::HIPoint),
			       NULL,
			       &eventLocation);

    status = GetEventParameter(eventRef,
			       ::kEventParamMouseButton,
			       ::typeMouseButton,
			       NULL,
			       sizeof(::EventMouseButton),
			       NULL,
			       &eventButtons);

    status = GetEventParameter(eventRef,
			       ::kEventParamMouseDelta,
			       ::typeHIPoint,
			       NULL,
			       sizeof(::HIPoint),
			       NULL,
			       &eventDelta);

    // handle the main event type
    switch(eventKind) {
    case ::kEventMouseDown:
    case ::kEventMouseUp:
      if(eventKind == ::kEventMouseDown) {
	bzf_event.type = BzfEvent::KeyDown;
      } else {
	bzf_event.type = BzfEvent::KeyUp;
      }
      if(eventButtons > 9) {
	// bzflag only handles 9 buttons for now
	eventButtons = 9;
      }

      switch(eventButtons) {
      case ::kEventMouseButtonSecondary:
	bzf_event.keyDown.button = BzfKeyEvent::RightMouse;
	break;
      case ::kEventMouseButtonTertiary:
	bzf_event.keyDown.button = BzfKeyEvent::MiddleMouse;
	break;
      default:
	/* consistent wth the rest of the mac experience, a command click is
	 * the same as a right click.
	 */
	if (bzf_event.keyDown.shift == BzfKeyEvent::AltKey) {
	  bzf_event.keyDown.shift = 0;
	  bzf_event.keyDown.button = BzfKeyEvent::RightMouse;
	} else {
	  bzf_event.keyDown.button = BzfKeyEvent::LeftMouse + eventButtons - 1;
	}
	break;
      }
      break;

    case ::kEventMouseMoved:
      bzf_event.type = BzfEvent::MouseMove;
      bzf_event.mouseMove.x = static_cast<int>(eventDelta.x);
      bzf_event.mouseMove.y = static_cast<int>(eventDelta.y);
      break;

    case ::kEventMouseWheelMoved:
      break;
    }
    break;

  case ::kEventClassKeyboard:
    status = GetEventParameter(eventRef,
			       ::kEventParamKeyMacCharCodes,
			       ::typeChar,
			       NULL,
			       sizeof(char),
			       NULL,
			       &eventChar);
    status = GetEventParameter(eventRef,
			       ::kEventParamKeyCode,
			       ::typeUInt32,
			       NULL,
			       sizeof(eventKeyCode),
			       NULL,
			       &eventKeyCode);
    switch(eventKind) {
    case ::kEventRawKeyDown:
    case ::kEventRawKeyRepeat:
      bzf_event.type = BzfEvent::KeyDown;
      getKey(bzf_event.keyDown, eventChar, eventKeyCode);
      break;

    case ::kEventRawKeyUp:
      bzf_event.type = BzfEvent::KeyUp;
      getKey(bzf_event.keyUp, eventChar, eventKeyCode);
      break;
    }
    break;

  case ::kEventClassApplication:
    switch(eventKind) {
    case ::kEventAppQuit:
      bzf_event.type = BzfEvent::Quit;
      break;
    }
    break;

  case ::kEventClassWindow:
    status = GetEventParameter(eventRef,
			       ::kEventParamDirectObject,
			       ::typeWindowRef,
			       NULL,
			       sizeof(::WindowRef),
			       NULL,
			       &eventWindow);
    switch(eventKind) {
    case ::kEventWindowUpdate:
    case ::kEventWindowDrawContent:
      bzf_event.type = BzfEvent::Redraw;
      BeginUpdate(eventWindow);
      EndUpdate(eventWindow);
      break;
    }
    break;

  case ::kEventClassCommand:
    switch(eventKind) {
    }
    break;
  }
  ReleaseEvent(eventRef);
  return true;
}

void MacDisplay::getKey (BzfKeyEvent &bzf_key, char char_code, ::UInt32 keycode) const {
  enum {
    kF1KeyCode	 = 0x7A,	// Undo
    kF2KeyCode	 = 0x78,	// Cut
    kF3KeyCode	 = 0x63,	// Copy
    kF4KeyCode	 = 0x76,	// Paste
    kF5KeyCode	 = 0x60,
    kF6KeyCode	 = 0x61,
    kF7KeyCode	 = 0x62,
    kF8KeyCode	 = 0x64,
    kF9KeyCode	 = 0x65,
    kF10KeyCode	 = 0x6D,
    kF11KeyCode	 = 0x67,
    kF12KeyCode	 = 0x6F,
    kF13KeyCode	 = 0x69,	// Print Screen
    kF14KeyCode	 = 0x6B,	// Scroll Lock
    kF15KeyCode	 = 0x71	// Pause
  };
  bzf_key.ascii = 0;
  bzf_key.button = BzfKeyEvent::NoButton;
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
  case kFunctionKeyCharCode:
    switch(keycode) {
      // These are the f-key codes on my apple extended keyboard
    case kF15KeyCode:	bzf_key.button = BzfKeyEvent::Pause; break;
    case kF12KeyCode:	bzf_key.button = BzfKeyEvent::F12;	break;
    case kF11KeyCode:	bzf_key.button = BzfKeyEvent::F11;	break;
    case kF10KeyCode:	bzf_key.button = BzfKeyEvent::F10;	break;
    case kF9KeyCode:	bzf_key.button = BzfKeyEvent::F9;	break;
    case kF8KeyCode:	bzf_key.button = BzfKeyEvent::F8;	break;
    case kF7KeyCode:	bzf_key.button = BzfKeyEvent::F7;	break;
    case kF6KeyCode:	bzf_key.button = BzfKeyEvent::F6;	break;
    case kF5KeyCode:	bzf_key.button = BzfKeyEvent::F5;	break;
    case kF4KeyCode:	bzf_key.button = BzfKeyEvent::F4;	break;
    case kF3KeyCode:	bzf_key.button = BzfKeyEvent::F3;	break;
    case kF2KeyCode:	bzf_key.button = BzfKeyEvent::F2;	break;
    case kF1KeyCode:	bzf_key.button = BzfKeyEvent::F1;	break;
    default:	fprintf(stderr, "Uknown function key code: 0x%lX\n", keycode);	break;
    }
    break;
    // standard key; a-z, 0-9 etc
  default:	bzf_key.ascii  = char_code;		break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
