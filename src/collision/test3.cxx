/* enhanced gilbert johnson keerthi;  rotation as quaternion.
 *
 * CC -g -n32 -mips3 -o egjk egjk.cxx -lglut -lGLU -lGL -lXmu -lX11 -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GL/glut.h>
#include "Trackball2.h"
#include "ShapeBox.h"
#include "ShapePyramid.h"
#include "Body.h"
#include "CollisionDetectorGJK.h"

//
//
//

class BodyInfo {
public:
    Body*		body;
    void		(*draw)(Body*);
};

static BodyInfo		objects[2] = { { NULL, NULL}, { NULL, NULL} };
static int		numObjects = 0;
static int		pickedObject = -1;
static int		width, height;
static Real		viewWidth, viewHeight, viewDepth;
static bool		moving = false, rotating = false, viewing = false;
static Vec3		orig;
static Vec3		du, dv;
static int		m0[2];
static Quaternion	viewRotation;
static Trackball*	trackball = NULL;

class CollisionDetectorTest : public CollisionDetectorGJK {
public:
    CollisionDetectorTest() { }
    ~CollisionDetectorTest() { }

    Type		compare(Vec3& aPoint, Vec3& bPoint,
				ContactSimplex* aSimplex,
				ContactSimplex* bSimplex,
				const Body* a, const Body* b) const
				{ return getPoints(aPoint, bPoint,
					aSimplex, bSimplex, a, b); }
};
static CollisionDetectorTest detector;

//
// display and interaction
//

static void		drawBox(Body* body)
{
    ShapeBox* box = (ShapeBox*)body->getShape();
    glPushMatrix();
    glMultMatrixr(body->getTransform().get());
    Real x = box->getX();
    Real y = box->getY();
    Real z = box->getZ();
    glBegin(GL_QUADS);
	glNormal3f( 1.0f,  0.0f,  0.0f);
	glVertex3f( x, -y,  z);
	glVertex3f( x, -y, -z);
	glVertex3f( x,  y, -z);
	glVertex3f( x,  y,  z);

	glNormal3f(-1.0f,  0.0f,  0.0f);
	glVertex3f(-x,  y,  z);
	glVertex3f(-x,  y, -z);
	glVertex3f(-x, -y, -z);
	glVertex3f(-x, -y,  z);

	glNormal3f( 0.0f,  1.0f,  0.0f);
	glVertex3f(-x,  y,  z);
	glVertex3f( x,  y,  z);
	glVertex3f( x,  y, -z);
	glVertex3f(-x,  y, -z);

	glNormal3f( 0.0f, -1.0f,  0.0f);
	glVertex3f(-x, -y, -z);
	glVertex3f( x, -y, -z);
	glVertex3f( x, -y,  z);
	glVertex3f(-x, -y,  z);

	glNormal3f( 0.0f,  0.0f,  1.0f);
	glVertex3f(-x, -y,  z);
	glVertex3f( x, -y,  z);
	glVertex3f( x,  y,  z);
	glVertex3f(-x,  y,  z);

	glNormal3f( 0.0f,  0.0f, -1.0f);
	glVertex3f(-x,  y, -z);
	glVertex3f( x,  y, -z);
	glVertex3f( x, -y, -z);
	glVertex3f(-x, -y, -z);
    glEnd();
    glPopMatrix();
}

static void		drawPyramid(Body* body)
{
    ShapePyramid* p = (ShapePyramid*)body->getShape();
    glPushMatrix();
    glMultMatrixr(body->getTransform().get());
    Real x = p->getX();
    Real y = p->getY();
    Real z = p->getZ();
    glBegin(GL_TRIANGLES);
	glNormal3f( 1.0f,  0.0f,  0.0f);
	glVertex3f(-x, -y, R_(-0.25) * z);
	glVertex3f( R_(0.0), R_(0.0), R_(0.75) * z);
	glVertex3f(-x,  y, R_(-0.25) * z);

	glNormal3f(-1.0f,  0.0f,  0.0f);
	glVertex3f( x, -y, R_(-0.25) * z);
	glVertex3f( x,  y, R_(-0.25) * z);
	glVertex3f( R_(0.0), R_(0.0), R_(0.75) * z);

	glNormal3f( 0.0f,  1.0f,  0.0f);
	glVertex3f(-x, -y, R_(-0.25) * z);
	glVertex3f( x, -y, R_(-0.25) * z);
	glVertex3f( R_(0.0), R_(0.0), R_(0.75) * z);

	glNormal3f( 0.0f, -1.0f,  0.0f);
	glVertex3f(-x,  y, R_(-0.25) * z);
	glVertex3f( R_(0.0), R_(0.0), R_(0.75) * z);
	glVertex3f( x,  y, R_(-0.25) * z);
    glEnd();

    glBegin(GL_QUADS);
	glNormal3f( 0.0f,  0.0f, -1.0f);
	glVertex3f(-x,  y, R_(-0.25) * z);
	glVertex3f( x,  y, R_(-0.25) * z);
	glVertex3f( x, -y, R_(-0.25) * z);
	glVertex3f(-x, -y, R_(-0.25) * z);
    glEnd();
    glPopMatrix();
}

static void		drawDot(const Vec3& v, int icon)
{
    static const GLubyte bitmap[5 * 28] = {
				0x38, 0x20, 0x00, 0x00,
				0x7c, 0x00, 0x00, 0x00,
				0xfe, 0x20, 0x00, 0x00,
				0xfe, 0x10, 0x00, 0x00,
				0xfe, 0x08, 0x00, 0x00,
				0x7c, 0x88, 0x00, 0x00,
				0x38, 0x70, 0x00, 0x00,

				0x38, 0x20, 0x00, 0x00,
				0x7c, 0x20, 0x00, 0x00,
				0xfe, 0x50, 0x00, 0x00,
				0xfe, 0x50, 0x00, 0x00,
				0xfe, 0x88, 0x00, 0x00,
				0x7c, 0x88, 0x00, 0x00,
				0x38, 0x88, 0x00, 0x00,

				0x38, 0xf8, 0x00, 0x00,
				0x7c, 0x80, 0x00, 0x00,
				0xfe, 0x80, 0x00, 0x00,
				0xfe, 0xf8, 0x00, 0x00,
				0xfe, 0x80, 0x00, 0x00,
				0x7c, 0x80, 0x00, 0x00,
				0x38, 0xf8, 0x00, 0x00,

				0x38, 0x80, 0x00, 0x00,
				0x7c, 0x80, 0x00, 0x00,
				0xfe, 0x80, 0x00, 0x00,
				0xfe, 0xf8, 0x00, 0x00,
				0xfe, 0x80, 0x00, 0x00,
				0x7c, 0x80, 0x00, 0x00,
				0x38, 0xf8, 0x00, 0x00,

				0x38, 0x00, 0x00, 0x00,
				0x7c, 0x00, 0x00, 0x00,
				0xfe, 0x00, 0x00, 0x00,
				0xfe, 0x00, 0x00, 0x00,
				0xfe, 0x00, 0x00, 0x00,
				0x7c, 0x00, 0x00, 0x00,
				0x38, 0x00, 0x00, 0x00,
			};
    if (icon < 0) icon = 4;
    else if (icon > 3) icon = 3;

    R_glv(glRasterPos3)(v.get());
    glBitmap(15, 7, 3, 3, 0, 0, bitmap + 28 * icon);
}

static void		redraw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Matrix tmp(viewRotation);
    glPushMatrix();
    glMultMatrixr(tmp.get());

    static float lightPos1[4] = { 1.0, 2.0, 3.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos1);
    glEnable(GL_LIGHTING);

    glColor3f(0.5f, 1.0f, 0.5f);
    objects[0].draw(objects[0].body);

    glColor3f(0.5f, 0.5f, 1.0f);
    objects[1].draw(objects[1].body);

    glDisable(GL_LIGHTING);

    ContactSimplex as, bs;
    Vec3 p0, p1;
    const CollisionDetector::Type state = detector.compare(p0, p1, &as, &bs,
				objects[0].body, objects[1].body);
Vec3 d = p1;
d -= p0;
const Real distance = d.length();
fprintf(stderr, "distance: %g\n", distance);
    if (state == CollisionDetector::Intersecting) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1.0f, 0.0f, 0.0f);
	objects[0].draw(objects[0].body);
	objects[1].draw(objects[1].body);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else /*if (state == CollisionDetector::Contacting)*/ {
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	R_glv(glVertex3)(p0.get());
	R_glv(glVertex3)(p1.get());
	glEnd();

	unsigned int i;
	glColor3f(0.25f, 1.0f, 0.25f);
	for (i = 0; i < as.size(); i++)
	    drawDot(as[i].point, -1);
	glColor3f(0.25f, 0.25f, 1.0f);
	for (i = 0; i < bs.size(); i++)
	    drawDot(bs[i].point, -1);

	glDisable(GL_DEPTH_TEST);
	glColor3f(0.0f, 1.0f, 0.0f);
	drawDot(p0, as.size());
	glColor3f(0.0f, 0.0f, 1.0f);
	drawDot(p1, bs.size());
	glEnable(GL_DEPTH_TEST);
    }

    if (pickedObject != -1) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1.0f, 1.0f, 1.0f);
	objects[pickedObject].draw(objects[pickedObject].body);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glPopMatrix();
    glutSwapBuffers();
}

static void		reshape(int w, int h)
{
    width  = w;
    height = h;
    viewWidth  = 2.0;
    viewHeight = 2.0 * (Real)h / (Real)w;
    viewDepth  = viewHeight;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewWidth,  viewWidth,
	    -viewHeight, viewHeight,
	    -viewDepth,  viewDepth);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

static void		rotateToView(Vec3& p)
{
    p = viewRotation * p;
}

static void		invRotateToView(Vec3& p)
{
    Quaternion invViewRotation(viewRotation);
    invViewRotation.invert();
    p = invViewRotation * p;
}

static void		mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN && (moving || rotating || viewing))
	return;

    Vec3 p(2.0 * viewWidth  * ((Real)x / (Real)width)  - viewWidth,
	  -2.0 * viewHeight * ((Real)y / (Real)height) + viewHeight,
	   2.0 * viewDepth);
    Vec3 d(p[0], p[1], p[2] - 1.0);
    invRotateToView(p);
    invRotateToView(d);
    d -= p;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
	if (++pickedObject == numObjects)
	    pickedObject = 0;
/*
	for (int i = 0; i < numObjects; i++) {
	    float t;
	    if (objects[i]->intersect(&t, p, d)) {
*/
		moving = true;
//		pickedObject = i;
		m0[0] = x;
		m0[1] = y;

		orig = objects[pickedObject].body->getPosition();

		du.set(1.0,  0.0, 0.0);
		dv.set(0.0, -1.0, 0.0);
		invRotateToView(du);
		invRotateToView(dv);
		du *= 2.0 * viewWidth  / (float)width;
		dv *= 2.0 * viewHeight / (float)height;
/*
		break;
	    }
	}
*/
	glutPostRedisplay();
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
	if (moving) {
//	    pickedObject = -1;
	    moving = false;
	}
	glutPostRedisplay();
    }
    else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
/*
	for (int i = 0; i < numObjects; i++) {
	    float t;
	    if (objects[i]->intersect(&t, p, d)) {
*/
		rotating = true;
//		pickedObject = i;
		trackball = new Trackball(
				objects[pickedObject].body->getOrientation(),
				viewRotation,
				0.5 * width, 0.5 * height,
				width / 2.0, width / 2.0,
				x, y);
/*
		break;
	    }
	}
*/
	glutPostRedisplay();
    }
    else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) {
	if (rotating) {
//	    pickedObject = -1;
	    rotating = false;
	    delete trackball;
	    trackball = NULL;
	}
	glutPostRedisplay();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
	viewing = true;
	trackball = new Trackball(viewRotation, Quaternion(),
				0.5 * width, 0.5 * height,
				width / 2.0, width / 2.0,
				x, y);
	glutPostRedisplay();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
	if (viewing) {
	    viewing = false;
	    delete trackball;
	    trackball = NULL;
	}
	glutPostRedisplay();
    }
}

static void		motion(int x, int y)
{
    const Real dx = (Real)(x - m0[0]);
    const Real dy = (Real)(y - m0[1]);

    if (moving) {
	Vec3 v = orig + dx * du + dy * dv;
	objects[pickedObject].body->setPosition(v);
	glutPostRedisplay();
    }
    else if (rotating) {
	objects[pickedObject].body->setOrientation((*trackball)(x, y));
	glutPostRedisplay();
    }
    else if (viewing) {
	viewRotation = (*trackball)(x, y);
	glutPostRedisplay();
    }
}

static void		key(unsigned char c, int /*x*/, int /*y*/)
{
    switch (c) {
      case 27:
	exit (0);
    }
}

//
// main
//

int			main(int argc, char** argv)
{
  glutInit(&argc, argv);

/*
  objects[0].body = new Body(new ShapeBox(0.5, 0.2, 0.3), 1.0);
  objects[0].draw = drawBox;
  objects[1].body = new Body(new ShapeBox(1.0, 1.0, 0.1), 1.0);
  objects[1].draw = drawBox;
//  objects[0].body->setPosition(Vec3(0.689206, -2.75899e-06, -0.693755));
//  objects[0].body->setOrientation(Quaternion(-0.710351, -0.703766, 0.00292813, 0.0103326));
//  objects[0].body->setPosition(Vec3(0.689207, -2.75901e-06, -0.693755));
//  objects[0].body->setOrientation(Quaternion(-0.710351, -0.703766, 0.00292416, 0.0103286));
//  objects[0].body->setPosition(Vec3(0.685294, 2.03564e-06, -0.693318));
//  objects[0].body->setOrientation(Quaternion(-0.707666, -0.705315, 0.0230057, 0.0348061));
  objects[0].body->setPosition(Vec3(0.2104555240, -0.0000000000, -0.6986667763));
  objects[0].body->setOrientation(Quaternion(0.7038593290, 0.7008316341, 0.0815694967, 0.0822403969));
  objects[1].body->setPosition(Vec3(0.0, 0.0, -1.0));
objects[0].body->dump();
*/
/*
  objects[0].body = new Body(new ShapeBox(0.5, 0.5, 0.5), 1.0);
  objects[0].draw = drawBox;
  objects[0].body->setPosition(Vec3(0.7, 0.0, 0.0));
  objects[1].body = new Body(new ShapeBox(0.33, 0.33, 0.33), 1.0);
  objects[1].draw = drawBox;
  objects[1].body->setPosition(Vec3(-1.0, 0.0, 0.0));
*/
/*
  objects[0] = new Box(0.5);
  objects[1] = new Box(0.33);
  objects[0]->setTranslation(Vec3(0.7, -0.2, 0.0));
  objects[1]->setTranslation(Vec3(-1.0, 0.4, 0.0));
  objects[1]->setRotation(Quaternion(Vec3(0.0, 0.0, 1.0), 45.0));
*/
  objects[0].body = new Body(new ShapeBox(0.5, 0.5, 0.5), 1.0);
  objects[0].draw = drawBox;
  objects[0].body->setPosition(Vec3(0.7, 0.0, 0.0));
  objects[1].body = new Body(new ShapePyramid(0.33, 0.33, 0.33), 1.0);
  objects[1].draw = drawPyramid;
  objects[1].body->setPosition(Vec3(-1.0, 0.0, 0.0));
  numObjects = 2;
  pickedObject = 0;

  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(400, 400);
  glutCreateWindow("EGJK");

  glutDisplayFunc(redraw);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(key);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHT0);

  glutMainLoop();
}
