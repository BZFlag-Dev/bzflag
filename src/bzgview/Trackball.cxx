#include "Trackball.h"
#include "BzfEvent.h"
#include "common.h"
#include <math.h>

typedef struct timeval	timeval;

//
// Trackball::Quaternion
//

Trackball::Quaternion::Quaternion()
{
	a[0] = 1.0;
	a[1] = a[2] = a[3] = 0.0;
}

Trackball::Quaternion::Quaternion(float c, float i, float j, float k)
{
	a[0] = c;
	a[1] = i;
	a[2] = j;
	a[3] = k;
}

Trackball::Quaternion::Quaternion(float b[4])
{
	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
	a[3] = b[3];
}

Trackball::Quaternion::Quaternion(float axis[3], float angle)
{
	float l = 1.0 / hypot(axis[0], hypot(axis[1], axis[2]));
	l *= sin(angle/2.0);
	a[0] = cos(angle/2.0);
	a[1] = l * axis[0];
	a[2] = l * axis[1];
	a[3] = l * axis[2];
}

Trackball::Quaternion::Quaternion(const Quaternion& q)
{
	a[0] = q.a[0];
	a[1] = q.a[1];
	a[2] = q.a[2];
	a[3] = q.a[3];
}

Trackball::Quaternion&	Trackball::Quaternion::operator=(
								const Quaternion& q)
{
	a[0] = q.a[0];
	a[1] = q.a[1];
	a[2] = q.a[2];
	a[3] = q.a[3];
	return *this;
}

float&					Trackball::Quaternion::operator [] (int i)
{
	return a[i];
}

float					Trackball::Quaternion::operator [] (int i) const
{
	return a[i];
}

void					Trackball::Quaternion::normalize()
{
	float d = 1.0 / hypot(a[0], hypot(a[1], hypot(a[2], a[3])));
	a[0] *= d;
	a[1] *= d;
	a[2] *= d;
	a[3] *= d;
}

float					Trackball::Quaternion::length() const
{
	return hypot(a[0], hypot(a[1], hypot(a[2], a[3])));
}

void					Trackball::Quaternion::matrix(float m[4][4]) const
{
	m[0][0] = 1.0 - 2.0 * (a[2] * a[2] + a[3] * a[3]);
	m[0][1] = 2.0 * (a[1] * a[2] - a[3] * a[0]);
	m[0][2] = 2.0 * (a[3] * a[1] + a[2] * a[0]);
	m[1][0] = 2.0 * (a[1] * a[2] + a[3] * a[0]);
	m[1][1] = 1.0 - 2.0 * (a[3] * a[3] + a[1] * a[1]);
	m[1][2] = 2.0 * (a[2] * a[3] - a[1] * a[0]);
	m[2][0] = 2.0 * (a[3] * a[1] - a[2] * a[0]);
	m[2][1] = 2.0 * (a[2] * a[3] + a[1] * a[0]);
	m[2][2] = 1.0 - 2.0 * (a[1] * a[1] + a[2] * a[2]);
	m[0][3] = m[1][3] = m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.0;
	m[3][3] = 1.0;
}

Trackball::Quaternion&	Trackball::Quaternion::operator *= (const Quaternion& q)
{
	float b[4];

	b[0] = a[0] * q.a[0] - a[1] * q.a[1] - a[2] * q.a[2] - a[3] * q.a[3];
	b[1] = a[2] * q.a[3] - a[3] * q.a[2] + a[0] * q.a[1] + a[1] * q.a[0];
	b[2] = a[3] * q.a[1] - a[1] * q.a[3] + a[0] * q.a[2] + a[2] * q.a[0];
	b[3] = a[1] * q.a[2] - a[2] * q.a[1] + a[0] * q.a[3] + a[3] * q.a[0];
	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
	a[3] = b[3];
	return *this;
}

//
// Trackball
//

#define TRACKBALLSIZE  (0.8)

Trackball::Trackball()
{
	reset();
	resize(0, 0);
}

Trackball::~Trackball()
{
}

float					Trackball::projectToSphere(
								float r, float x, float y) const
{
	float d, t, z;

	d = hypot(x, y);
	if (d < r*M_SQRT1_2)  		// Inside sphere
		z = sqrt(r*r - d*d);
	else { 					// On hyperbola
		t = r / M_SQRT2;
		z = t*t / d;
	}
	return z;
}

void					Trackball::trackball(float p1x, float p1y,
								float p2x, float p2y, float delta[4]) const
{
	float p1[3], p2[3], d[3];

	if (p1x == p2x && p1y == p2y) {
		delta[0] = 1.0;
		delta[1] = delta[2] = delta[3] = 0.0;
		return;
	}

	// get projected coordinates
	p1[0] = p1x;
	p1[1] = p1y;
	p1[2] = projectToSphere(TRACKBALLSIZE, p1x, p1y);
	p2[0] = p2x;
	p2[1] = p2y;
	p2[2] = projectToSphere(TRACKBALLSIZE, p2x, p2y);

	// get axis of rotation (cross product)
	delta[0] = p1[2] * p2[1] - p1[1] * p2[2];
	delta[1] = p1[0] * p2[2] - p1[2] * p2[0];
	delta[2] = p1[1] * p2[0] - p1[0] * p2[1];

	// compute amount of rotation
	d[0] = p1[0] - p2[0];
	d[1] = p1[1] - p2[1];
	d[2] = p1[2] - p2[2];
	delta[3] = hypot(d[0], hypot(d[1], d[2])) / (2.0 * TRACKBALLSIZE);
}

void					Trackball::doFlip(Quaternion& q, float dt)
{
	float t = dt * delta[3];

	// watch out for values out of range
	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	float phi = 2.0 * asin(t);

	// convert to quaternion
	Quaternion d(delta, phi * 0.66);

	// turn
	q *= d;
}

void					Trackball::reset()
{
	rot = Quaternion();
	base = Quaternion();
	delta[0] = 1.0;
	delta[1] = delta[2] = delta[3] = 0.0;
	xlate[0] = xlate[1] = xlate[2] = 0.0;
	spinning = false;
	turning = false;
	panning = false;
	trucking = false;
}

void					Trackball::resize(int w, int h)
{
	wdx = w >> 1;
	wdy = h >> 1;
	wx = wdx;
	wy = wdy;
}

void					Trackball::getMatrix(float* m) const
{
	rot.matrix((float(*)[4])m);
	m[12] = xlate[0];
	m[13] = xlate[1];
	m[14] = xlate[2];
}

static float				timediff(timeval& t1, timeval& t2)
{
	return float(t2.tv_sec - t1.tv_sec) +
		float(t2.tv_usec - t1.tv_usec) / 1000000.0;
}

#define MAXHISTORY countof(history)

bool					Trackball::onEvent(const BzfEvent& event, bool& redraw)
{
	switch (event.type) {
		case BzfEvent::KeyDown:
			if (event.keyDown.button == BzfKeyEvent::LeftMouse) {
				x0 = x;
				y0 = y;
				if (event.keyDown.shift & BzfKeyEvent::ShiftKey) {
					// pan
					panning = true;
					xlate0[0] = xlate[0];
					xlate0[1] = xlate[1];
				}
				else if (event.keyDown.shift & BzfKeyEvent::ControlKey) {
					// zoom
					trucking = true;
					xlate0[2] = xlate[2];
				}
				else {
					// turn
					spinning = false;
					turning = true;
					gettimeofday(&lastMove, NULL);
					for (pHistory = 0; pHistory < MAXHISTORY; pHistory++) {
						history[pHistory][0] = x;
						history[pHistory][1] = y;
						hTime[pHistory]      = lastMove;
					}
					pHistory = 0;
				}
				return true;
			}
			break;

		case BzfEvent::KeyUp:
			if (event.keyUp.button == BzfKeyEvent::LeftMouse) {
				if (turning) {
					base = rot;
					unsigned int i = 1;
					timeval t1;
					gettimeofday(&t1, NULL);
					unsigned int h = (pHistory + MAXHISTORY - 1) % MAXHISTORY;
					while (++i < MAXHISTORY && timediff(hTime[h], t1) < 0.1)
						h = (pHistory + MAXHISTORY - 1) % MAXHISTORY;
					if (i != MAXHISTORY)
						h = (h + 1) % MAXHISTORY;
					if (h != pHistory) {
						spinning = true;
						trackball(history[h][0], history[h][1], x, y, delta);
						delta[3] /= timediff(hTime[h], t1);
						normcount = 0;
						redraw = true;
					}
					turning = false;
				}
				else {
					panning = false;
					trucking = false;
				}
				return true;
			}
			break;

		case BzfEvent::MouseMove:
			x = (float)(event.mouseMove.x - wx) / wdx;
			y = -(float)(event.mouseMove.y - wy) / wdy;
			if (turning) {
				timeval t1;
				gettimeofday(&t1, NULL);
				float dt = timediff(lastMove, t1);
				if (dt > 0.01) {
					lastMove = t1;
					pHistory = (pHistory + 1) % MAXHISTORY;
				}
				history[pHistory][0] = x;
				history[pHistory][1] = y;
				hTime[pHistory]      = t1;
				trackball(x0, y0, x, y, delta);
				rot = base;
				doFlip(rot, 1.0);
				redraw = true;
			}
			else if (panning) {
				xlate[0] = xlate0[0] + 4.0 * (x - x0);
				xlate[1] = xlate0[1] + 4.0 * (y - y0);
			}
			else if (trucking) {
				xlate[2] = xlate0[2] - 4.0 * (y - y0);
			}
			return true;

		default:
			// ignore other events
			break;
	}
	return false;
}

bool					Trackball::isSpinning() const
{
	return spinning;
}

void					Trackball::spin()
{
	if (spinning) {
		timeval t1;
		gettimeofday(&t1, NULL);
		float dt = timediff(lastMove, t1);
		lastMove = t1;
		doFlip(base, dt);
		if (++normcount > 50) {
			normcount = 0;
			base.normalize();
		}
		rot = base;
	}
}
