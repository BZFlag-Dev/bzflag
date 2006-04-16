#include <sys/time.h>

#include "MacWindow.h"

#ifdef __cplusplus
#  define EXTERN_C_BEGIN extern "C" {
#  define EXTERN_C_END   }
#endif

#include <iostream>

EXTERN_C_BEGIN
extern WindowRef GetWindowRefFromNativeWindow(void * nativeWindow);
EXTERN_C_END

struct Settings
{
  bool Use_Main_Display;
  bool Switch_Display;
  bool Capture_Display;
  CGSize Window_Size;
  long Display_Hz;
  bool VBL_Synch;
  int depth;

  CGGammaValue redMin;
  CGGammaValue redMax;
  CGGammaValue redGamma;
  CGGammaValue greenMin;
  CGGammaValue greenMax;
  CGGammaValue greenGamma;
  CGGammaValue blueMin;
  CGGammaValue blueMax;
  CGGammaValue blueGamma;
  double gamma;

  Settings() {
    Use_Main_Display = true;
    Capture_Display = true;
    Switch_Display = true;
    Window_Size = CGSizeMake(CGDisplayPixelsWide(kCGDirectMainDisplay),
	CGDisplayPixelsHigh(kCGDirectMainDisplay));
    Display_Hz = 60;
    VBL_Synch = false; //CAPS-LOCK overrides
    depth = 24;
  }
} settings;

static double get_time()
{
  struct timeval tod;

  gettimeofday(&tod, NULL);

  return tod.tv_sec + tod.tv_usec * 1.0E-6;
}

#ifndef USE_DSP
  #define GLOBAL_OFFSET_X 0
  #define GLOBAL_OFFSET_Y 20
#else
  #define GLOBAL_OFFSET_X 0
  #define GLOBAL_OFFSET_Y 0
#endif

//Point gMousePosition;
bool gMouseGrabbed;

class GLContext
{
  const CGLPixelFormatAttribute* GetPixelFormat(u_int32_t display_id = 0x01,
      int color = settings.depth, int depth = 16, int stencil = 0) {
    static CGLPixelFormatAttribute attribs[32];

    CGLPixelFormatAttribute* attrib = attribs;

    *attrib++ = kCGLPFANoRecovery;
    *attrib++ = kCGLPFADoubleBuffer;
    *attrib++ = kCGLPFAAccelerated;
    *attrib++ = kCGLPFAFullScreen;
    *attrib++ = kCGLPFADisplayMask; *attrib++ = (CGLPixelFormatAttribute) display_id;
    *attrib++ = kCGLPFAColorSize; *attrib++ = (CGLPixelFormatAttribute) color;
    *attrib++ = kCGLPFADepthSize; *attrib++ = (CGLPixelFormatAttribute) depth;
    *attrib++ = kCGLPFAStencilSize; *attrib++ = (CGLPixelFormatAttribute) stencil;
    *attrib++ = (CGLPixelFormatAttribute) 0;

    return attribs;
  }

  public:

    CGRect bounds;
    CGLContextObj cgl_context;

    GLContext() {
      bounds = CGRectMake(0,0,0,0);
      cgl_context = NULL;
    }

    CGLContextObj GetGLContext(void) {
      return cgl_context;
    }

    bool Init(u_int32_t display_id, const CGRect& display_rect) {
      CGLPixelFormatObj pixel_format;
      long num_pixel_formats;

      CGLError err = CGLChoosePixelFormat(GetPixelFormat(display_id, 24),
	  &pixel_format, &num_pixel_formats);

      if (err || !pixel_format)
	return false;

      err = CGLCreateContext(pixel_format, NULL, &cgl_context);

      if (err)
	return false;

      CGLDestroyPixelFormat(pixel_format);
      pixel_format = NULL;

      err = CGLSetCurrentContext(cgl_context);

      if (err)
	return false;

      err = CGLSetFullScreen(cgl_context);

      if (err)
	return false;

      bounds = display_rect;

      if (settings.VBL_Synch)
	SetVBLSynch(true);

      // spit out OpenGL capabilities
      fprintf(stderr, "----------------------------------------------------------------\n");
      fprintf(stderr, "Vendor:     \"%s\"\n", glGetString(GL_VENDOR));
      fprintf(stderr, "Renderer:   \"%s\"\n", glGetString(GL_RENDERER));
      fprintf(stderr, "Version:    \"%s\"\n", glGetString(GL_VERSION));
      fprintf(stderr, "Extensions:\n");
      const GLubyte * extensions = glGetString(GL_EXTENSIONS);
      char * tmp = new char[strlen((const char *)extensions)+2];
      strcpy(tmp, (const char *)extensions);
      char * word;
      char * sep = " \t";
      for(word = strtok(tmp, sep); word != NULL; word = strtok(NULL, sep)) {
	fprintf(stderr, "\t%s\n", word);
      }
      delete [] tmp;
      tmp = NULL;
      fprintf(stderr, "----------------------------------------------------------------\n");
      return true;
    }

    void Reset() {
      if (cgl_context) {
	CGLSetCurrentContext(NULL);
	CGLClearDrawable(cgl_context);
	CGLDestroyContext(cgl_context);
	cgl_context = NULL;
      }
    }

    void SetPerspective(float angle, float near, float far) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      float aspect_ratio = bounds.size.width / bounds.size.height;

      gluPerspective(angle, aspect_ratio, near, far);
      glMatrixMode(GL_MODELVIEW);
    }

    void SetOrthographic() {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      GLint params[4];
      glGetIntegerv(GL_VIEWPORT, params);
      gluOrtho2D(0, params[2], params[3], 0);

      glMatrixMode(GL_MODELVIEW);
    }

    void SetVBLSynch(bool synch) {
      long params[] = { synch ? 1 : 0 };
      CGLSetParameter(cgl_context, kCGLCPSwapInterval, params);

      settings.VBL_Synch = synch;
    }

    void Flush() {
      // glFinish(); //CGLFlushDrawable says don't call glFinsh first
      if (cgl_context)
	CGLFlushDrawable(cgl_context);
    }

    void Process() {
      if (!cgl_context)
	return;

      static float t0 = get_time();

      float t = get_time() - t0;

      bool vbl_synch = false;

      if (settings.VBL_Synch != vbl_synch)
	SetVBLSynch(vbl_synch);

      float t1 = fabs(sin(t));

      glClearColor(0,0,0.1*t1,0);
      glClear(GL_COLOR_BUFFER_BIT);

      SetOrthographic();

      glPushMatrix();

      glTranslatef(bounds.size.width*0.5f-64.0,0,0);
      glTranslatef((bounds.size.width*0.5f-64.0-(1-0.435))*sin(t*3),0,0);

      glBegin(GL_QUADS);

      glColor3f(fabs(cos(t)),0,fabs(sin(t)));
      glVertex2f(0.0, 0.435);

      glColor3f(fabs(cos(t)),fabs(sin(t)),0);
      glVertex2f(0.0, bounds.size.height);

      glColor3f(0,fabs(cos(t)),fabs(sin(t)));
      glVertex2f(128.0, bounds.size.height);

      glColor3f(fabs(sin(t)),0,fabs(cos(t)));
      glVertex2f(128.0, 0.435);

      glEnd();

      GLfloat frame[4] = { 1,1,1,1};
      glColor4fv(frame);

      glBegin(GL_LINE_LOOP);

      glVertex2f(0.0, 0.435);

      glVertex2f(0.0, bounds.size.height);

      glVertex2f(128.0, bounds.size.height);

      glVertex2f(128.0, 0.435);

      glEnd();

      glPopMatrix();

      //glFinish();

      CGLFlushDrawable(cgl_context);
    }
} gl_context;

long get_value(CFDictionaryRef values, CFStringRef key)
{
  CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(values, key);

  if (!number_value)
    return -1;

  long int_value;

  if (!CFNumberGetValue(number_value, kCFNumberLongType, &int_value))
    return -1;

  return int_value;
}

class DirectDisplay
{
  CGDisplayCount num_displays;
  CGDirectDisplayID* display_ids;

  void DumpDisplayModeValues(CFDictionaryRef values) {
  /*
    dprintf("   ----- Display Mode Info for %d -----\n", get_value(values, kCGDisplayMode));
    dprintf("   Bounds = %d x %d\n", get_value(values, kCGDisplayWidth), get_value(values, kCGDisplayHeight));
    dprintf("   bpp = %d, hz = %d\n", get_value(values, kCGDisplayBitsPerPixel), get_value(values,  kCGDisplayRefreshRate));
  */
  }

public:

  DirectDisplay() {
    num_displays = 0;
    display_ids = 0;
  }

  int Init() {
     if (CGGetActiveDisplayList(0, NULL, &num_displays) != CGDisplayNoErr || num_displays == 0)
      return 0;

    display_ids = new CGDirectDisplayID[num_displays];

    if (CGGetActiveDisplayList(num_displays, display_ids, &num_displays) != CGDisplayNoErr)
      return false;

    return num_displays;
  }

  CGRect GetDisplayBounds(CGDisplayCount display_num) {
    if (display_num >= num_displays)
      return CGRectMake(0,0,0,0);

    return CGDisplayBounds(display_ids[display_num]);
  }

  u_int32_t GetOpenGLDisplayID(CGDisplayCount display_num) {
    if (display_num >= num_displays)
      return 0;

    return CGDisplayIDToOpenGLDisplayMask(display_ids[display_num]);
  }

  CGDirectDisplayID GetDirectDisplayID(CGDisplayCount display_num) {
    if (display_num >= num_displays)
      return 0;

    return display_ids[display_num];
  }

  void DumpCurrentDisplayMode(CGDisplayCount display_num) {
    if (display_num >= num_displays)
      return;

    CFDictionaryRef display_mode_values = CGDisplayCurrentMode(display_ids[display_num]);

    DumpDisplayModeValues(display_mode_values);
  }

  void DumpDisplayModes(CGDisplayCount display_num) {
    if (display_num >= num_displays)
      return;

//    CFArrayRef display_modes = CGDisplayAvailableModes(display_ids[display_num]);

//    CFIndex num_modes = CFArrayGetCount(display_modes);
  }

  bool SetDisplayMode(CGDisplayCount display_num, const CGSize& size, size_t bpp, CGRefreshRate hz) {
    if (display_num >= num_displays)
      return false;

    CFDictionaryRef display_mode_values = CGDisplayBestModeForParametersAndRefreshRate(display_ids[display_num], bpp, (size_t) size.width, (size_t) size.height, hz, NULL);

//    int display_hz = get_value(display_mode_values,  kCGDisplayRefreshRate);

    CGDisplayErr err = CGDisplaySwitchToMode(display_ids[display_num], display_mode_values);

    return err == CGDisplayNoErr;
  }

  bool Capture(CGDisplayCount display_num) {
    if (display_num >= num_displays || CGDisplayIsCaptured(display_ids[display_num]))
      return false;

    return CGDisplayCapture(display_ids[display_num]) == CGDisplayNoErr;
  }

  bool Release(CGDisplayCount display_num = 0) {
    if (display_num >= num_displays || !CGDisplayIsCaptured(display_ids[display_num]))
      return false;

    return CGDisplayRelease(display_ids[display_num]) == CGDisplayNoErr;
  }
} displays;

void Display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

MacWindow::MacWindow(const MacDisplay *display, MacVisual *visual) :
			BzfWindow(display)
{
//  GLboolean ok;
//  int argc = 0;
//  char **argv = {NULL};

  /* set the size of the window to the default (existing resolution). this
   * includes initializing the opengl context to the correct aspect ratio.
   */
  setSize((int)settings.Window_Size.width, (int)settings.Window_Size.height);

  display->setContext(gl_context.GetGLContext());

#ifdef USE_DSP
  display->setWindow(window);
#endif

  visual->reset();
  visual->setLevel(0);
  visual->setIndex(24);
  //visual->setDepth(24);
  visual->setDoubleBuffer(true);
  visual->setRGBA(1, 1, 1, 0);
  //visual->setStencil(1);

  //visual->addAttribute1(AGL_FULLSCREEN);

  visual->build();

  makeCurrent();

  CGGetDisplayTransferByFormula(kCGDirectMainDisplay,
			       &settings.redMin, &settings.redMax, &settings.redGamma,
			       &settings.greenMin, &settings.greenMax, &settings.greenGamma,
			       &settings.blueMin, &settings.blueMax, &settings.blueGamma);
  settings.gamma = (settings.redGamma + settings.greenGamma + settings.blueGamma) / 3.0;

#ifdef DEBUG
  std::cout << "Initial gamma settings: " << settings.gamma << " for (" << settings.redGamma << "," << settings.greenGamma << "," <<settings.blueGamma << std::endl;
#endif

  //hideMouse();
}

MacWindow::~MacWindow() {
  if (window != NULL)
    DisposeWindow(window);

  showMouse();
  gl_context.Reset();
  displays.Release();
  CGDisplayRestoreColorSyncSettings();
}

bool MacWindow::isValid() const { return true; }

void MacWindow::showWindow(bool) {}

void MacWindow::getPosition(int &x, int &y) { x = 0, y = 0; }

void MacWindow::getSize(int &width, int &height) const {

  width = CGDisplayPixelsWide(kCGDirectMainDisplay);
  height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
  // width = settings.Window_Size.width;
  // width = settings.Window_Size.height;
}

void MacWindow::setTitle(const char *) {}
void MacWindow::setPosition(int, int) {}
void MacWindow::setSize(int width, int height)
{
  settings.Window_Size.width = width;
  settings.Window_Size.height = height;

  std::cout << "setSize was called with " << width << " x " << height << std::endl;

  int num_displays = displays.Init();
  if (num_displays == 0) {
    return;
  }
  int display_index = settings.Use_Main_Display ? 0 : num_displays-1;
  u_int32_t display_id = displays.GetOpenGLDisplayID(display_index);

  if (settings.Capture_Display) {
    displays.Capture(display_index);
  }

  if (settings.Switch_Display) {
    if (displays.SetDisplayMode(display_index, settings.Window_Size, settings.depth, settings.Display_Hz)) {
      displays.DumpCurrentDisplayMode(display_index);
    }
  }

  CGRect window_rect = CGRectMake(0,0, settings.Window_Size.width, settings.Window_Size.height);
  if (!settings.Switch_Display) {
    window_rect.origin = CGPointMake(32,32);
  }

  if (!gl_context.Init(display_id, window_rect)) {
    return;
  }

}

void MacWindow::setMinSize(int, int) {
#ifndef USE_DSP
  if (window == NULL) return;

  // Not sure what this wants, leave out for now.
  //CollapseWindow(window, true);
#endif
}

void MacWindow::setFullscreen() {} // do nothing for now
void MacWindow::warpMouse(int x, int y) {
  CGDisplayErr cgErr;
  CGPoint  mp;

  mp = CGPointMake(x,y);
  cgErr = CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, mp);
}

void MacWindow::getMouse(int &x, int &y) const {
  Point   tmpMouse;
  GrafPtr savedPort;

  GetPort(&savedPort);
  //SetPort(window);
  GetMouse(&tmpMouse);  // returns mouse location in coords of local
			  // window which must be set
  x = tmpMouse.h;
  y = tmpMouse.v;
  SetPort(savedPort);
}

void MacWindow::grabMouse()
{
	// CGAssociateMouseAndMouseCursorPosition() does not have the
	// expected effect. Comment out for now.
//	CGAssociateMouseAndMouseCursorPosition(false);
	gMouseGrabbed = true;
}
void MacWindow::ungrabMouse()
{
	// CGAssociateMouseAndMouseCursorPosition() does not have the
	// expected effect. Comment out for now.
//	CGAssociateMouseAndMouseCursorPosition(true);
	gMouseGrabbed = false;
}
void MacWindow::showMouse() { ShowCursor(); }
void MacWindow::hideMouse() { HideCursor(); }

void MacWindow::setGamma(float value)
{
  CGDisplayErr err;

  settings.gamma = value;

#ifdef DEBUG
  std::cout << "Setting Gamma to " << value << std::endl;
#endif
  err = CGSetDisplayTransferByFormula( kCGDirectMainDisplay,
				       settings.redMin, settings.redMax, 1.0 / value, //red
				       settings.greenMin, settings.greenMax, 1.0 / value, //green
				       settings.blueMin, settings.blueMax, 1.0 / value); //blue
}
float MacWindow::getGamma()	const { return settings.gamma;   }
bool MacWindow::hasGammaControl() const { return true; }

void MacWindow::makeContext() {}
void MacWindow::freeContext() {}
void MacWindow::makeCurrent() {}

void MacWindow::swapBuffers() {
  gl_context.Flush();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
