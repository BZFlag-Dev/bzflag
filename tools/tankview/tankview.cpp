// tankview.cpp : Defines the entry point for the application.
//

#include "tankview.h"
#include "Model.h"
#include "config.h"

// for the stupid debug openGLcount model uses
#ifdef DEBUG
int __beginendCount;
#endif

void drawGridAndBounds ( void )
{
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  glBegin(GL_LINES);
  // axis markers
  glColor4f(1,0,0,0.25f);
  glVertex3f(0,0,0);
  glVertex3f(1,0,0);

  glColor4f(0,1,0,0.25f);
  glVertex3f(0,0,0);
  glVertex3f(0,1,0);

  glColor4f(0,0,1,0.25f);
  glVertex3f(0,0,0);
  glVertex3f(0,0,1);

  // grid
  glColor4f(1,1,1,0.125f);
  float grid = 10.0f;
  float increment = 0.25f;

  for( float i = -grid; i <= grid; i += increment )
  {
    glVertex3f(i,grid,0);
    glVertex3f(i,-grid,0);
    glVertex3f(grid,i,0);
    glVertex3f(-grid,i,0);
  }

  // tank bbox
  glColor4f(0,1,1,0.25f);
  float width = 1.4f;
  float height = 2.05f;
  float front = 4.94f;
  float back = -3.10f;

  // front
  glVertex3f(front,-width,0);
  glVertex3f(front,width,0);
  glVertex3f(front,width,0);
  glVertex3f(front,width,height);
  glVertex3f(front,-width,height);
  glVertex3f(front,width,height);
  glVertex3f(front,-width,0);
  glVertex3f(front,-width,height);

  // back
  glVertex3f(back,-width,0);
  glVertex3f(back,width,0);
  glVertex3f(back,width,0);
  glVertex3f(back,width,height);
  glVertex3f(back,-width,height);
  glVertex3f(back,width,height);
  glVertex3f(back,-width,0);
  glVertex3f(back,-width,height);

  // sides
  glVertex3f(back,-width,0);
  glVertex3f(front,-width,0);

  glVertex3f(back,width,0);
  glVertex3f(front,width,0);

  glVertex3f(back,-width,height);
  glVertex3f(front,-width,height);

  glVertex3f(back,width,height);
  glVertex3f(front,width,height);
  glEnd();
}

OBJModel base,turret,barrel,lTread,rTread;

void loadModels ( void )
{
  barrel.read(std::string("/data/geometry/tank/std/barrel.obj"));
  base.read(std::string("/data/geometry/tank/std/base.obj"));
  turret.read(std::string("/data/geometry/tank/std/turret.obj"));
  lTread.read(std::string("/data/geometry/tank/std/lcasing.obj"));
  rTread.read(std::string("/data/geometry/tank/std/rcasing.obj"));
}

void drawModels ( void )
{
  base.draw();
  lTread.draw();
  rTread.draw();
  turret.draw();
  barrel.draw();
}

#ifdef _WIN32
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPInst, LPSTR lpCmdLine, int nShowCmd )
{
#else
int main ( int argc, char* argv[] )
{
#endif
  SimpleDisplay display;

  SimpleDisplayCamera camera;

  // load up the models
  loadModels();

  camera.rotateGlob(-90,1,0,0);
  camera.moveLoc(0,0,-20);
  camera.moveLoc(0,1.5f,0);

  while (display.update())
  {
    display.clear();
    camera.applyView();

    // bind a texture

    drawGridAndBounds();
    drawModels();

    camera.removeView();
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