#include "common.h"
#include "bzfio.h"
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "BzfEvent.h"
#include "PlatformMediaFactory.h"
#include "ConfigIO.h"
#include "TimeKeeper.h"
#include "StateDatabase.h"
#include <stdarg.h>
#include <stdio.h>
#include <fstream>
#include <math.h>

#include "SceneReader.h"
#include "SceneVisitorRender.h"
#include "SceneVisitorSimpleRender.h"
#include "Trackball.h"

typedef SceneVisitorRender RendererType;

static SceneNode* scene = NULL;
static SceneNodeMatrixTransform* camera;
static SceneNodeMatrixTransform* orientation;
static Trackball trackball;
static double fov = 45.0;
static double aspect = 1.0;
static float t = 0.0f;
static float frame = 0.0f;
static bool useLight = false;
static RendererType* renderer;

//
// override global operator new/delete to check for memory usage
//

#include <new>

static size_t			nAlloc = 0;
static size_t			sAlloc = 0;

void *operator new (size_t n) throw (std::bad_alloc)
{
	sAlloc += n;
	nAlloc += 1;
	return malloc(n);
}

void *operator new[] (size_t n) throw (std::bad_alloc)
{
	sAlloc += n;
	nAlloc += n;
	return malloc(n);
}

void operator delete (void * p) throw()
{
	if (p != NULL)
		free(p);
}

void operator delete[] (void * p) throw()
{
	if (p != NULL)
		free(p);
}


//
// draw a frame
//

static void render()
{
	// prep dynamic nodes
	float m[16];
	trackball.getMatrix(m);
	orientation->matrix.set(m, 16);

	float n = 1.0f, f = 3000.0f;
	float a = static_cast<float>(1.0 / tan(0.5 * fov));
	m[0]  = aspect * n / a;
	m[1]  = 0.0f;
	m[2]  = 0.0f;
	m[3]  = 0.0f;
	m[4]  = 0.0f;
	m[5]  = n / a;
	m[6]  = 0.0f;
	m[7]  = 0.0f;
	m[8]  = 0.0f;
	m[9]  = 0.0f;
	m[10] = -(f + n) / (f - n);
	m[11] = -1.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = -2.0f * f * n / (f - n);
	m[15] = 0.0f;
	camera->matrix.set(m, 16);

	// update clocks
	TimeKeeper oldTick(TimeKeeper::getTick());
	TimeKeeper::setTick();
	t += TimeKeeper::getTick() - oldTick;
	frame += 1.0f;

	// prep renderer
	renderer->setTime(t);
	renderer->setFrame(frame);

	// render
	OpenGLGState::instrReset();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer->traverse(scene);

	const RendererType::Instruments* instr = renderer->instrGet();
	const OpenGLGState::Instruments* instr2 = OpenGLGState::instrGet();
	fprintf(stderr, "time: %f (%dk), xform:%d, tri:%d, gstate:%d, tex:%d, mem: %u/%u\n",
								instr->time,
								(int)(instr->nTriangles / instr->time) / 1000,
								instr->nTransforms,
								instr->nTriangles,
								instr2->nState,
								instr2->nTexture,
								nAlloc, sAlloc);
	nAlloc = 0;
	sAlloc = 0;
}

//
// handle events
//

static int wx, wy;
static void resize(int w, int h)
{
	wx = w >> 1;
	wy = h >> 1;
	aspect = (double)h / (double)w;
	trackball.resize(w, h);
	glViewport(0, 0, w, h);
	glScissor(0, 0, w, h);
}

static bool handleEvent(const BzfEvent& event, bool& redraw)
{
	static bool zoom = false;
	static float x, y, x0, y0, fov0;
	switch (event.type) {
		case BzfEvent::KeyDown:
			if (event.keyDown.button == BzfKeyEvent::LeftMouse &&
		  event.keyDown.shift & BzfKeyEvent::ShiftKey &&
		  event.keyDown.shift & BzfKeyEvent::ControlKey) {
				// zoom
				x0 = x;
				y0 = y;
				zoom = true;
				fov0 = fov;
				return true;
			}
			break;

		case BzfEvent::KeyUp:
			if (event.keyUp.button == BzfKeyEvent::LeftMouse) {
				if (zoom) {
					zoom = false;
					return true;
				}
			}
			break;

		case BzfEvent::MouseMove:
			x = (float)(event.mouseMove.x - wx) / wx;
			y = -(float)(event.mouseMove.y - wy) / wy;
			if (zoom) {
				fov = fov0 - (y - y0);
				if (fov < 1.0f)
					fov = 1.0f;
				else if (fov > 80.0f)
					fov = 80.0f;
				redraw = true;
				return true;
			}
			break;
	}
	return false;
}


//
// read scene file
//

static SceneNode* readScene(const char* pathname)
{
	ifstream stream(pathname);
	if (!stream) {
		fprintf(stderr, "can't read file %s\n", pathname);
		return NULL;
	}

	SceneReader reader;
	return reader.read(stream);
}

static SceneNode* wrapScene(SceneNode* world)
{
	SceneNodeLight* light = new SceneNodeLight;
	camera = new SceneNodeMatrixTransform;
	SceneNodeTransform* view = new SceneNodeTransform;
	orientation = new SceneNodeMatrixTransform;
	SceneNodeTransform* worldOrientation = new SceneNodeTransform;

	camera->type.set(SceneNodeBaseTransform::Projection);
	light->position.push(1.0f, 1.0f, 1.0f, 0.0f);
	light->diffuse.push(1.0f, 1.0f, 1.0f);
	view->translate.push(0.0f, 0.0f, -8.0f);
	worldOrientation->rotate.push(1.0f, 0.0f, 0.0f, -90.0f);

	// assemble wrapper nodes
	camera->pushChild(view);
	if (useLight) {
		view->pushChild(light);
		light->pushChild(orientation);
	}
	else {
		view->pushChild(orientation);
	}
	orientation->pushChild(worldOrientation);

	// add child
	worldOrientation->pushChild(world);

	// clean up
	light->unref();
	view->unref();
	orientation->unref();
	worldOrientation->unref();

	return camera;
}

//
// main
//

void printFatalError(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);
	fprintf(stderr, "%s", buffer);
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s [-d] [-l] <scene-file>\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc - 1; ++i) {
		if (strcmp(argv[i], "-l") == 0) {
			useLight = true;
		}
		else if (strcmp(argv[i], "-d") == 0) {
			BZDB->set("displayDebug", "1");
		}
		else {
			fprintf(stderr, "usage: %s [-d] [-l] <scene-file>\n", argv[0]);
			return 1;
		}
	}

	BzfDisplay* display = MPLATFORM->createDisplay(NULL, NULL);
	if (!display) {
		fprintf(stderr, "Can't open display.  Exiting.");
		return 1;
	}

	// choose visual
	BzfVisual* visual = MPLATFORM->createVisual(display);
	visual->setLevel(0);
	visual->setDoubleBuffer(true);
	visual->setRGBA(1, 1, 1, 0);
	visual->setDepth(16);

	// make the window
	BzfWindow* window = MPLATFORM->createWindow(display, visual);
	if (!window->isValid()) {
		fprintf(stderr, "Can't create window.  Exiting.");
		return 1;
	}

	SceneNode* mainScene = readScene(argv[argc - 1]);
	if (mainScene == NULL)
		return 1;

	// augment scene
	scene = wrapScene(mainScene);
	mainScene->unref();

	// reshape window and show it
	window->setSize(480, 480);
	window->setPosition(700, 0);
	window->showWindow(true);
	window->setSize(480, 480);

	// prep context
	int w, h;
	window->getSize(w, h);
	window->makeCurrent();
	resize(w, h);

	// create renderer
	renderer = new RendererType;

	// initialize TimeKeeper
	TimeKeeper::setTick();

	// discard first frame
	nAlloc = 0;
	sAlloc = 0;
	render();

	bool quit = false;
	while (!quit) {
		while (!quit && display->isEventPending()) {
			BzfEvent event;
			if (display->getEvent(event)) {
				switch (event.type) {
					case BzfEvent::Quit:
						quit = true;
						break;

					case BzfEvent::KeyDown:
						if (event.keyUp.ascii == 27)
							quit = true;
						break;

					case BzfEvent::Resize:
						resize(event.resize.width, event.resize.height);
						break;
				}

				// do events
				bool redraw;
				if (!handleEvent(event, redraw))
					trackball.onEvent(event, redraw);
			}
		}

		if (trackball.isSpinning())
			trackball.spin();

		// draw frame
		render();
		window->swapBuffers();
	}

	delete window;
	delete visual;
	delete display;

	return 0;
}
