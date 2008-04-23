#ifndef BZF_WINWINDOW_H
#define  BZF_WINWINDOW_H

#include <Carbon/Carbon.h>

#include "bzfgl.h"

#include "BzfWindow.h"
#include "MacDisplay.h"
#include "MacVisual.h"

class MacWindow : public BzfWindow {
  public:
    MacWindow(const MacDisplay*, MacVisual*);
    ~MacWindow();

    bool isValid() const;

    void showWindow(bool);

    void getPosition(int& x, int& y);
    void getSize(int& width, int& height) const;

    void setTitle(const char*);
    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setMinSize(int width, int height);
    void setFullscreen(bool);

    void warpMouse(int x, int y);
    void getMouse(int& x, int& y) const;
    void grabMouse();
    void ungrabMouse();
    void showMouse();
    void hideMouse();

    void setGamma(float);
    float getGamma() const;
    bool hasGammaControl() const;

    void makeCurrent();
    void swapBuffers();
    void makeContext();
    void freeContext();

  private:
    Rect rect;

    WindowRef window;
    CGLContextObj context;
};

#endif // BZF_WINWINDOW_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
