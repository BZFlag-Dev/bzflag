#include "Trackball2.h"
#include <assert.h>

//
// Trackball
//

const Real		Trackball::s_radius = 0.8;

Trackball::Trackball(const Quaternion& q, const Quaternion& v,
				Real xc, Real yc, Real w, Real h,
				Real x, Real y) :
				d_xc(xc),
				d_yc(yc),
				d_w(1.0 / w),
				d_h(1.0 / h),
				d_x0(x),
				d_y0(y),
				d_q0(q),
				d_vq(v)
{
    d_x0 = -d_w * (x - d_xc);
    d_y0 = -d_h * (d_yc - y);
}

Trackball::~Trackball()
{
    // do nothing
}

Quaternion		Trackball::operator()(Real x, Real y)
{
    d_q0 = d_q0 * getRotation(x, y);

    d_x0 = -d_w * (x - d_xc);
    d_y0 = -d_h * (d_yc - y);

    return d_q0;
}

Real			Trackball::project(Real x, Real y) const
{
    const Real d = hypotf(x, y);
    if (d < s_radius * M_SQRT1_2) 		// inside sphere
	return sqrtf(s_radius * s_radius - d * d);

    // on hyperbola
    const Real t = s_radius / M_SQRT2;
    return t * t / d;
}

Quaternion		Trackball::getRotation(Real x, Real y) const
{
    if (x == d_x0 && y == d_y0) return Quaternion();

    x = -d_w * (x - d_xc);
    y = -d_h * (d_yc - y);

    // get projected coordinates
    Vec3 v1(s_radius * d_x0, s_radius * d_y0,
	    project(s_radius * d_x0, s_radius * d_y0));
    Vec3 v2(s_radius * x, s_radius * y,
	    project(s_radius * x, s_radius * y));

    // get axis of rotation (cross product)
    Vec3 a(v2);
    a %= v1;

    // compute amount of rotation
    v1 -= v2;
    Real t = v1.length() / (2.0 * s_radius);

    // watch out for values out of range
    if (t > 1.0) t = 1.0;
    else if (t < -1.0) t = -1.0;
    const Real phi = 2.0 * asinf(t);

    // convert to quaternion
    return Quaternion(a, phi * 0.99 * 180.0 / M_PI);
}
// ex: shiftwidth=4 tabstop=4
