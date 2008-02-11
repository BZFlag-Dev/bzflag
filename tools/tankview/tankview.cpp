// tankview.cpp : Defines the entry point for the application.
//

#include "tankview.h"

#ifdef _WIN32
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPInst, LPSTR lpCmdLine, int nShowCmd )
{
#else
int main ( int argc, char* argv[] )
{
#endif
  SimpleDisplay display;

  while (display.update())
  {
    display.clear();

    display.flip();
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8