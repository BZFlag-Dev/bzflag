#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "math3D.h"

class Trackball {
  public:
    Trackball(const Quaternion& q0, const Quaternion& vq,
				Real xc, Real yc, Real w, Real h,
				Real x, Real y);
    ~Trackball();

    Quaternion		operator()(Real x, Real y);

  private:
    Real		project(Real x, Real y) const;
    Quaternion		getRotation(Real x, Real y) const;

  private:
    Real		d_xc, d_yc;
    Real		d_w, d_h;
    Real		d_x0, d_y0;
    Quaternion		d_q0;
    Quaternion		d_vq;
    static const Real	s_radius;
};

#endif
// ex: shiftwidth=4 tabstop=4
