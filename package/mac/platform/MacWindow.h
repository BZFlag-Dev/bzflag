
#ifndef BZF_WINWINDOW_H
#define	BZF_WINWINDOW_H

// Use Apple's DrawSprockets to create a full-screen context,
// otherwise use a plain old window.
//#define USE_DSP

#include <CursorDevices.h>

#include <agl.h>

#include "BzfWindow.h"
#include "MacDisplay.h"
#include "MacVisual.h"

#ifdef USE_DSP
    #include "agl_drawsprocket.h"
#endif

class MacWindow : public BzfWindow {

  public:
			MacWindow(const MacDisplay*, MacVisual*);
			~MacWindow();

    boolean		isValid() const;

    void		showWindow(boolean);

    void		getPosition(int& x, int& y);
    void		getSize(int& width, int& height) const;

    void		setTitle(const char*);
    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int height);
    void		setFullscreen();

    void		warpMouse(int x, int y);
    void		getMouse(int& x, int& y) const;
    void		grabMouse();
    void		ungrabMouse();
    void		showMouse();
    void		hideMouse();

    void	setGamma(float);
    float	getGamma() const;
    boolean	hasGammaControl() const;

    void    makeCurrent();
    void    swapBuffers();
    void	makeContext();
    void	freeContext();

  private:
    Rect       rect;

#ifdef USE_DSP
	CGrafPtr  window;
#else
    WindowPtr  window;
#endif
    AGLContext context;

};

#endif // BZF_WINWINDOW_H
