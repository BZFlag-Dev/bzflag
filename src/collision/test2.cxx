#include "common.h"
#include "bzfio.h"
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "BzfEvent.h"
#include "PlatformMediaFactory.h"
#include "StateDatabase.h"
#include "TimeKeeper.h"
#include <stdarg.h>
#include <stdio.h>
#include <sstream>
#include <math.h>

#include "SceneReader.h"
#include "SceneVisitorRender.h"
#include "SceneVisitorSimpleRender.h"
#include "Trackball.h"

#include "ODESolverRungeKutta.h"
#include "BodyODEAssistant.h"
#include "ShapeBox.h"
#include "ShapePyramid.h"
#include "Body.h"
#include "BodyManager.h"

#include "ContactSurfacePoint.h"
#include "ContactSurfaceEdge.h"
#include "ContactSurfacePolygon.h"
#include "ContactSurfaceIntersector.h"
#include "ContactSurfaceIntersectorPolygon.h"

typedef SceneVisitorRender RendererType;

static SceneNode* scene = NULL;
static SceneNodeMatrixTransform* camera;
static SceneNodeMatrixTransform* orientation;
static SceneNodeGeometry* contactGeometry;
static SceneNodePrimitive* contactPrimitive;
static Trackball trackball;
static int wx, wy;
static float wsize;
static float t = 0.0f;
static float frame = 0.0f;
static bool useLight = true;
static bool pause = false;
static bool depth = false;
static RendererType* renderer;

class BodyInfo {
public:
	Body*						body;
	SceneNodeMatrixTransform*	xform;
};
typedef std::vector<BodyInfo> BodyInfoList;
static BodyInfoList			bodies;
static ContactPoints		s_contacts;

class TestODEAssistant : public BodyODEAssistant {
public:
	TestODEAssistant() { }
	virtual ~TestODEAssistant() { }

protected:
	// ODEAssistant overrides
	virtual void		onDrive(const ContactPoints&);
};

void					TestODEAssistant::onDrive(
								const ContactPoints& contacts)
{
	s_contacts = contacts;
	contactGeometry->setBundle(0);
	contactGeometry->color->clear();
	contactGeometry->vertex->clear();
	contactPrimitive->index.clear();
	unsigned int i = 0;
	for (ContactPoints::const_iterator index = contacts.begin();
								index != contacts.end(); ++i, ++index) {
		contactPrimitive->index.push(i);
		if (index->isEdgeEdge)
			contactGeometry->color->push(0.0f, 1.0f, 0.0f, 1.0f);
		else
			contactGeometry->color->push(1.0f, 0.0f, 0.0f, 1.0f);
		contactGeometry->vertex->push(index->point[0],
								index->point[1], index->point[2]);
	}
	if (i == 0)
		contactGeometry->color->push(1.0f, 0.0f, 0.0f, 1.0f);
}

static void				addIntersectors()
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

static void drawVel(Body* a)
{
if (depth)
	glEnable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
	std::vector<Vec3> points;
	a->getShape()->getDumpPoints(points);
	for (unsigned int i = 0; i < points.size(); ++i)
		points[i].xformPoint(a->getTransform());
	for (unsigned int i = 0; i < s_contacts.size(); ++i)
//		if (s_contacts[i].a == a)
			points.push_back(s_contacts[i].point);
	const unsigned int n = points.size();
	for (unsigned int i = 0; i < n; ++i) {
		Vec3 p = points[i];

		Vec3 v, p2;
		p2 = p;
		a->getPointVelocity(v, p2);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3d(p2[0], p2[1], p2[2]);
		p2 += 0.0002 * wsize * v;
		glVertex3d(p2[0], p2[1], p2[2]);

		p2 = p;
		a->getPointAcceleration(v, p2);
		glColor3f(1.0, 0.0, 1.0);
		glVertex3d(p2[0], p2[1], p2[2]);
		p2 += 0.00004 * wsize * v;
		glVertex3d(p2[0], p2[1], p2[2]);
	}
	for (unsigned int i = 0; i < s_contacts.size(); ++i) {
		Vec3 p2 = s_contacts[i].point;
		Vec3 aa, ab;
		s_contacts[i].a->getPointAcceleration(aa, p2);
		s_contacts[i].b->getPointAcceleration(ab, p2);
		aa -= ab;
		glColor3f(1.0, 0.75, 0.5);
		glVertex3d(p2[0], p2[1], p2[2]);
		p2 += 0.00004 * wsize * aa;
		glVertex3d(p2[0], p2[1], p2[2]);

Vec3 n = s_contacts[i].normal;
Real d = -(n * aa);
glColor3f(0.5, 0.2, 0.2);
glVertex3d(p2[0], p2[1], p2[2]);
p2 += 0.00004 * wsize * d * n;
glVertex3d(p2[0], p2[1], p2[2]);

glColor3f(0.2, 0.5, 0.2);
glVertex3d(p2[0], p2[1], p2[2]);
p2 = s_contacts[i].point;
glVertex3d(p2[0], p2[1], p2[2]);

		Vec3 ndot;
		s_contacts[i].getNormalDerivative(ndot);
		p2 = s_contacts[i].point;
		glColor3f(0.0, 0.5, 1.0);
		glVertex3d(p2[0], p2[1], p2[2]);
		p2 += 0.0001 * wsize * ndot;
		glVertex3d(p2[0], p2[1], p2[2]);

		p2 = s_contacts[i].point;
		glColor3f(0.0, 1.0, 0.5);
		glVertex3d(p2[0], p2[1], p2[2]);
		p2 += 0.0001 * wsize * s_contacts[i].normal;
		glVertex3d(p2[0], p2[1], p2[2]);
	}
	glEnd();
if (depth)
	glDisable(GL_DEPTH_TEST);
}

static void render()
{
	// prep dynamic nodes
	float m[16], p[16];
	trackball.getMatrix(m);
	orientation->matrix.set(m, 16);
	trackball.getProjection(p, 1.0f, 3000.0f);
	camera->matrix.set(p, 16);

	// prep renderer
	renderer->setTime(t);
	renderer->setFrame(frame);

	// render
	OpenGLGState::instrReset();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer->traverse(scene);

glMatrixMode(GL_PROJECTION);
glPushMatrix();
glLoadMatrixf(p);
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glLoadIdentity();
glTranslatef(0.0f, 0.0f, -8.0f);
glMultMatrixf(m);
glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
for (BodyManager::const_iterator i = BODYMGR->begin(); i != BODYMGR->end(); ++i)
drawVel(*i);
glPopMatrix();
glMatrixMode(GL_PROJECTION);
glPopMatrix();
glMatrixMode(GL_MODELVIEW);
}

//
// handle events
//

static void resize(int w, int h)
{
	wx = w >> 1;
	wy = h >> 1;
	if (h < w)
		wsize = (float)h;
	else
		wsize = (float)w;
	trackball.resize(w, h);
	glViewport(0, 0, w, h);
	glScissor(0, 0, w, h);
}

static bool handleEvent(const BzfEvent& event, bool& /*redraw*/)
{
	switch (event.type) {
		case BzfEvent::KeyDown:
			if (event.keyDown.ascii == ' ')
				pause = !pause;
			if (event.keyDown.ascii == 'd')
				depth = !depth;
			break;

		case BzfEvent::KeyUp:
			break;

		case BzfEvent::MouseMove:
			break;

		default:
			// ignore other events
			break;
	}
	return false;
}


//
// read scene file
//

static SceneNode* parse(const std::string& buffer)
{
	std::istringstream stream(buffer.c_str());

	try {
		// read XML
		XMLTree xmlTree;
		xmlTree.read(stream, XMLStreamPosition());

		// parse scene
		SceneReader reader;
		return reader.parse(xmlTree.begin());
	}
	catch (XMLIOException& e) {
		fprintf(stderr, "<model> (%d,%d): %s",
						e.position.line,
						e.position.column,
						e.what());
		exit(1);
	}
}

static SceneNode* createBoxSceneNode(Real x, Real y, Real z)
{
	static const float s_normal[][3] = {
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },
								{ -1.0f,  0.0f,  0.0f },

								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },
								{  0.0f,  0.0f,  1.0f },

								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f }
						};
	static const float s_vertex[][3] = {
								{ -1.0f, -1.0f,  1.0f },
								{ -1.0f, -1.0f, -1.0f },
								{  1.0f, -1.0f,  1.0f },
								{  1.0f, -1.0f, -1.0f },
								{ -1.0f,  1.0f,  1.0f },
								{ -1.0f,  1.0f, -1.0f },
								{  1.0f,  1.0f,  1.0f },
								{  1.0f,  1.0f, -1.0f },
								{ -1.0f, -1.0f,  1.0f },
								{ -1.0f, -1.0f, -1.0f },

								{ -1.0f, -1.0f,  1.0f },
								{  1.0f, -1.0f,  1.0f },
								{ -1.0f,  1.0f,  1.0f },
								{  1.0f,  1.0f,  1.0f },

								{ -1.0f, -1.0f, -1.0f },
								{  1.0f, -1.0f, -1.0f },
								{ -1.0f,  1.0f, -1.0f },
								{  1.0f,  1.0f, -1.0f }
						};

	std::string model;
	model +=	"<gstate>\n";
	model +=	"  <shading model=\"flat\"/>\n";
	model +=	"<geometry>\n";
	model +=	"  <color>1 1 1 1</color>\n";
	model +=	"  <normal>\n";
	for (unsigned int i = 0; i < countof(s_normal); ++i)
		model += string_util::format("%f %f %f\n",
							s_normal[i][0], s_normal[i][1], s_normal[i][2]);
	model +=	"  </normal>\n";
	model +=	"  <vertex>\n";
	for (unsigned int i = 0; i < countof(s_vertex); ++i)
		model += string_util::format("%f %f %f\n",
							x * s_vertex[i][0],
							y * s_vertex[i][1],
							z * s_vertex[i][2]);
	model +=	"  </vertex>\n";

	model +=	"  <primitive type=\"tstrip\"><index>"
								"0 1 2 3 6 7 4 5 8 9"
								"</index></primitive>\n";
	model +=	"  <primitive type=\"tstrip\"><index>"
								"10 11 12 13"
								"</index></primitive>\n";
	model +=	"  <primitive type=\"tstrip\"><index>"
								"15 14 17 16"
								"</index></primitive>\n";
	model +=	"</geometry>\n";
	model +=	"</gstate>\n";

	return parse(model);
}

static SceneNode* createPyramidSceneNode(Real x, Real y, Real z)
{
	static const float s_normal[][3] = {
								{  0.0f,  0.0f,  1.0f },
								{ -1.0f,  0.0f,  0.0f },
								{  0.0f, -1.0f,  0.0f },
								{  0.0f,  1.0f,  0.0f },
								{  1.0f,  0.0f,  0.0f },

								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f },
								{  0.0f,  0.0f, -1.0f }
						};
	static const float s_vertex[][3] = {
								{  0.0f,  0.0f,  0.75f },
								{ -1.0f, -1.0f, -0.25f },
								{  1.0f, -1.0f, -0.25f },
								{ -1.0f,  1.0f, -0.25f },
								{  1.0f,  1.0f, -0.25f },

								{ -1.0f, -1.0f, -0.25f },
								{  1.0f, -1.0f, -0.25f },
								{ -1.0f,  1.0f, -0.25f },
								{  1.0f,  1.0f, -0.25f }
						};

	std::string model;
	model +=	"<gstate>\n";
	model +=	"  <shading model=\"flat\"/>\n";
	model +=	"<geometry>\n";
	model +=	"  <color>1 1 1 1</color>\n";
	model +=	"  <normal>\n";
	for (unsigned int i = 0; i < countof(s_normal); ++i)
		model += string_util::format("%f %f %f\n",
							s_normal[i][0], s_normal[i][1], s_normal[i][2]);
	model +=	"  </normal>\n";
	model +=	"  <vertex>\n";
	for (unsigned int i = 0; i < countof(s_vertex); ++i)
		model += string_util::format("%f %f %f\n",
							x * s_vertex[i][0],
							y * s_vertex[i][1],
							z * s_vertex[i][2]);
	model +=	"  </vertex>\n";

	model +=	"  <primitive type=\"tfan\"><index>"
								"0 1 2 4 3 1"
								"</index></primitive>\n";
	model +=	"  <primitive type=\"tstrip\"><index>"
								"6 5 8 7"
								"</index></primitive>\n";
	model +=	"</geometry>\n";
	model +=	"</gstate>\n";

	return parse(model);
}

static Body* addBox(SceneNodeGroup* group,
						Real sx, Real sy, Real sz, Real invDensity)
{
	BodyInfo info;
	info.body = new Body(new ShapeBox(sx, sy, sz), invDensity);
	info.xform = new SceneNodeMatrixTransform;
	bodies.push_back(info);

	SceneNode* box = createBoxSceneNode(sx, sy, sz);
	info.xform->pushChild(box);
	group->pushChild(info.xform);
	info.xform->unref();
	box->unref();

	BODYMGR->add(info.body);
	return info.body;
}

static Body* addPyramid(SceneNodeGroup* group,
						Real sx, Real sy, Real sz, Real invDensity)
{
	BodyInfo info;
	info.body = new Body(new ShapePyramid(sx, sy, sz), invDensity);
	info.xform = new SceneNodeMatrixTransform;
	bodies.push_back(info);

	SceneNode* box = createPyramidSceneNode(sx, sy, sz);
	info.xform->pushChild(box);
	group->pushChild(info.xform);
	info.xform->unref();
	box->unref();

	BODYMGR->add(info.body);
	return info.body;
}

static SceneNode* createScene()
{
	SceneNodeGroup* group = new SceneNodeGroup;

	Body* body;
//	body = addBox(group, 0.5, 0.2, 0.3, R_(1.0) / R_(1000.0));
	body = addPyramid(group, 0.5, 0.2, 0.3, R_(1.0) / R_(1000.0));
	body->setPosition(Vec3(0.0, 0.0, 0.5));
	body->setVelocity(Vec3( 0.1, 0.0, 0.0));
	body->setAngularVelocity(Vec3(1.0, 1.0, 2.0));
//	body->setAngularVelocity(Vec3(1.0, 1.0, 20.0));
	body->setOrientation(Quaternion(Vec3(0.0, 0.0, 1.0), 30.0));

	body = addBox(group, 1.0, 1.0, 0.1, R_(0.0));
	body->setPosition(Vec3( 0.0, 0.0, -1.0));
	body->setVelocity(Vec3( 0.0, 0.0, 0.0));
/*
	body = addBox(group, 0.2, 0.2, 0.2, R_(1.0) / R_(1000.0));
	body->setPosition(Vec3( 0.0, 0.0, -0.69));
	body->setVelocity(Vec3( 0.0, 0.0, 0.0));
	body->setAngularVelocity(Vec3(1.5, 0.5, 1.0));
*/
	return group;
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

	// create contact point nodes
	OpenGLGStateBuilder builder;
	builder.setPointSize(4.0f);
	builder.setBlending(GState::kSrcAlpha, GState::kOneMinusSrcAlpha);
	SceneNodeGState* contactGroup = new SceneNodeGState;
	contactGroup->set(builder.getState());
	contactGeometry = new SceneNodeGeometry;
	contactPrimitive = new SceneNodePrimitive;
	contactGroup->pushChild(contactGeometry);
	contactGeometry->pushChild(contactPrimitive);
	contactPrimitive->type.set(SceneNodePrimitive::Points);
	contactGeometry->pushBundle();
//	contactGeometry->color->push(1.0f, 0.0f, 0.0f, 1.0f);

	// add child
	worldOrientation->pushChild(world);
	worldOrientation->pushChild(contactGroup);

	// clean up
	contactPrimitive->unref();
	contactGeometry->unref();
	contactGroup->unref();
	light->unref();
	view->unref();
	orientation->unref();
	worldOrientation->unref();

	return camera;
}

//
// main
//

#if defined(_WIN32)
int myMain(int /*argc*/, char** /*argv*/)
#else
int main(int /*argc*/, char** /*argv*/)
#endif
{
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

	// create scene
	SceneNode* mainScene = createScene();
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

	// turn on features
	BZDB->set("renderBlending", "1");
	BZDB->set("renderSmoothing", "1");
	BZDB->set("renderTexturing", "1");
	if (useLight)
		BZDB->set("renderLighting", "1");

	// create renderer
	renderer = new RendererType;

	// prepare simulator
	addIntersectors();
	ODESolver* solver = new ODESolverRungeKutta;
	ODEAssistant* solverAssistant = new TestODEAssistant;
	VectorN y;
	solverAssistant->marshall(y);

	// initialize TimeKeeper
	TimeKeeper::setTick();

	// discard first frame
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

					default:
						// ignore other events
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

		// update clocks
		TimeKeeper oldTick(TimeKeeper::getTick());
		TimeKeeper::setTick();
		float dt = TimeKeeper::getTick() - oldTick;
		t     += dt;
		frame += 1.0f;

		// update simulation
dt = 0.002;
if (!pause) {
		solver->solve(y, t, dt, solverAssistant);
 solverAssistant->drive(t, ODEAssistant::Normal);
 for (unsigned int i = 0; i < s_contacts.size(); ++i) {
  Vec3 aAcc, bAcc;
  s_contacts[i].a->getPointAcceleration(aAcc, s_contacts[i].point);
  s_contacts[i].b->getPointAcceleration(bAcc, s_contacts[i].point);
  fprintf(stderr, "%d:\n  a: %+8.4e %+8.4e %+8.4e\n  b: %+8.4e %+8.4e %+8.4e\n",
				i, aAcc[0], aAcc[1], aAcc[2], bAcc[0], bAcc[1], bAcc[2]);
  aAcc -= bAcc;
  fprintf(stderr, "  n: %+8.4e %+8.4e %+8.4e\n  r: %+8.4e\n",
				s_contacts[i].normal[0], s_contacts[i].normal[1],
				s_contacts[i].normal[2], aAcc * s_contacts[i].normal);
 }
}

		// update model
		float m[16];
		for (BodyInfoList::const_iterator index = bodies.begin();
								index != bodies.end(); ++index)
			index->xform->matrix.set(index->body->getTransform().get(m), 16);

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
