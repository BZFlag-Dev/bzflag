// test intersectors in 2D

#include "common.h"
#include "bzfgl.h"
#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "BzfEvent.h"
#include "PlatformMediaFactory.h"
#include "math3D.h"
#include <stdio.h>
#include <math.h>
#include <vector>

#include "ContactSurfacePoint.h"
#include "ContactSurfaceEdge.h"
#include "ContactSurfacePolygon.h"
#include "ContactSurfaceIntersector.h"
#include "ContactSurfaceIntersectorPolygon.h"

class Object {
public:
	ContactSurface*		surface;
	float				x, y;
	float				rotation;
};
typedef std::vector<Object> Objects;

static double aspect = 1.0;
static unsigned int current = 0;
static Objects objects;

static void addObject(ContactSurface* surface, float x, float y)
{
	Object object;
	object.surface  = surface;
	object.x        = x;
	object.y        = y;
	object.rotation = 0.0f;
	objects.push_back(object);
}

static void addObjects()
{
	ContactSurfacePolygon::VertexList vertices;
	for (unsigned int i = 0; i < 5; ++i) {
		float a = ((float)i / 5.0f) * 2.0f * M_PI;
		vertices.push_back(Vec3(0.2 * cosf(a), 0.2 * sinf(a), 0.0));
	}

/*
	addObject(new ContactSurfacePoint(Vec3(0.0f, 0.0f, 0.0f),
										Vec3(0.0f, 0.0f, 1.0f)), 0.25, 0.25);
*/
	addObject(new ContactSurfaceEdge(Vec3(-0.2f, 0.0f, 0.0f),
										Vec3(0.2f, 0.0f, 0.0f),
										Vec3(0.0f, 0.0f, 1.0f)), -0.25, 0.25);
	addObject(new ContactSurfacePolygon(vertices), -0.3, -0.5);

/*
	addObject(new ContactSurfacePoint(Vec3(0.0f, 0.0f, 0.0f),
										Vec3(0.0f, 0.0f, 1.0f)), 0.35, 0.25);
*/
	addObject(new ContactSurfaceEdge(Vec3(-0.2f, 0.0f, 0.0f),
										Vec3(0.2f, 0.0f, 0.0f),
										Vec3(0.0f, 0.0f, 1.0f)), -0.25, 0.35);
	addObject(new ContactSurfacePolygon(vertices), 0.3, -0.5);
}

static void addIntersectors()
{
	ContactSurfaceIntersectorManager::getInstance()->add(
								ContactSurfacePoint::getClassType(),
								ContactSurfacePoint::getClassType(),
								new ContactSurfaceIntersectorPointPoint);
	ContactSurfaceIntersectorManager::getInstance()->add(
								ContactSurfacePoint::getClassType(),
								ContactSurfaceEdge::getClassType(),
								new ContactSurfaceIntersectorPointEdge);
	ContactSurfaceIntersectorManager::getInstance()->add(
								ContactSurfacePoint::getClassType(),
								ContactSurfacePolygon::getClassType(),
								new ContactSurfaceIntersectorPointPolygon);
	ContactSurfaceIntersectorManager::getInstance()->add(
								ContactSurfaceEdge::getClassType(),
								ContactSurfaceEdge::getClassType(),
								new ContactSurfaceIntersectorEdgeEdge);
	ContactSurfaceIntersectorManager::getInstance()->add(
								ContactSurfaceEdge::getClassType(),
								ContactSurfacePolygon::getClassType(),
								new ContactSurfaceIntersectorEdgePolygon);
	ContactSurfaceIntersectorManager::getInstance()->add(
								ContactSurfacePolygon::getClassType(),
								ContactSurfacePolygon::getClassType(),
								new ContactSurfaceIntersectorPolygonPolygon);
}

//
// draw a frame
//

static void drawSurface(ContactSurface* vsurface,
						const float* c1, const float* c2)
{
	if (vsurface->getType() == ContactSurfacePoint::getClassType()) {
		ContactSurfacePoint* surface = (ContactSurfacePoint*)vsurface;
		glColor3fv(c1);
		glBegin(GL_POINTS);
		glVertex3fv(surface->getVertex());
		glEnd();
	}

	else if (vsurface->getType() == ContactSurfaceEdge::getClassType()) {
		ContactSurfaceEdge* surface = (ContactSurfaceEdge*)vsurface;
		glColor3fv(c1);
		glBegin(GL_LINES);
		glVertex3fv(surface->getVertex1());
		glVertex3fv(surface->getVertex2());
		glEnd();
	}

	else if (vsurface->getType() == ContactSurfacePolygon::getClassType()) {
		ContactSurfacePolygon* surface = (ContactSurfacePolygon*)vsurface;
		const ContactSurfacePolygon::VertexList& v = surface->getVertices();
		glColor3fv(c2);
		glBegin(GL_POLYGON);
		for (unsigned int i = 0; i < v.size(); ++i)
			glVertex3fv(v[i]);
		glEnd();
		glColor3fv(c1);
		glBegin(GL_LINE_LOOP);
		for (unsigned int i = 0; i < v.size(); ++i)
			glVertex3fv(v[i]);
		glEnd();
	}
}

static void getContacts(ContactPoints& contacts,
								Object& o1, Object& o2)
{
	unsigned int start = contacts.size();

	Matrix m1, m2;
	m1.setTranslate(o1.x, o1.y, 0.0f);
	m2.setTranslate(o2.x, o2.y, 0.0f);
	Matrix tmp;
	tmp.setRotate(0.0f, 0.0f, 1.0f, 270.0f * o1.rotation);
	m1 *= tmp;
	tmp.setRotate(0.0f, 0.0f, 1.0f, 270.0f * o2.rotation);
	m2 *= tmp;

	const ContactSurfaceIntersectorBase* intersector =
						ContactSurfaceIntersectorManager::getInstance()->
							get(o1.surface->getType(), o2.surface->getType());
	if (intersector != NULL) {
		ContactSurface* s1 = o1.surface->clone();
		ContactSurface* s2 = o2.surface->clone();
		tmp = m1;
		tmp.invert();
		s2->transform(m2);
		s2->transform(tmp);
		intersector->intersect(contacts, NULL, NULL, s1, s2);
		delete s1;
		delete s2;
		for (unsigned int i = start; i < contacts.size(); ++i)
			contacts[i].point.xformPoint(m1);
		return;
	}

	intersector = ContactSurfaceIntersectorManager::getInstance()->
							get(o2.surface->getType(), o1.surface->getType());
	if (intersector != NULL) {
		ContactSurface* s1 = o1.surface->clone();
		ContactSurface* s2 = o2.surface->clone();
		tmp = m2;
		tmp.invert();
		s1->transform(m1);
		s1->transform(tmp);
		intersector->intersect(contacts, NULL, NULL, s2, s1);
		delete s1;
		delete s2;
		for (unsigned int i = start; i < contacts.size(); ++i)
			contacts[i].point.xformPoint(m2);
		return;
	}
}

static void render()
{
	static const float colorSelected[3] = { 1.0f, 1.0f, 0.0f };
	static const float colorUnselected[3] = { 0.0f, 0.0f, 1.0f };
	static const float colorSecondary[3] = { 0.5f, 0.5f, 0.5f };
	static const float colorContact[3] = { 1.0f, 0.1f, 0.1f };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (unsigned int j = 0; j < objects.size(); ++j) {
		unsigned int i = (j + 1 + current) % objects.size();

		Matrix m;
		m.setTranslate(objects[i].x, objects[i].y, 0.0f);
		Matrix tmp;
		tmp.setRotate(0.0f, 0.0f, 1.0f, 270.0f * objects[i].rotation);
		m *= tmp;
		ContactSurface* surface = objects[i].surface;

		glPushMatrix();
		glMultMatrixf(m);
		drawSurface(surface, (i == current) ? colorSelected :
											colorUnselected, colorSecondary);
		glPopMatrix();
	}

	// now find collisions
	ContactPoints contacts;
	for (unsigned int i = 0; i < objects.size(); ++i) {
		if (i == current)
			continue;
		getContacts(contacts, objects[current], objects[i]);
	}

	// draw contacts
	glColor3fv(colorContact);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < contacts.size(); ++i)
		glVertex3fv(contacts[i].point);
	glEnd();
}

//
// handle events
//

static int wx, wy;
static void resize(int w, int h)
{
	wx = w >> 1;
	wy = h >> 1;
	aspect = (double)w / (double)h;
	glViewport(0, 0, w, h);
	glScissor(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}


//
// main
//

#if defined(_WIN32)
int myMain(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
	if (argc != 1) {
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
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

	addObjects();
	addIntersectors();

	// discard first frame
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glPointSize(5.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	render();

	float x0, y0, x = 0.0f, y = 0.0f;
	bool move = false, turn = false;
	bool quit = false;
	while (!quit) {
		while (!quit && display->isEventPending()) {
			BzfEvent event;
			if (display->getEvent(event)) {
				switch (event.type) {
					case BzfEvent::Quit:
						quit = true;
						break;

					case BzfEvent::Resize:
						resize(event.resize.width, event.resize.height);
						break;

					case BzfEvent::KeyDown:
						if (event.keyUp.ascii == 27) {
							quit = true;
						}
						else if (event.keyDown.button == BzfKeyEvent::LeftMouse) {
							x0 = x;
							y0 = y;
							move = true;
							turn = false;
						}
						else if (event.keyDown.button == BzfKeyEvent::MiddleMouse) {
							x0 = x;
							y0 = y;
							turn = true;
							move = false;
						}
						else if (event.keyDown.button == BzfKeyEvent::RightMouse) {
							if (!move && !turn)
								current = (current + 1) % objects.size();
						}
						break;

					case BzfEvent::KeyUp:
						if (event.keyUp.button == BzfKeyEvent::LeftMouse) {
							move = false;
						}
						else if (event.keyDown.button == BzfKeyEvent::MiddleMouse) {
							turn = false;
						}
						break;

					case BzfEvent::MouseMove:
						x =  (float)(event.mouseMove.x - wx) / wx;
						y = -(float)(event.mouseMove.y - wy) / wy;
						if (move) {
							objects[current].x += (x - x0);
							objects[current].y += (y - y0);
							x0 = x;
							y0 = y;
						}
						else if (turn) {
							objects[current].rotation += (y - y0);
							x0 = x;
							y0 = y;
						}
						break;

					default:
						// ignore other events
						break;
				}
			}
		}

		// draw frame
		render();
		window->swapBuffers();
	}

	delete window;
	delete visual;
	delete display;

	return 0;
}

#if defined(_WIN32)

#include <stdlib.h>

int WINAPI				WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return myMain(__argc, __argv);
}

#endif
// ex: shiftwidth=4 tabstop=4
