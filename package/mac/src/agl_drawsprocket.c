#include <LowMem.h>
#include <DrawSprocket.h>
#include <agl.h>

#include "agl_drawsprocket.h"
/* globals */
static DSpContextAttributes gDSpTheContextAttributes;
static DSpContextReference  gDSpTheContext;

CGrafPtr dsSetupScreen( int width, int height )
{
	OSStatus theError;

	CGrafPtr theFrontBuffer;

	/* Get the bit depth of the screen */
	PixMapHandle pixMapHdl;
	GDHandle deviceHdl;
	int scrn_depth;

	deviceHdl  = LMGetMainDevice();
	pixMapHdl  = (**deviceHdl).gdPMap;
	scrn_depth = (**pixMapHdl).pixelSize;

	switch ( scrn_depth ) {

		case 8:
		gDSpTheContextAttributes.displayDepthMask	= kDSpDepthMask_8;
		gDSpTheContextAttributes.displayBestDepth	= 8;
		gDSpTheContextAttributes.backBufferDepthMask	= kDSpDepthMask_8;
		gDSpTheContextAttributes.backBufferBestDepth	= 8;
		break;

		case 16:
		gDSpTheContextAttributes.displayDepthMask	= kDSpDepthMask_16;
		gDSpTheContextAttributes.displayBestDepth	= 16;
		gDSpTheContextAttributes.backBufferDepthMask	= kDSpDepthMask_16;
		gDSpTheContextAttributes.backBufferBestDepth	= 16;
		break;

		case 32:
		gDSpTheContextAttributes.displayDepthMask	= kDSpDepthMask_32;
		gDSpTheContextAttributes.displayBestDepth	= 32;
		gDSpTheContextAttributes.backBufferDepthMask	= kDSpDepthMask_32;
		gDSpTheContextAttributes.backBufferBestDepth	= 32;
		break;

		default:
		DebugStr("\pInvalid bit depth\n");

		ExitToShell();

	}

	theError = DSpStartup();
	if( theError )
		DebugStr("\pUnable to startup\n");

	gDSpTheContextAttributes.frequency					= 0;
	gDSpTheContextAttributes.reserved1					= 0;
	gDSpTheContextAttributes.reserved2					= 0;
	gDSpTheContextAttributes.colorTable				    = NULL;
	gDSpTheContextAttributes.contextOptions			= 0;
	gDSpTheContextAttributes.gameMustConfirmSwitch	    = false;
	gDSpTheContextAttributes.reserved3[0]				= 0;
	gDSpTheContextAttributes.reserved3[1]				= 0;
	gDSpTheContextAttributes.reserved3[2]				= 0;
	gDSpTheContextAttributes.reserved3[3]				= 0;
	gDSpTheContextAttributes.displayWidth			    = width;
	gDSpTheContextAttributes.displayHeight			= height;
	gDSpTheContextAttributes.colorNeeds				= kDSpColorNeeds_Require;
	gDSpTheContextAttributes.pageCount		  = 1;

	theError = DSpFindBestContext( &gDSpTheContextAttributes, &gDSpTheContext );
	if( theError )
		DebugStr("\pUnable to find a suitable device\n");

	theError = DSpContext_Reserve( gDSpTheContext, &gDSpTheContextAttributes );
	if( theError )
		DebugStr("\pUnable to create the display!");

	theError = DSpContext_FadeGammaOut( NULL, NULL );
	if( theError )
		DebugStr("\pUnable to fade the display!");

	theError = DSpContext_SetState( gDSpTheContext, kDSpContextState_Active );
	if( theError )
		DebugStr("\pUnable to set the display!");

	/* // Not supported in 1.1.4?
	theError = DSpContext_SetScale (gDSpTheContext, kDSpBufferScale_2Interpolate );
	if( theError )
		DebugStr("\pUnable to set pixel doubling");
	*/
	theError = DSpContext_FadeGammaIn( NULL, NULL );
	if( theError )
		DebugStr("\pUnable to fade the display!");

	{
		/* create a window to draw into */
		Rect r;
		AuxWinHandle awh;
		r.top = r.left = 0;
		DSpContext_LocalToGlobal(gDSpTheContext, (Point *)&r);
		r.right = r.left + width;
		r.bottom = r.top + height;

		theFrontBuffer = (CGrafPtr)NewCWindow (NULL, &r, "\p", 0, plainDBox, (WindowPtr)-1, 0, 0);

		/* set the content color of the window to black to avoid a white flash when the window appears */
		if(GetAuxWin ((WindowPtr)theFrontBuffer, &awh)){
			CTabHandle theColorTable;
			OSErr err;

			/*****/

			theColorTable = (**awh).awCTable;
			err = HandToHand((Handle*)&theColorTable);
			if(err)
				DebugStr("\pOut of memory!");

			(**theColorTable).ctTable[wContentColor].rgb.red = 0;
			(**theColorTable).ctTable[wContentColor].rgb.green = 0;
			(**theColorTable).ctTable[wContentColor].rgb.blue = 0;

			CTabChanged(theColorTable);

			/* the color table will be disposed by the window manager when the window is disposed */
			SetWinColor((WindowPtr)theFrontBuffer, (WCTabHandle)theColorTable);

		}
		ShowWindow((GrafPtr)theFrontBuffer);
		SetPort((GrafPtr)theFrontBuffer);

		{
			RGBColor b = { 0xFFFF, 0xFFFF, 0xFFFF };
			RGBColor f = { 0x0000, 0x0000, 0x0000 };

			RGBForeColor(&f);
			RGBBackColor(&b);
		}
	}
	return theFrontBuffer;
}

void dsShutdownScreen(CGrafPtr theFrontBuffer)
{
	DSpContext_FadeGammaOut( NULL, NULL );
	DisposeWindow((WindowPtr)theFrontBuffer);
	DSpContext_SetState( gDSpTheContext, kDSpContextState_Inactive );
	DSpContext_FadeGammaIn( NULL, NULL );
	DSpContext_Release( gDSpTheContext );
	DSpShutdown();
}

/*
** OpenGL Setup
*/
AGLContext dsSetupAGL(AGLDrawable win)
{
	GLint	  attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_NONE };
	AGLPixelFormat fmt;
	AGLContext     ctx;
	GLboolean      ok;

	/* Choose an rgb pixel format */
	fmt = aglChoosePixelFormat(NULL, 0, attrib);
	if(fmt == NULL) return NULL;

	/* Create an AGL context */
	ctx = aglCreateContext(fmt, NULL);
	if(ctx == NULL) return NULL;

	/* Attach the window to the context */
	ok = aglSetDrawable(ctx, win);
	if(!ok) return NULL;

	/* Make the context the current context */
	ok = aglSetCurrentContext(ctx);
	if(!ok) return NULL;

	/* Pixel format is no longer needed */
	aglDestroyPixelFormat(fmt);

	return ctx;
}

/*
** OpenGL Cleanup
*/
static void cleanupAGL(AGLContext ctx)
{
	aglSetCurrentContext(NULL);
	aglSetDrawable(ctx, NULL);
	aglDestroyContext(ctx);
}
