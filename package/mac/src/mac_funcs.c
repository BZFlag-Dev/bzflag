#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <SegLoad.h>
#include <Sound.h>
#include <Timer.h>

#include <SIOUX.h>
#include <stdlib.h>
#include <stdio.h>
#include <console.h>

/* From GUSI */
#include <signal.h>

/* From MSL */
#include <path2fss.h>

#include "mac_funcs.h"
#include "FinderLaunch.h" /* borrowed from Apple */

#ifdef main
#undef main
#endif

#define SERVER_PATH "::bzfs:bzfs"

extern int  bzf_main (int, char**);
extern void MacInitGUSIHook ();

void MacInitAppleEvents ();
void MacInitSIOUXSettings ();
void MacInitToolbox ();
void MacWaitForArgs ();


bool gInBackground     = false;
bool gSleepTime	= 60;
RgnHandle   gCursorRgn = 0;

AEEventHandlerUPP gOpenDocsUPP;
AEEventHandlerUPP gQuitUPP;

int    argc;
char **argv;

pascal OSErr MacAEOpenDocs (AppleEvent *event, AppleEvent *reply, SInt32 handlerRef) {

  return noErr;

}
pascal OSErr MacAEQuit (AppleEvent *event, AppleEvent *reply, SInt32 handlerRef) {

  raise (SIGQUIT);

  return noErr;
}




/* Tell finder to open server,  send args apple event */
void MacLaunchServer (int argc, const char **argv) {

/* AppleEvent msg, reply; */
/*  AEDescList descList; */
/*  AEAddressDesc address; */
/*  TargetID targetID; */
/*  LaunchParamBlockRec launchRecord; */

  FSSpec server;
  int i;

  printf ("server args\n");

  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);

  printf ("\n");

  __path2fss(SERVER_PATH, &server);
  FinderLaunch(1, &server);


}

/* Wait for an apple event to send command-line args */
void MacWaitForArgs () {

  int wait = 0;
  UnsignedWide t1, t0;

  Microseconds (&t0);

  /* Wait for 1 second */
  while (wait < 1) {

    MacOneEvent ();

    Microseconds (&t1);
    wait = (t1.lo - t0.lo) / 1000000;
  }
}

void MacInitAppleEvents () {

  OSErr	osErr;

  gOpenDocsUPP  = NewAEEventHandlerProc ((ProcPtr) MacAEOpenDocs);
  gQuitUPP      = NewAEEventHandlerProc ((ProcPtr) MacAEQuit);

	osErr = AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,  gOpenDocsUPP, 0L,false);

	osErr = AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,  gQuitUPP, 0L,false);

}

void MacInitSIOUXSettings () {

  SIOUXSettings.toppixel  = 41;
  SIOUXSettings.leftpixel = 645;
  SIOUXSettings.columns   = 57;
  SIOUXSettings.rows      = 40;
  SIOUXSetTitle ("\pDebug output");
}

void MacInitToolbox ()
{
	MaxApplZone ();
	MoreMasters ();
	MoreMasters ();

	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
}

void MacOneEvent () {

  EventRecord eventRec;
  int	 gotEvent;

  if (!gCursorRgn)
    gCursorRgn = NewRgn();

  gotEvent = WaitNextEvent(everyEvent,&eventRec,gSleepTime,gCursorRgn);
  if (gotEvent) {

    switch (eventRec.what) {

      case updateEvt:
			BeginUpdate((WindowPtr)eventRec.message);
	EndUpdate((WindowPtr)eventRec.message);
		  break;

		case osEvt:

		  switch( (eventRec.message >> 24) & 0x000000FF) {

		    case suspendResumeMessage:
				     gInBackground = (eventRec.message & resumeFlag) != 1;
				     gSleepTime = gInBackground ? MAC_BG_SLEEP : MAC_FG_SLEEP;
			    break;
		}
	break;

      case kHighLevelEvent:
	AEProcessAppleEvent(&eventRec);
	break;
    }

    SIOUXHandleOneEvent (&eventRec);
  }
}




int main () {

  MacInitToolbox ();
  MacInitAppleEvents ();
  MacWaitForArgs ();



  MacInitSIOUXSettings ();
  MacInitGUSIHook ();


  argc = ccommand (&argv);



/*  atexit (MyExit); */


  bzf_main (argc, argv);

  return 0;
}
