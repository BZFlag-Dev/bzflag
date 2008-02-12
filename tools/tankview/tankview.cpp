// tankview.cpp : Defines the entry point for the application.
//

#include "tankview.h"
#include "Model.h"
#include "config.h"
#include "TextUtils.h"

// for the stupid debug openGLcount model uses
#ifdef DEBUG
int __beginendCount;
#endif

class Application : public SimpleDisplayEventCallbacks
{
public:
  Application();
  virtual void key ( int key, bool down, const ModiferKeys& mods );
  virtual void mouseButton( int key, int x, int y, bool down );
  virtual void mouseMoved( int x, int y );

  int run ( void );

protected:
  bool init ( void );
  void drawGridAndBounds ( void );
  void loadModels ( void );
  void drawModels ( void );

  void drawObjectNormals ( OBJModel &model );

  SimpleDisplay display;
  SimpleDisplayCamera *camera;

  OBJModel base,turret,barrel,lTread,rTread;
  unsigned int red,green,blue,purple,black,current;

  bool moveKeysDown[3];
  int mousePos[2];
  bool drawDebugNormals;
};

const char* convertPath ( const char* path )
{
  static std::string temp;
  if (!path)
    return NULL;

  temp = path;
#ifdef _WIN32
  temp = TextUtils::replace_all(temp,std::string("/"),std::string("\\"));
#endif

  return temp.c_str();
}

Application::Application() : display(800,600,false,"tankview")
{
  camera = NULL;
  drawDebugNormals = false;

  for (int i = 0; i<3; i++)
    moveKeysDown[i] = false;
}

void Application::drawGridAndBounds ( void )
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

  glColor4f(1,1,1,1);
}

void Application::loadModels ( void )
{
  barrel.read(convertPath("./data/geometry/tank/std/barrel.obj"));
  base.read(convertPath("./data/geometry/tank/std/body.obj"));
  turret.read(convertPath("./data/geometry/tank/std/turret.obj"));
  lTread.read(convertPath("./data/geometry/tank/std/lcasing.obj"));
  rTread.read(convertPath("/.data/geometry/tank/std/rcasing.obj"));

  red = display.loadImage(convertPath("./data/skins/red/tank.png"));
  green = display.loadImage(convertPath("./data/skins/green/tank.png"));
  blue = display.loadImage(convertPath("./data/skins/blue/tank.png"));
  purple = display.loadImage(convertPath("./data/skins/purple/tank.png"));
  black = display.loadImage(convertPath("./data/skins/rogue/tank.png"));

  current = green;
}

void Application::drawModels ( void )
{
  base.draw();
  lTread.draw();
  rTread.draw();
  turret.draw();
  barrel.draw();
}

void Application::drawObjectNormals ( OBJModel &model )
{
  glBegin(GL_LINES);

  for ( size_t f = 0; f < model.faces.size(); f++ )
  {
    for ( size_t v = 0; v < model.faces[f].verts.size(); v++ )
    {
      size_t vIndex = model.faces[f].verts[v];
      if ( vIndex < model.vertList.size() )
      {
	OBJVert vert = model.vertList[vIndex];

	size_t nIndex = model.faces[f].norms[v];
	if ( nIndex < model.normList.size() )
	{
	  vert.glVertex();
	  vert += model.normList[nIndex];
	  vert.glVertex();
	}
      }
    }
  }

  glEnd();
}


bool Application::init ( void )
{
  camera = new SimpleDisplayCamera;
  display.addEventCallback(this);

  // load up the models
  loadModels();

  camera->rotateGlob(-90,1,0,0);
  camera->moveLoc(0,0,-20);
  camera->moveLoc(0,1.5f,0);

  //setup light
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  float v[4] = {1};
  v[0] = v[1] = v[2] = 0.125f;
  glLightfv (GL_LIGHT0, GL_AMBIENT,v);
  v[0] = v[1] = v[2] = 0.75f;
  glLightfv (GL_LIGHT0, GL_DIFFUSE,v);
  v[0] = v[1] = v[2] = 0;
  glLightfv (GL_LIGHT0, GL_SPECULAR,v);

  return true;
}

int Application::run ( void )
{
  if (!init())
    return -1;

  while (display.update())
  {
    display.clear();
    camera->applyView();

    drawGridAndBounds();

    // draw normals

    if (drawDebugNormals)
    {
      glColor4f(1,0,0,1);
      drawObjectNormals(base);
      glColor4f(1,1,0,1);
      drawObjectNormals(barrel);
      glColor4f(0,1,1,1);
      drawObjectNormals(turret);
      glColor4f(0,0,1,1);
      drawObjectNormals(lTread);
      glColor4f(0,1,0,1);
      drawObjectNormals(lTread);  
    }


    glColor4f(1,1,1,1);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float v[4] = {1};
    v[0] = v[1] = v[2] = 20.f;
    glLightfv (GL_LIGHT0, GL_POSITION,v);

    display.bindImage(current);
    drawModels();

    camera->removeView();
    display.flip();
    //display.yeld(0.5f);
  }

  display.kill();

  return 0;
}

void Application::key ( int key, bool down, const ModiferKeys& mods )
{
  if (!camera || !down)
    return;

  switch(key)
  {
  case SD_KEY_ESCAPE:
    display.quit();
    break;

  case SD_KEY_UP:
    if (mods.alt)
      camera->rotateLoc(2.5f,1,0,0);
    else if (mods.ctl)
      camera->moveLoc(0,0,0.25f);
    else
      camera->moveLoc(0,0.25f,0);
    break;

  case SD_KEY_DOWN:
    if (mods.alt)
      camera->rotateLoc(-2.5f,1,0,0);
    else if (mods.ctl)
      camera->moveLoc(0,0,-0.25f);
    else
      camera->moveLoc(0,-0.25f,0);
    break;

  case SD_KEY_LEFT:
    if (mods.alt)
      camera->rotateLoc(2.5f,0,1,0);
    else
      camera->moveLoc(-0.25f,0,0);
    break;

  case SD_KEY_RIGHT:
    if (mods.alt)
      camera->rotateLoc(-2.5f,0,1,0);
    else
      camera->moveLoc(0.25f,0,0);
    break;

  case SD_KEY_F1:
    current = red;
    break;
  case SD_KEY_F2:
    current = green;
    break;
  case SD_KEY_F3:
    current = blue;
    break;
  case SD_KEY_F4:
    current = purple;
    break;
  case SD_KEY_F5:
    current = black;
    break;
  }
}

void Application::mouseButton( int key, int x, int y, bool down )
{
  mousePos[0] = x;
  mousePos[1] = y;

  moveKeysDown[key-1] = down;
}

void Application::mouseMoved( int x, int y )
{
  int mouseDelta[2];
  mouseDelta[0] = x - mousePos[0];
  mouseDelta[1] = y - mousePos[1];

  if ( moveKeysDown[0] )
  {
    camera->moveLoc(-mouseDelta[0]*0.0125f,-mouseDelta[1]*0.0125f,0);
  }
  else if ( moveKeysDown[1] )
    camera->moveLoc(0,0,mouseDelta[1]*0.025f);

  if ( moveKeysDown[2] )
  {
    camera->rotateLoc(mouseDelta[1]*0.025f,1,0,0);
    camera->rotateGlob(mouseDelta[0]*0.025f,0,0,-1);
 }

  mousePos[0] = x;
  mousePos[1] = y;
}


#ifdef _WIN32
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPInst, LPSTR lpCmdLine, int nShowCmd )
{
#else
int main ( int argc, char* argv[] )
{
#endif

  Application app;
  return app.run();
}




// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8