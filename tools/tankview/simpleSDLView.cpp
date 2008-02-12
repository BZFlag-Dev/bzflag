#include "simpleSDLView.h"

SimpleDisplayCamera::SimpleDisplayCamera( float x, float y, float z )
{
  memset(matrix, 0, 16*sizeof(float));
  matrix[0] = 1.0f;
  matrix[5] = 1.0f;
  matrix[10] = -1.0f;
  matrix[15] = 1.0f;
  matrix[12] = x; matrix[13] = y; matrix[14] = z;

  right = &matrix[0];
  up = &matrix[4];
  forward = &matrix[8];
  pos = &matrix[12];

  viewport[0] = viewport[1] = viewport[2] = viewport[3] = 0.0f;
  setOrthoViewport();
  setOrthoHitherYon(0.001f,1000.0f);
}

void SimpleDisplayCamera::setOrthoViewport ( void )
{
  glGetFloatv(GL_VIEWPORT,viewport);
}

void SimpleDisplayCamera::setOrthoHitherYon ( float hither, float yon )
{
  hitherYon[0] = hither;
  hitherYon[1] = yon;
}

SimpleDisplayCamera::~SimpleDisplayCamera()
{
}

void SimpleDisplayCamera::applyView()
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  float viewmatrix[16]={//Remove the three - for non-inverted z-axis
    matrix[0], matrix[4], -matrix[8], 0,
    matrix[1], matrix[5], -matrix[9], 0,
    matrix[2], matrix[6], -matrix[10], 0,

    -(matrix[0]*matrix[12] +
    matrix[1]*matrix[13] +
    matrix[2]*matrix[14]),

    -(matrix[4]*matrix[12] +
    matrix[5]*matrix[13] +
    matrix[6]*matrix[14]),

    //add a - like above for non-inverted z-axis
    (matrix[8]*matrix[12] +
    matrix[9]*matrix[13] +
    matrix[10]*matrix[14]), 1};
    glLoadMatrixf(viewmatrix);
}

void SimpleDisplayCamera::removeView ( void )
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
}

void SimpleDisplayCamera::applyOrtho ( void )
{
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
  glOrtho(0.0,(double)viewport[2],0.0,(double)viewport[3],hitherYon[0],hitherYon[1]);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_LIGHTING);
}

void SimpleDisplayCamera::removeOrtho ( void )
{
  glEnable(GL_LIGHTING);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode (GL_PROJECTION);
  glPopMatrix();

  glMatrixMode (GL_PROJECTION);
}

void SimpleDisplayCamera::moveLoc (float x, float y, float z, float distance ) 
{
  float dx=x*matrix[0] + y*matrix[4] + z*matrix[8];
  float dy=x*matrix[1] + y*matrix[5] + z*matrix[9];
  float dz=x*matrix[2] + y*matrix[6] + z*matrix[10];
  matrix[12] += dx * distance;
  matrix[13] += dy * distance;
  matrix[14] += dz * distance;
}

void SimpleDisplayCamera::moveGlob ( float x, float y, float z, float distance )
{
  matrix[12] += x * distance;
  matrix[13] += y * distance;
  matrix[14] += z * distance;
}

void SimpleDisplayCamera::rotateLoc ( float deg, float x, float y, float z )
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf(matrix);
  glRotatef(deg, x,y,z);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();
}


void SimpleDisplayCamera::rotateGlob ( float deg, float x, float y, float z ) 
{
  float dx=x*matrix[0] + y*matrix[1] + z*matrix[2];
  float dy=x*matrix[4] + y*matrix[5] + z*matrix[6];
  float dz=x*matrix[8] + y*matrix[9] + z*matrix[10];

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf(matrix);
  glRotatef(deg, dx,dy,dz);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();
}

//--------------------SimpleDisplay------------------------------//

#define _INVALID_ID 0xFFFFFFFF

SimpleDisplay::SimpleDisplay ( size_t width , size_t height, bool full, const char* caption )
{
  size[0] = size[1] = 0;
  aspect = 0;
  nearZ = 0.0001f;
  farZ = 1000.0f;
  fov = 30;
  fullscreen = false;
  valid = false;

  create(width,height,full,caption);

  lastImageID = 0;
}

SimpleDisplay::~SimpleDisplay ()
{
  kill();
}

bool SimpleDisplay::create ( size_t width, size_t height, bool full, const char* caption )
{
  if ( width == 0 || height == 0 )
    return false;

  kill();

  run = true;
  size[0] = width;
  size[1] = height;
  fullscreen = full;

  if (!SDL_WasInit(SDL_INIT_VIDEO))
  {
    if ( SDL_InitSubSystem(SDL_INIT_VIDEO ) < 0 )
      return false;
  }

  Uint32 SDLflags = SDL_OPENGL;

  if ( fullscreen )
    SDLflags |= SDL_FULLSCREEN;

  SDL_Surface	*surface = SDL_SetVideoMode((int)width, (int)height, 0, SDLflags);
  if ( surface == NULL )
    return false;

  if ( !fullscreen  && caption && strlen(caption)>0 )
    SDL_WM_SetCaption(caption,NULL);

  initGL();

  return valid;
}

void SimpleDisplay::kill ( void )
{
  imageNameMap.clear();
  std::map<unsigned int,LoadedImage>::iterator itr = images.begin();

  while (itr != images.end() )
  {
    //clear the image
    if (itr->second.boundID != _INVALID_ID)
      glDeleteTextures(1,&itr->second.boundID);

    itr++;
  }
  images.clear();

  lastImageID = 0;

  callbacks.clear();

  if ( valid )
  {
    valid = false;
    if (!SDL_WasInit(SDL_INIT_VIDEO))
      return;

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }

  run = false;
}

unsigned int  SimpleDisplay::loadImage ( const char* file )
{
  if (!file)
    return _INVALID_ID;

  std::string path = file;
  if (imageNameMap.find(path) != imageNameMap.end())
    return imageNameMap[path];

  LoadedImage image;
  image.boundID = _INVALID_ID;
  image.name = path;
  image.id = lastImageID++;

  imageNameMap[path] = image.id;
  images[image.id] = image;

  return image.id;
}

void  SimpleDisplay::unloadImage ( unsigned int imageID )
{
  std::map<unsigned int,LoadedImage>::iterator itr = images.find(imageID);

  if ( itr == images.end() )
    return;

  if (imageNameMap.find(itr->second.name) != imageNameMap.end())
    imageNameMap.erase(imageNameMap.find(itr->second.name));

  if (itr->second.boundID != _INVALID_ID)
    glDeleteTextures(1,&itr->second.boundID);

  images.erase(itr);
}

void  SimpleDisplay::bindImage ( unsigned int imageID )
{
  std::map<unsigned int,LoadedImage>::iterator itr = images.find(imageID);

  glEnable(GL_TEXTURE_2D);

  if ( itr == images.end() )
    return;

  if (itr->second.boundID == _INVALID_ID)
  {
    // load the image

    SDL_Surface* surface = IMG_Load(itr->second.name.c_str());
    if (surface == NULL)
      return;

    const int origWidth = surface->w;
    const int origHeight = surface->h;
    const int origBpp = surface->format->BitsPerPixel;

    SDL_PixelFormat fmt;
    fmt.palette = NULL;
    fmt.BitsPerPixel = 32;
    fmt.BytesPerPixel = 4;
    fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;
    fmt.Rshift = fmt.Gshift = fmt.Bshift = fmt.Ashift = 0;
    fmt.colorkey = 0;
    fmt.alpha = 0;
    fmt.Rmask = fmt.Gmask = fmt.Bmask = fmt.Amask = 0;    // handle endianess
    ((unsigned char*)&fmt.Rmask)[0] = 0xff;
    ((unsigned char*)&fmt.Gmask)[1] = 0xff;
    ((unsigned char*)&fmt.Bmask)[2] = 0xff;
    ((unsigned char*)&fmt.Amask)[3] = 0xff;

    SDL_Surface* rgba = SDL_ConvertSurface(surface, &fmt, SDL_SWSURFACE);
    SDL_FreeSurface(surface);

    // bail if the conversion failed
    if (rgba == NULL) 
      return;

    int width = rgba->w;
    int height = rgba->h;

    const int rowlen = (rgba->w * 4);
    const int imageSize = (rowlen * rgba->h);
    unsigned char* image = new unsigned char[imageSize];
    const unsigned char* source = (unsigned char*) rgba->pixels;

    for (int i = 0; i < rgba->h; i++)
    {
      memcpy(image + (rowlen * i), source + (rowlen * (rgba->h - 1 - i)), rowlen);
    }

    SDL_FreeSurface(rgba);

    // bind the image to GL
    glGenTextures(1,(GLuint*)&itr->second.boundID );
    glBindTexture(GL_TEXTURE_2D,itr->second.boundID );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 

    gluBuild2DMipmaps(GL_TEXTURE_2D,4,rgba->w,rgba->h,GL_RGBA,GL_UNSIGNED_BYTE,image);
    free(image);
  }

  glBindTexture(GL_TEXTURE_2D,itr->second.boundID);

}

unsigned int  SimpleDisplay::bindImage ( const char* file )
{
  unsigned int id = loadImage(file);
  bindImage(id);
  return id;
}


void SimpleDisplay::clear ( void )
{
  glDrawBuffer(GL_BACK);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void SimpleDisplay::flip ( void )
{
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
#ifdef _FORCE_FLUSH_ON_FLIP
  glFlush();
#endif 
  SDL_GL_SwapBuffers();
}

void SimpleDisplay::getBacgroundColor ( float &r, float &g, float &b )
{
  r = clearColor[0];
  g = clearColor[1];
  b = clearColor[2];
}
void SimpleDisplay::setBacgroundColor ( float r, float g, float b )
{
  clearColor[0] = r;
  clearColor[1] = g;
  clearColor[2] = b;

  glDrawBuffer(GL_FRONT_AND_BACK);
  glClearColor (clearColor[0], clearColor[1], clearColor[2], 0.0);
}

void SimpleDisplay::getDesktopRes ( size_t &x, size_t &y )
{
  x = 0;
  y = 0;

#ifdef _WIN32
  x = (size_t)GetSystemMetrics(SM_CXSCREEN);
  y = (size_t)GetSystemMetrics(SM_CYSCREEN);
#else
  SDL_Rect **modes;

  modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE|SDL_OPENGL);

  if ( modes == (SDL_Rect **)0 ||  modes == (SDL_Rect **)-1 ) // no reses or all reses
    return;

  for ( int i = 0; modes[i]; ++i )
  {
    y = (size_t)modes[i]->h;
    x = (size_t)modes[i]->w;
  }
#endif
}

void SimpleDisplay::getCurrentRes ( size_t &x, size_t &y )
{
  x = size[0];
  y = size[1];
}

void SimpleDisplay::initGL ( void )
{
  setViewport();

  glDrawBuffer(GL_FRONT_AND_BACK);
  setBacgroundColor(clearColor[0], clearColor[1], clearColor[2]);
  glClearDepth(1.0f);

  glHint(GL_PERSPECTIVE_CORRECTION_HINT ,GL_NICEST);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);		

  glEnable (GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glShadeModel (GL_SMOOTH);
  glPolygonMode (GL_FRONT, GL_FILL);

  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0); 

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glEnable (GL_COLOR_MATERIAL); 

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity ();

  glDrawBuffer(GL_FRONT); 
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawBuffer(GL_BACK);
  glClear(GL_COLOR_BUFFER_BIT);
}

void SimpleDisplay::setViewport ( void )
{
  aspect = 1.33f;

  if (size[1] != 0)
    aspect = (float) size[0] / (float) size[1];

  // this stuff don't work in SDL
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (fov,aspect,nearZ,farZ);			
  glViewport (0, 0, (int)size[0], (int)size[1]); 

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void SimpleDisplay::yeld ( float time )
{
#ifdef _WIN32
  Sleep((DWORD)(1000.0f * time));
#else
  usleep((unsigned int )(100000 * time));
#endif
}

bool SimpleDisplay::update ( void )
{
  yeld();
  if (!run)
    return false;

  SDL_Event event;
  while (SDL_PollEvent(&event)) 
  {
    switch(event.type)
    {
    case SDL_QUIT:
      return false;
      break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
      {
	ModiferKeys mods;
	mods.alt = event.key.keysym.mod & KMOD_LALT || event.key.keysym.mod & KMOD_RALT;
	mods.ctl = event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_RCTRL;
	mods.shift = event.key.keysym.mod & KMOD_LSHIFT || event.key.keysym.mod & KMOD_RSHIFT || event.key.keysym.mod & KMOD_CAPS;
	mods.meta = event.key.keysym.mod & KMOD_LMETA || event.key.keysym.mod & KMOD_RMETA;

	key(event.key.keysym.sym,event.type == SDL_KEYDOWN,mods);
      }
      break;

    case SDL_ACTIVEEVENT:
      if (event.active.state == SDL_APPACTIVE)	// iconification
      {
	if (event.active.gain)
	  activate();
	else
	  deactivate();
      }
      else if ( event.active.state == SDL_APPINPUTFOCUS )
	focus(event.active.gain == 0);
      break; 

    case SDL_VIDEORESIZE:
      resize((size_t)event.resize.h,(size_t)event.resize.w);
      break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      {
	int x,y;
	SDL_GetMouseState(&x,&y);
	y = (int)size[1]-y;

   	mouseButton(event.button.button,x,y,event.type == SDL_MOUSEBUTTONDOWN);
      }
      break;

    case SDL_MOUSEMOTION:
	mouseMoved(event.motion.x,(int)size[1]-event.motion.y);
      break;

    }
  }

  return true;
}

void SimpleDisplay::activate ( void )
{
  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->activate();
  }
}

void SimpleDisplay::deactivate ( void )
{

  // blow out any texture IDs
  std::map<unsigned int,LoadedImage>::iterator itr = images.begin();

  while (itr != images.end() )
  {
    //clear the image
    if (itr->second.boundID != _INVALID_ID)
      glDeleteTextures(1,&itr->second.boundID);

    itr->second.boundID = _INVALID_ID;
  }

  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->deactivate();
  }
}

void SimpleDisplay::focus ( bool lost )
{
  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->focus(lost);
  }
}

void SimpleDisplay::resize ( size_t x, size_t y )
{
  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->resize(x,y);
  }
}

void SimpleDisplay::key ( int key, bool down, const ModiferKeys& mods )
{
  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->key(key,down,mods);
  }
}

void SimpleDisplay::mouseButton( int key, int x, int y, bool down )
{
  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->mouseButton(key,x,y,down);
  }
}

void SimpleDisplay::mouseMoved( int x, int y )
{
  if (!callbacks.size())
    return;

  for ( size_t i = 0; i< callbacks.size(); i++ )
  {
    if (callbacks[i])
      callbacks[i]->mouseMoved(x,y);
  }
}


void SimpleDisplay::addEventCallback ( SimpleDisplayEventCallbacks *callback )
{
  if (callback)
    callbacks.push_back(callback);
}

void SimpleDisplay::removeEventCallback ( SimpleDisplayEventCallbacks *callback )
{
  if (callback)
  {
    std::vector<SimpleDisplayEventCallbacks*>::iterator itr = callbacks.begin();
    while (itr != callbacks.end())
    {
      if ( *itr == callback)
      {
	callbacks.erase(itr);
	return;
      }
    }
  }
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8