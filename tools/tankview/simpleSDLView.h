#ifndef _SIMPLE_SDL_VIEW_H_
#define _SIMPLE_SDL_VIEW_H_

#include <SDL/SDL.h>

#ifdef _WIN32 // this file only has windows stuff
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <time.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "sdl.lib")

#else
#include <unistd.h>
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <AGL/gl.h>
#include <AGL/glu.h>
#else	// linux
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif // _WIN32

#include <vector>

class ModiferKeys
{
public:
  bool alt;
  bool ctl;
  bool shift;
  bool meta;

  ModiferKeys(bool a = false, bool c = false, bool s = false, bool m = false )
  {
    alt = a;
    ctl = c;
    shift = s;
    meta = m;
  }

  bool operator == ( const ModiferKeys& r ) const
  {
    return r.alt == alt && r.ctl == ctl && r.shift == shift && r.meta == meta;
  }
};

class SimpleDisplayEventCallbacks
{
public:
  virtual ~SimpleDisplayEventCallbacks(){};

  virtual void activate ( void ){};
  virtual void deactivate ( void ){};
  virtual void focus ( bool lost ){};
  virtual void resize ( size_t x, size_t y ){};
  virtual void key ( int key, bool down, const ModiferKeys& mods ){};

};

class SimpleDisplay
{
public:
  SimpleDisplay ( size_t width = 800, size_t height = 600, bool full = false, const char* caption = NULL );
  ~SimpleDisplay ();

  bool create ( size_t width = 800, size_t height = 600, bool full = false, const char* caption = NULL );
  void kill ( void );

  void clear ( void );
  void flip ( void );

  float getFOV ( void ) { return fov; }
  void setFOV ( float f ) { fov = f; }

  void getBacgroundColor ( float &r, float &g, float &b );
  void setBacgroundColor ( float r, float g, float b );

  void getDesktopRes ( size_t &x, size_t &y );
  void getCurrentRes ( size_t &x, size_t &y );

  bool update ( void );
  void yeld ( float time = 0.01f );

  void addEventCallback ( SimpleDisplayEventCallbacks *callback );
  void removeEventCallback ( SimpleDisplayEventCallbacks *callback );

protected:
  void initGL ( void );
  void setViewport ( void );
  
  size_t size[2];
  float aspect;
  float nearZ, farZ;
  float fov;
  float clearColor[3];
  bool fullscreen;
  bool valid;

  std::vector<SimpleDisplayEventCallbacks*> callbacks;

  void activate ( void );
  void deactivate ( void );
  void focus ( bool lost );
  void resize ( size_t x, size_t y );
  void key ( int key, bool down, const ModiferKeys& mods );

};


#endif //_SIMPLE_SDL_VIEW_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8