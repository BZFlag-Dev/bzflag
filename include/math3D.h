/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_MATH3D_H
#define BZF_MATH3D_H

// decide if compiler supports member templates properly.  VC++ 6.0
// and earlier do not.
#define BZF_MTE
#if defined(_MSC_VER)
#undef BZF_MTE
#endif

#include "mathr.h"

class Matrix;
class Quaternion;

template<class A>
class VecExpr;
template<class A>
class MatrixExpr;
template<class A>
class QuatExpr;

class Vec3 {
public:
	typedef const Real* iterator;

	Vec3();
	Vec3(Real x, Real y, Real z);
	Vec3(const float*);
	Vec3(const double*);

	// manipulators

	Vec3&				operator=(const Vec3&);

	Vec3&				operator+=(const Vec3&);
	Vec3&				operator-=(const Vec3&);
	Vec3&				operator%=(const Vec3&);		// cross product
	Vec3&				operator*=(Real);

#if defined(BZF_MTE)
	template<class A>
	Vec3(VecExpr<A>);
	template<class A>
	Vec3&				operator=(VecExpr<A>);
	template<class A>
	Vec3&				operator+=(VecExpr<A>);
	template<class A>
	Vec3&				operator-=(VecExpr<A>);
	template<class A>
	Vec3&				operator%=(VecExpr<A>);			// cross product
#endif

	// transform as a homogeneous point with w = 1 and discard the
	// transformed w.
	Vec3&				xformPoint(const Matrix&);

	// transform as a homogeneous point with given w.  x,y,z are
	// divided by the transformed w then w is discarded.
	Vec3&				xformPoint(const Matrix&, Real w);

	Vec3&				normalize();
	Vec3&				negate();

	Vec3&				zero();
	Vec3&				set(Real x, Real y, Real z);
	Real&				operator[](unsigned int index);

	// accessors

	Real				operator[](unsigned int index) const;
	iterator			begin() const { return v; }
	const Real*			get() const { return v; }
	Real				length() const;
	Real				length2() const;				// length squared

	bool				operator==(const Vec3&) const;
	bool				operator!=(const Vec3&) const;

private:
	Real				v[3];
};

class Matrix {
public:
	typedef const Real* iterator;

	Matrix(); // identity, not zero
	Matrix(const Quaternion&);

	// manipulators

	Matrix&				operator=(const Quaternion&);

	Matrix&				operator*=(Real);
	Matrix&				operator*=(const Matrix&);

#if defined(BZF_MTE)
	template<class A>
	Matrix(MatrixExpr<A>);
	template<class A>
	Matrix(QuatExpr<A>);
	template<class A>
	Matrix&				operator=(MatrixExpr<A>);
	template<class A>
	Matrix&				operator=(QuatExpr<A>);
	template<class A>
	Matrix&				operator*=(MatrixExpr<A>);
#else
	Matrix(const float*);
	Matrix(const double*);
#endif

	Matrix&				transpose();
	Matrix&				invert();

	Matrix&				zero();
	Matrix&				identity();
	Matrix&				setRotate(Real x, Real y, Real z, Real a);
	Matrix&				setScale(Real x, Real y, Real z);
	Matrix&				setTranslate(Real x, Real y, Real z);
	Real&				operator[](unsigned int index);

	// accessors

	Real				operator[](unsigned int index) const;
	iterator			begin() const { return m; }
	const Real*			get() const { return m; }
	float*				get(float* buffer) const;
	double*				get(double* buffer) const;

	// transform a homogeneous vector.  src and dst can be the same.
	void				xformPoint(float* dst, const float* src) const;
	void				xformPoint(double* dst, const double* src) const;

private:
	Real				m[16];
};

class Quaternion {
public:
	typedef const Real* iterator;

	Quaternion();
	Quaternion(const Quaternion&);
	Quaternion(Real r, Real i, Real j, Real k);
	Quaternion(const Vec3&);	// r == 0.0
	Quaternion(const Vec3& axis, Real angle);
	~Quaternion();

	// manipulators

	Quaternion&			operator=(const Quaternion&);
	Quaternion&			operator=(const Vec3&);	// r == 0.0

	Quaternion&			operator*=(const Quaternion&);
	Quaternion&			operator*=(Real);

#if defined(BZF_MTE)
	template<class A>
	Quaternion(QuatExpr<A>);
	template<class A>
	Quaternion&			operator=(QuatExpr<A>);
#endif

	Quaternion&			invert();
	Quaternion&			normalize();
	Quaternion&			negate();

	Quaternion&			zero();
	Quaternion&			identity();
	Quaternion&			set(Real r, Real i, Real j, Real k);
	Real&				operator[](unsigned int index);

	// accessors

	Real				operator[](unsigned int index) const;
	iterator			begin() const { return v; }
	const Real*			get() const { return v; }
	Real				length() const;

private:
	Real				v[4];	// r, i, j, k
};

class Plane {
public:
	Plane();	// +z half-space
	Plane(const Vec3& normal, Real d);
	Plane(const Vec3& normal, const Vec3& pointOnPlane);
	// normal points out if v0,v1,v2 are CCW
	Plane(const Vec3& v0, const Vec3& v1, const Vec3& v2);

	// manipulators

	Plane&				negate();

	Plane&				set(const Vec3& normal, Real d);
	Plane&				set(const Vec3& normal, const Vec3& pointOnPlane);
	Plane&				set(const Vec3& v0, const Vec3& v1, const Vec3& v2);

	// accessors

	const Vec3&			getNormal() const;
	Real				getOffset() const;

	// projection
	Real				distance(const Vec3&) const;
	void				projectPoint(Vec3&, const Vec3&) const;
	void				projectVector(Vec3&, const Vec3&) const;

private:
	Vec3				normal;
	Real				d;
};

class Ray {
public:
	Ray();
	Ray(const Vec3& origin, const Vec3& direction);
	Ray(const float* origin, const float* direction);
	Ray(const double* origin, const double* direction);

	const Vec3&			getOrigin() const;
	const Vec3&			getDirection() const;
	void				getPoint(Real t, float p[3]) const;
	void				getPoint(Real t, double p[3]) const;

private:
	Vec3				o;
	Vec3				d;
};


//
// Vec3
//

inline
Vec3::Vec3()
{
    v[0] = v[1] = v[2] = R_(0.0);
}

inline
Vec3::Vec3(Real x, Real y, Real z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

inline
Vec3::Vec3(const float* a)
{
    v[0] = static_cast<Real>(a[0]);
    v[1] = static_cast<Real>(a[1]);
    v[2] = static_cast<Real>(a[2]);
}

inline
Vec3::Vec3(const double* a)
{
    v[0] = static_cast<Real>(a[0]);
    v[1] = static_cast<Real>(a[1]);
    v[2] = static_cast<Real>(a[2]);
}

inline
Vec3&					Vec3::operator=(const Vec3& v_)
{
    v[0] = v_.v[0];
    v[1] = v_.v[1];
    v[2] = v_.v[2];
    return *this;
}

inline
Vec3&					Vec3::operator+=(const Vec3& v_)
{
    v[0] += v_.v[0];
    v[1] += v_.v[1];
    v[2] += v_.v[2];
    return *this;
}

inline
Vec3&					Vec3::operator-=(const Vec3& v_)
{
    v[0] -= v_.v[0];
    v[1] -= v_.v[1];
    v[2] -= v_.v[2];
    return *this;
}

inline
Vec3&					Vec3::operator%=(const Vec3& v_)
{
    const Real a = v[1] * v_.v[2] - v[2] * v_.v[1];
    const Real b = v[2] * v_.v[0] - v[0] * v_.v[2];
    v[2]         = v[0] * v_.v[1] - v[1] * v_.v[0];
    v[0]         = a;
    v[1]         = b;
    return *this;
}

inline
Vec3&					Vec3::operator*=(Real a)
{
    v[0] *= a;
    v[1] *= a;
    v[2] *= a;
    return *this;
}

#if defined(BZF_MTE)

template<class A>
inline
Vec3::Vec3(VecExpr<A> expr)
{
	operator=(expr);
}

template<class A>
inline
Vec3&					Vec3::operator=(VecExpr<A> expr)
{
	Real t[2];
	t[0] = expr[0];
	t[1] = expr[1];
	v[2] = expr[2];
	v[0] = t[0];
	v[1] = t[1];
	return *this;
}

template<class A>
inline
Vec3&					Vec3::operator+=(VecExpr<A> x)
{
    v[0] += x[0];
    v[1] += x[1];
    v[2] += x[2];
    return *this;
}

template<class A>
inline
Vec3&					Vec3::operator-=(VecExpr<A> x)
{
    v[0] -= x[0];
    v[1] -= x[1];
    v[2] -= x[2];
    return *this;
}

template<class A>
inline
Vec3&					Vec3::operator%=(VecExpr<A> x)
{
	Real t[3];
	t[0] = x[0];
	t[1] = x[1];
	t[2] = x[2];
    const Real a = v[1] * t[2] - v[2] * t[1];
    const Real b = v[2] * t[0] - v[0] * t[2];
    v[2]         = v[0] * t[1] - v[1] * t[0];
    v[0]         = a;
    v[1]         = b;
    return *this;
}

#endif // defined(BZF_MTE)

inline
Vec3&					Vec3::zero()
{
    v[0] = R_(0.0);
    v[1] = R_(0.0);
    v[2] = R_(0.0);
    return *this;
}

inline
Vec3&					Vec3::set(Real x, Real y, Real z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
    return *this;
}

inline
Real&					Vec3::operator[](unsigned int i)
{
    assert(i <= 2);
    return v[i];
}

inline
Real					Vec3::operator[](unsigned int i) const
{
    assert(i <= 2);
    return v[i];
}

inline
Real					Vec3::length() const
{
    return sqrtr(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

inline
Real					Vec3::length2() const
{
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

inline
Vec3&					Vec3::normalize()
{
    return (*this) *= R_(1.0) / length();
}

inline
Vec3&					Vec3::negate()
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
    return *this;
}

inline
bool					Vec3::operator==(const Vec3& v_) const
{
    return (v[0] == v_.v[0] && v[1] == v_.v[1] && v[2] == v_.v[2]);
}

inline
bool					Vec3::operator!=(const Vec3& v_) const
{
    return (v[0] != v_.v[0] || v[1] != v_.v[1] || v[2] != v_.v[2]);
}


#if defined(BZF_MTE)

//
// member template implementation of 3D math
//

//
// vector expressions
//

template<class Expr>
class VecExpr {
public:
	VecExpr(const Expr& expr_) : expr(expr_) { }

	Real				operator[](unsigned int i) const { return expr[i]; }

private:
	Expr				expr;
};

class VecExprConstant {
public:
	VecExprConstant(Real c_) : c(c_) { }

	Real				operator[](unsigned int) const { return c; }

private:
	Real				c;
};

template<class A, class Op>
class VecExprUOp {
public:
	VecExprUOp(const A& a_, const Op& op_) :
								a(a_), op(op_) { }

	Real				operator[](unsigned int i) const
								{ return op.apply(a[i]); }

private:
	A					a;
	Op					op;
};

template<class A, class B, class Op>
class VecExprBOp {
public:
	VecExprBOp(const A& a_, const B& b_, const Op& op_) :
								a(a_), b(b_), op(op_) { }

	Real				operator[](unsigned int i) const
								{ return op.apply(a[i], b[i]); }

private:
	A					a;
	B					b;
	Op					op;
};

template<class A, class B>
class VecCrossExpr {
public:
	VecCrossExpr(const A& a_, const B& b_) : ready(false), a(a_), b(b_) { }

	Real				operator[](unsigned int i) const
	{
		if (!ready)
			init();
		switch (i) {
			case 0: return u[1] * v[2] - u[2] * v[1];
			case 1: return u[2] * v[0] - u[0] * v[2];
			case 2: return u[0] * v[1] - u[1] * v[0];
		}
		return R_(0.0);
	}

private:
	void				init() const
	{
		u[0]  = a[0];
		u[1]  = a[1];
		u[2]  = a[2];
		v[0]  = b[0];
		v[1]  = b[1];
		v[2]  = b[2];
		ready = true;
	}

private:
	mutable bool		ready;
	mutable Real		u[3], v[3];
	A					a;
	B					b;
};

template<class A, class B>
class MatrixVecMultExpr {
public:
	MatrixVecMultExpr(const A& a_, const B& b_) : a(a_), b(b_) { }

	Real				operator[](unsigned int i) const
	{
		return	a[i + 0] * b[0] +
				a[i + 4] * b[1] +
				a[i + 8] * b[2];
	}

private:
	A					a;
	B					b;
};

template<class A, class B>
class QuatVecMultExpr {
public:
	QuatVecMultExpr(const A& a_, const B& b_) : ready(false), a(a_), b(b_) { }

	Real				operator[](unsigned int i) const
	{
		if (!ready)
			init();
		if (i == 0)
			return qv[0] * q[1] + q[0] * qv[1] + qv[2] * q[3] - qv[3] * q[2];
		if (i == 1)
			return qv[0] * q[2] + q[0] * qv[2] + qv[3] * q[1] - qv[1] * q[3];
		return qv[0] * q[3] + q[0] * qv[3] + qv[1] * q[2] - qv[2] * q[1];
	}

private:
	void				init() const
	{
		Real v[3];
		v[0]  = b[0];
		v[1]  = b[1];
		v[2]  = b[2];
		qv[0] = a[0];
		qv[1] = a[1];
		qv[2] = a[2];
		qv[3] = a[3];
		q[0]  = v[0] * qv[1] + v[1] * qv[2] + v[2] * qv[3];
		q[1]  = v[0] * qv[0] + v[2] * qv[2] - v[1] * qv[3];
		q[2]  = v[1] * qv[0] + v[0] * qv[3] - v[2] * qv[1];
		q[3]  = v[2] * qv[0] + v[1] * qv[1] - v[0] * qv[2];
		ready = true;
	}

private:
	mutable bool		ready;
	mutable Real		q[4], qv[4];
	A					a;
	B					b;
};


//
// matrix expressions
//

template<class Expr>
class MatrixExpr {
public:
	MatrixExpr(const Expr& expr_) : expr(expr_) { }

	Real				operator[](unsigned int i) const
								{ return static_cast<Real>(expr[i]); }

private:
	Expr				expr;
};

typedef MatrixExpr<const float*> FloatMatrix;
typedef MatrixExpr<const double*> DoubleMatrix;
typedef MatrixExpr<const Real*> RealMatrix;

template<class A, class Op>
class MatrixExprUOp {
public:
	MatrixExprUOp(const A& a_, const Op& op_) :
								a(a_), op(op_) { }

	Real				operator[](unsigned int i) const
								{ return op.apply(a[i]); }

private:
	A					a;
	Op					op;
};

template<class A, class B, class Op>
class MatrixExprBOp {
public:
	MatrixExprBOp(const A& a_, const B& b_, const Op& op_) :
								a(a_), b(b_), op(op_) { }

	Real				operator[](unsigned int i) const
								{ return op.apply(a[i], b[i]); }

private:
	A					a;
	B					b;
	Op					op;
};

template<class A>
class MatrixFromQuatExpr {
public:
	MatrixFromQuatExpr(const A& a_) : ready(false), a(a_) { }

	Real				operator[](unsigned int i) const;

private:
	void				init() const;

private:
	mutable bool		ready;
	mutable Real		q[4];
	A					a;
};

template<class A>
Real					MatrixFromQuatExpr<A>::operator[](unsigned int i) const
{
	if (!ready)
		init();
	switch (i) {
		case 0:  return R_(1.0) - R_(2.0) * (q[2] * q[2] + q[3] * q[3]);
		case 1:  return R_(2.0) * (q[1] * q[2] + q[3] * q[0]);
		case 2:  return R_(2.0) * (q[3] * q[1] - q[2] * q[0]);
		case 3:  return R_(0.0);
		case 4:  return R_(2.0) * (q[1] * q[2] - q[3] * q[0]);
		case 5:  return R_(1.0) - R_(2.0) * (q[3] * q[3] + q[1] * q[1]);
		case 6:  return R_(2.0) * (q[2] * q[3] + q[1] * q[0]);
		case 7:  return R_(0.0);
		case 8:  return R_(2.0) * (q[3] * q[1] + q[2] * q[0]);
		case 9:  return R_(2.0) * (q[2] * q[3] - q[1] * q[0]);
		case 10: return R_(1.0) - R_(2.0) * (q[1] * q[1] + q[2] * q[2]);
		case 11: return R_(0.0);
		case 12: return R_(0.0);
		case 13: return R_(0.0);
		case 14: return R_(0.0);
		case 15: return R_(1.0);
	}
	return R_(0.0);
}

template<class A>
void					MatrixFromQuatExpr<A>::init() const
{
	q[0]  = a[0];
	q[1]  = a[1];
	q[2]  = a[2];
	q[3]  = a[3];
	ready = true;
}

template<class A, class B>
class MatrixMultExpr {
public:
	MatrixMultExpr(const A& a_, const B& b_) : a(a_), b(b_) { }

	Real				operator[](unsigned int i) const
	{
		const unsigned int r = i & 3;
		const unsigned int c = i - r;
		return	a[r + 0]  * b[c + 0] +
				a[r + 4]  * b[c + 1] +
				a[r + 8]  * b[c + 2] +
				a[r + 12] * b[c + 3];
	}

private:
	A					a;
	B					b;
};


//
// quaternion expressions
//

template<class Expr>
class QuatExpr {
public:
	QuatExpr(const Expr& expr_) : expr(expr_) { }

	Real				operator[](unsigned int i) const
								{ return static_cast<Real>(expr[i]); }

private:
	Expr				expr;
};

template<class A, class Op>
class QuatExprUOp {
public:
	QuatExprUOp(const A& a_, const Op& op_) :
								a(a_), op(op_) { }

	Real				operator[](unsigned int i) const
								{ return op.apply(a[i]); }

private:
	A					a;
	Op					op;
};

template<class A, class B, class Op>
class QuatExprBOp {
public:
	QuatExprBOp(const A& a_, const B& b_, const Op& op_) :
								a(a_), b(b_), op(op_) { }

	Real				operator[](unsigned int i) const
								{ return op.apply(a[i], b[i]); }

private:
	A					a;
	B					b;
	Op					op;
};

template<class A, class B>
class QuatMultExpr {
public:
	QuatMultExpr(const A& a_, const B& b_) : ready(false), a(a_), b(b_) { }

	Real				operator[](unsigned int i) const
	{
		if (!ready)
			init();
		switch (i) {
			case 0:
				return q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3];
			case 1:
				return q1[0] * q2[1] + q2[0] * q1[1] + q1[2] * q2[3] - q1[3] * q2[2];
			case 2:
				return q1[0] * q2[2] + q2[0] * q1[2] + q1[3] * q2[1] - q1[1] * q2[3];
			case 3:
				return q1[0] * q2[3] + q2[0] * q1[3] + q1[1] * q2[2] - q1[2] * q2[1];
		}
		return R_(0.0);
	}

private:
	void				init() const
	{
		q1[0] = a[0];
		q1[1] = a[1];
		q1[2] = a[2];
		q1[3] = a[3];
		q2[0] = b[0];
		q2[1] = b[1];
		q2[2] = b[2];
		q2[3] = b[3];
		ready = true;
	}

private:
	mutable bool		ready;
	mutable Real		q1[4], q2[4];
	A					a;
	B					b;
};

template<class A>
class QuatFromVecExpr {
public:
	QuatFromVecExpr(const A& a_) : a(a_) { }

	Real				operator[](unsigned int i) const
	{
		if (i == 0)
			return R_(0.0);
		else
			return a[i - 1];
	}

private:
	A					a;
};

//
// component operators
//

class VecUExprPlus {
public:
	static Real apply(Real a) { return a; }
};

class VecUExprMinus {
public:
	static Real apply(Real a) { return -a; }
};

class VecBExprAdd {
public:
	static Real apply(Real a, Real b) { return a + b; }
};

class VecBExprSubtract {
public:
	static Real apply(Real a, Real b) { return a - b; }
};

class VecBExprMultiply {
public:
	static Real apply(Real a, Real b) { return a * b; }
};

//
// vector operators
//

// vector unary plus
inline
VecExpr<VecExprUOp<Vec3::iterator, VecUExprPlus> >
operator+(const Vec3& a)
{
	typedef VecExprUOp<Vec3::iterator, VecUExprPlus> T;
	return VecExpr<T>(T(a.begin(), VecUExprPlus()));
}
template<class A>
inline
VecExpr<VecExprUOp<VecExpr<A>, VecUExprPlus> >
operator+(const VecExpr<A>& a)
{
	typedef VecExprUOp<VecExpr<A>, VecUExprPlus> T;
	return VecExpr<T>(T(a, VecUExprPlus()));
}

// vector unary minus
inline
VecExpr<VecExprUOp<Vec3::iterator, VecUExprMinus> >
operator-(const Vec3& a)
{
	typedef VecExprUOp<Vec3::iterator, VecUExprMinus> T;
	return VecExpr<T>(T(a.begin(), VecUExprMinus()));
}
template<class A>
inline
VecExpr<VecExprUOp<VecExpr<A>, VecUExprMinus> >
operator-(const VecExpr<A>& a)
{
	typedef VecExprUOp<VecExpr<A>, VecUExprMinus> T;
	return VecExpr<T>(T(a, VecUExprMinus()));
}

// vector sum
inline
VecExpr<VecExprBOp<Vec3::iterator, Vec3::iterator, VecBExprAdd> >
operator+(const Vec3& a, const Vec3& b)
{
	typedef VecExprBOp<Vec3::iterator, Vec3::iterator, VecBExprAdd> T;
	return VecExpr<T>(T(a.begin(), b.begin(), VecBExprAdd()));
}
template<class B>
inline
VecExpr<VecExprBOp<Vec3::iterator, VecExpr<B>, VecBExprAdd> >
operator+(const Vec3& a, const VecExpr<B>& b)
{
	typedef VecExprBOp<Vec3::iterator, VecExpr<B>, VecBExprAdd> T;
	return VecExpr<T>(T(a.begin(), b, VecBExprAdd()));
}
template<class A>
inline
VecExpr<VecExprBOp<VecExpr<A>, Vec3::iterator, VecBExprAdd> >
operator+(const VecExpr<A>& a, const Vec3& b)
{
	typedef VecExprBOp<VecExpr<A>, Vec3::iterator, VecBExprAdd> T;
	return VecExpr<T>(T(a, b.begin(), VecBExprAdd()));
}
template<class A, class B>
inline
VecExpr<VecExprBOp<VecExpr<A>, VecExpr<B>, VecBExprAdd> >
operator+(const VecExpr<A>& a, const VecExpr<B>& b)
{
	typedef VecExprBOp<VecExpr<A>, VecExpr<B>, VecBExprAdd> T;
	return VecExpr<T>(T(a, b, VecBExprAdd()));
}

// vector difference
inline
VecExpr<VecExprBOp<Vec3::iterator, Vec3::iterator, VecBExprSubtract> >
operator-(const Vec3& a, const Vec3& b)
{
	typedef VecExprBOp<Vec3::iterator, Vec3::iterator, VecBExprSubtract> T;
	return VecExpr<T>(T(a.begin(), b.begin(), VecBExprSubtract()));
}
template<class B>
inline
VecExpr<VecExprBOp<Vec3::iterator, VecExpr<B>, VecBExprSubtract> >
operator-(const Vec3& a, const VecExpr<B>& b)
{
	typedef VecExprBOp<Vec3::iterator, VecExpr<B>, VecBExprSubtract> T;
	return VecExpr<T>(T(a.begin(), b, VecBExprSubtract()));
}
template<class A>
inline
VecExpr<VecExprBOp<VecExpr<A>, Vec3::iterator, VecBExprSubtract> >
operator-(const VecExpr<A>& a, const Vec3& b)
{
	typedef VecExprBOp<VecExpr<A>, Vec3::iterator, VecBExprSubtract> T;
	return VecExpr<T>(T(a, b.begin(), VecBExprSubtract()));
}
template<class A, class B>
inline
VecExpr<VecExprBOp<VecExpr<A>, VecExpr<B>, VecBExprSubtract> >
operator-(const VecExpr<A>& a, const VecExpr<B>& b)
{
	typedef VecExprBOp<VecExpr<A>, VecExpr<B>, VecBExprSubtract> T;
	return VecExpr<T>(T(a, b, VecBExprSubtract()));
}

// constant product
inline
VecExpr<VecExprBOp<VecExprConstant, Vec3::iterator, VecBExprMultiply> >
operator*(Real c, const Vec3& a)
{
	typedef VecExprBOp<VecExprConstant, Vec3::iterator, VecBExprMultiply> T;
	return VecExpr<T>(T(VecExprConstant(c), a.begin(), VecBExprMultiply()));
}
template<class A>
inline
VecExpr<VecExprBOp<VecExprConstant, VecExpr<A>, VecBExprMultiply> >
operator*(Real c, const VecExpr<A>& a)
{
	typedef VecExprBOp<VecExprConstant, VecExpr<A>, VecBExprMultiply> T;
	return VecExpr<T>(T(VecExprConstant(c), a, VecBExprMultiply()));
}
inline
VecExpr<VecExprBOp<Vec3::iterator, VecExprConstant, VecBExprMultiply> >
operator*(const Vec3& a, Real c)
{
	typedef VecExprBOp<Vec3::iterator, VecExprConstant, VecBExprMultiply> T;
	return VecExpr<T>(T(a.begin(), VecExprConstant(c), VecBExprMultiply()));
}
template<class A>
inline
VecExpr<VecExprBOp<VecExpr<A>, VecExprConstant, VecBExprMultiply> >
operator*(const VecExpr<A>& a, Real c)
{
	typedef VecExprBOp<VecExpr<A>, VecExprConstant, VecBExprMultiply> T;
	return VecExpr<T>(T(a, VecExprConstant(c), VecBExprMultiply()));
}

// dot product
inline
Real
operator*(const Vec3& a, const Vec3& b)
{
	Vec3::iterator i = a.begin();
	Vec3::iterator j = b.begin();
	return i[0] * j[0] + i[1] * j[1] + i[2] * j[2];
}
template<class A>
inline
Real
operator*(const Vec3& a, const VecExpr<A>& j)
{
	Vec3::iterator i = a.begin();
	return i[0] * j[0] + i[1] * j[1] + i[2] * j[2];
}
template<class A>
inline
Real
operator*(const VecExpr<A>& i, const Vec3& b)
{
	Vec3::iterator j = b.begin();
	return i[0] * j[0] + i[1] * j[1] + i[2] * j[2];
}
template<class A, class B>
inline
Real
operator*(const VecExpr<A>& i, const VecExpr<B>& j)
{
	return i[0] * j[0] + i[1] * j[1] + i[2] * j[2];
}

// cross product (would prefer ^ but it has wrong precedence)
inline
VecExpr<VecCrossExpr<Vec3::iterator, Vec3::iterator> >
operator%(const Vec3& a, const Vec3& b)
{
	typedef VecCrossExpr<Vec3::iterator, Vec3::iterator> T;
	return VecExpr<T>(T(a.begin(), b.begin()));
}

template<class B>
inline
VecExpr<VecCrossExpr<Vec3::iterator, VecExpr<B> > >
operator%(const Vec3& a, const VecExpr<B>& b)
{
	typedef VecCrossExpr<Vec3::iterator, VecExpr<B> > T;
	return VecExpr<T>(T(a.begin(), b));
}

template<class A>
inline
VecExpr<VecCrossExpr<VecExpr<A>, Vec3::iterator> >
operator%(const VecExpr<A>& a, const Vec3& b)
{
	typedef VecCrossExpr<VecExpr<A>, Vec3::iterator> T;
	return VecExpr<T>(T(a, b.begin()));
}

template<class A, class B>
inline
VecExpr<VecCrossExpr<VecExpr<A>, VecExpr<B> > >
operator%(const VecExpr<A>& a, const VecExpr<B>& b)
{
	typedef VecCrossExpr<VecExpr<A>, VecExpr<B> > T;
	return VecExpr<T>(T(a, b));
}


//
// matrix operators
//

// matrix unary plus
inline
MatrixExpr<MatrixExprUOp<Matrix::iterator, VecUExprPlus> >
operator+(const Matrix& a)
{
	typedef MatrixExprUOp<Matrix::iterator, VecUExprPlus> T;
	return MatrixExpr<T>(T(a.begin(), VecUExprPlus()));
}
template<class A>
inline
MatrixExpr<MatrixExprUOp<MatrixExpr<A>, VecUExprPlus> >
operator+(const MatrixExpr<A>& a)
{
	typedef MatrixExprUOp<MatrixExpr<A>, VecUExprPlus> T;
	return MatrixExpr<T>(T(a, VecUExprPlus()));
}

// matrix unary minus
inline
MatrixExpr<MatrixExprUOp<Matrix::iterator, VecUExprMinus> >
operator-(const Matrix& a)
{
	typedef MatrixExprUOp<Matrix::iterator, VecUExprMinus> T;
	return MatrixExpr<T>(T(a.begin(), VecUExprMinus()));
}
template<class A>
inline
MatrixExpr<MatrixExprUOp<MatrixExpr<A>, VecUExprMinus> >
operator-(const MatrixExpr<A>& a)
{
	typedef MatrixExprUOp<MatrixExpr<A>, VecUExprMinus> T;
	return MatrixExpr<T>(T(a, VecUExprMinus()));
}

// matrix sum
inline
MatrixExpr<MatrixExprBOp<Matrix::iterator, Matrix::iterator, VecBExprAdd> >
operator+(const Matrix& a, const Matrix& b)
{
	typedef MatrixExprBOp<Matrix::iterator, Matrix::iterator, VecBExprAdd> T;
	return MatrixExpr<T>(T(a.begin(), b.begin(), VecBExprAdd()));
}
template<class B>
inline
MatrixExpr<MatrixExprBOp<Matrix::iterator, MatrixExpr<B>, VecBExprAdd> >
operator+(const Matrix& a, const MatrixExpr<B>& b)
{
	typedef MatrixExprBOp<Matrix::iterator, MatrixExpr<B>, VecBExprAdd> T;
	return MatrixExpr<T>(T(a.begin(), b, VecBExprAdd()));
}
template<class A>
inline
MatrixExpr<MatrixExprBOp<MatrixExpr<A>, Matrix::iterator, VecBExprAdd> >
operator+(const MatrixExpr<A>& a, const Matrix& b)
{
	typedef MatrixExprBOp<MatrixExpr<A>, Matrix::iterator, VecBExprAdd> T;
	return MatrixExpr<T>(T(a, b.begin(), VecBExprAdd()));
}
template<class A, class B>
inline
MatrixExpr<MatrixExprBOp<MatrixExpr<A>, MatrixExpr<B>, VecBExprAdd> >
operator+(const MatrixExpr<A>& a, const MatrixExpr<B>& b)
{
	typedef MatrixExprBOp<MatrixExpr<A>, MatrixExpr<B>, VecBExprAdd> T;
	return MatrixExpr<T>(T(a, b, VecBExprAdd()));
}

// matrix difference
inline
MatrixExpr<MatrixExprBOp<Matrix::iterator, Matrix::iterator, VecBExprSubtract> >
operator-(const Matrix& a, const Matrix& b)
{
	typedef MatrixExprBOp<Matrix::iterator, Matrix::iterator, VecBExprSubtract> T;
	return MatrixExpr<T>(T(a.begin(), b.begin(), VecBExprSubtract()));
}
template<class B>
inline
MatrixExpr<MatrixExprBOp<Matrix::iterator, MatrixExpr<B>, VecBExprSubtract> >
operator-(const Matrix& a, const MatrixExpr<B>& b)
{
	typedef MatrixExprBOp<Matrix::iterator, MatrixExpr<B>, VecBExprSubtract> T;
	return MatrixExpr<T>(T(a.begin(), b, VecBExprSubtract()));
}
template<class A>
inline
MatrixExpr<MatrixExprBOp<MatrixExpr<A>, Matrix::iterator, VecBExprSubtract> >
operator-(const MatrixExpr<A>& a, const Matrix& b)
{
	typedef MatrixExprBOp<MatrixExpr<A>, Matrix::iterator, VecBExprSubtract> T;
	return MatrixExpr<T>(T(a, b.begin(), VecBExprSubtract()));
}
template<class A, class B>
inline
MatrixExpr<MatrixExprBOp<MatrixExpr<A>, MatrixExpr<B>, VecBExprSubtract> >
operator-(const MatrixExpr<A>& a, const MatrixExpr<B>& b)
{
	typedef MatrixExprBOp<MatrixExpr<A>, MatrixExpr<B>, VecBExprSubtract> T;
	return MatrixExpr<T>(T(a, b, VecBExprSubtract()));
}

// matrix multiply
inline
MatrixExpr<MatrixMultExpr<Matrix::iterator, Matrix::iterator> >
operator*(const Matrix& a, const Matrix& b)
{
	typedef MatrixMultExpr<Matrix::iterator, Matrix::iterator> T;
	return MatrixExpr<T>(T(a.begin(), b.begin()));
}
template<class B>
inline
MatrixExpr<MatrixMultExpr<Matrix::iterator, MatrixExpr<B> > >
operator*(const Matrix& a, const MatrixExpr<B>& b)
{
	typedef MatrixMultExpr<Matrix::iterator, MatrixExpr<B> > T;
	return MatrixExpr<T>(T(a.begin(), b));
}
template<class A>
inline
MatrixExpr<MatrixMultExpr<MatrixExpr<A>, Matrix::iterator> >
operator*(const MatrixExpr<A>& a, const Matrix& b)
{
	typedef MatrixMultExpr<MatrixExpr<A>, Matrix::iterator> T;
	return MatrixExpr<T>(T(a, b.begin()));
}
template<class A, class B>
inline
MatrixExpr<MatrixMultExpr<MatrixExpr<A>, MatrixExpr<B> > >
operator*(const MatrixExpr<A>& a, const MatrixExpr<B>& b)
{
	typedef MatrixMultExpr<MatrixExpr<A>, MatrixExpr<B> > T;
	return MatrixExpr<T>(T(a, b));
}
inline
MatrixExpr<MatrixMultExpr<MatrixFromQuatExpr<Quaternion::iterator>, Matrix::iterator> >
operator*(const Quaternion& a, const Matrix& b)
{
	typedef MatrixMultExpr<MatrixFromQuatExpr<Quaternion::iterator>, Matrix::iterator> T;
	return MatrixExpr<T>(T(MatrixFromQuatExpr<Quaternion::iterator>(a.begin()), b.begin()));
}
template<class B>
inline
MatrixExpr<MatrixMultExpr<MatrixFromQuatExpr<Quaternion::iterator>, MatrixExpr<B> > >
operator*(const Quaternion& a, const MatrixExpr<B>& b)
{
	typedef MatrixMultExpr<MatrixFromQuatExpr<Quaternion::iterator>, MatrixExpr<B> > T;
	return MatrixExpr<T>(T(MatrixFromQuatExpr<Quaternion::iterator>(a.begin()), b));
}
inline
MatrixExpr<MatrixMultExpr<Matrix::iterator, MatrixFromQuatExpr<Quaternion::iterator> > >
operator*(const Matrix& a, const Quaternion& b)
{
	typedef MatrixMultExpr<Matrix::iterator, MatrixFromQuatExpr<Quaternion::iterator> > T;
	return MatrixExpr<T>(T(a.begin(), MatrixFromQuatExpr<Quaternion::iterator>(b.begin())));
}
template<class A>
inline
MatrixExpr<MatrixMultExpr<MatrixExpr<A>, MatrixFromQuatExpr<Quaternion::iterator> > >
operator*(const MatrixExpr<A>& a, const Quaternion& b)
{
	typedef MatrixMultExpr<MatrixExpr<A>, MatrixFromQuatExpr<Quaternion::iterator> > T;
	return MatrixExpr<T>(T(a, MatrixFromQuatExpr<Quaternion::iterator>(b.begin())));
}

// matrix/constant multiply
inline
MatrixExpr<MatrixExprBOp<VecExprConstant, Matrix::iterator, VecBExprMultiply> >
operator*(Real c, const Matrix& a)
{
	typedef MatrixExprBOp<VecExprConstant, Matrix::iterator, VecBExprMultiply> T;
	return MatrixExpr<T>(T(VecExprConstant(c), a.begin(), VecBExprMultiply()));
}
template<class A>
inline
MatrixExpr<MatrixExprBOp<VecExprConstant, MatrixExpr<A>, VecBExprMultiply> >
operator*(Real c, const MatrixExpr<A>& a)
{
	typedef MatrixExprBOp<VecExprConstant, MatrixExpr<A>, VecBExprMultiply> T;
	return MatrixExpr<T>(T(VecExprConstant(c), a, VecBExprMultiply()));
}
inline
MatrixExpr<MatrixExprBOp<Matrix::iterator, VecExprConstant, VecBExprMultiply> >
operator*(const Matrix& a, Real c)
{
	typedef MatrixExprBOp<Matrix::iterator, VecExprConstant, VecBExprMultiply> T;
	return MatrixExpr<T>(T(a.begin(), VecExprConstant(c), VecBExprMultiply()));
}
template<class A>
inline
MatrixExpr<MatrixExprBOp<MatrixExpr<A>, VecExprConstant, VecBExprMultiply> >
operator*(const MatrixExpr<A>& a, Real c)
{
	typedef MatrixExprBOp<MatrixExpr<A>, VecExprConstant, VecBExprMultiply> T;
	return MatrixExpr<T>(T(a, VecExprConstant(c), VecBExprMultiply()));
}


//
// quaternion operators
//

// quaternion unary plus
inline
QuatExpr<QuatExprUOp<Quaternion::iterator, VecUExprPlus> >
operator+(const Quaternion& a)
{
	typedef QuatExprUOp<Quaternion::iterator, VecUExprPlus> T;
	return QuatExpr<T>(T(a.begin(), VecUExprPlus()));
}
template<class A>
inline
QuatExpr<QuatExprUOp<QuatExpr<A>, VecUExprPlus> >
operator+(const QuatExpr<A>& a)
{
	typedef QuatExprUOp<QuatExpr<A>, VecUExprPlus> T;
	return QuatExpr<T>(T(a, VecUExprPlus()));
}

// quaternion unary minus
inline
QuatExpr<QuatExprUOp<Quaternion::iterator, VecUExprMinus> >
operator-(const Quaternion& a)
{
	typedef QuatExprUOp<Quaternion::iterator, VecUExprMinus> T;
	return QuatExpr<T>(T(a.begin(), VecUExprMinus()));
}
template<class A>
inline
QuatExpr<QuatExprUOp<QuatExpr<A>, VecUExprMinus> >
operator-(const QuatExpr<A>& a)
{
	typedef QuatExprUOp<QuatExpr<A>, VecUExprMinus> T;
	return QuatExpr<T>(T(a, VecUExprMinus()));
}

// quaternion product
inline
QuatExpr<QuatMultExpr<Quaternion::iterator, Quaternion::iterator> >
operator*(const Quaternion& a, const Quaternion& b)
{
	typedef QuatMultExpr<Quaternion::iterator, Quaternion::iterator> T;
	return QuatExpr<T>(T(a.begin(), b.begin()));
}
template<class B>
inline
QuatExpr<QuatMultExpr<Quaternion::iterator, QuatExpr<B> > >
operator*(const Quaternion& a, const QuatExpr<B>& b)
{
	typedef QuatMultExpr<Quaternion::iterator, QuatExpr<B> > T;
	return QuatExpr<T>(T(a.begin(), b));
}
template<class A>
inline
QuatExpr<QuatMultExpr<QuatExpr<A>, Quaternion::iterator> >
operator*(const QuatExpr<A>& a, const Quaternion& b)
{
	typedef QuatMultExpr<QuatExpr<A>, Quaternion::iterator> T;
	return QuatExpr<T>(T(a, b.begin()));
}
template<class A, class B>
inline
QuatExpr<QuatMultExpr<QuatExpr<A>, QuatExpr<B> > >
operator*(const QuatExpr<A>& a, const QuatExpr<B>& b)
{
	typedef QuatMultExpr<QuatExpr<A>, QuatExpr<B> > T;
	return QuatExpr<T>(T(a, b));
}

// quaternion product, vector promoted to quaternion
inline
QuatExpr<QuatMultExpr<Quaternion::iterator, QuatFromVecExpr<Vec3::iterator> > >
operator%(const Quaternion& a, const Vec3& b)
{
	typedef QuatMultExpr<Quaternion::iterator, QuatFromVecExpr<Vec3::iterator> > T;
	return QuatExpr<T>(T(a.begin(), QuatFromVecExpr<Vec3::iterator>(b.begin())));
}
template<class B>
inline
QuatExpr<QuatMultExpr<Quaternion::iterator, QuatFromVecExpr<VecExpr<B> > > >
operator%(const Quaternion& a, const VecExpr<B>& b)
{
	typedef QuatMultExpr<Quaternion::iterator, QuatFromVecExpr<VecExpr<B> > > T;
	return QuatExpr<T>(T(a.begin(), QuatFromVecExpr<VecExpr<B> >(b)));
}
template<class A>
inline
QuatExpr<QuatMultExpr<QuatExpr<A>, QuatFromVecExpr<Vec3::iterator> > >
operator%(const QuatExpr<A>& a, const Vec3& b)
{
	typedef QuatMultExpr<QuatExpr<A>, QuatFromVecExpr<Vec3::iterator> > T;
	return QuatExpr<T>(T(a, QuatFromVecExpr<Vec3::iterator>(b.begin())));
}
template<class A, class B>
inline
QuatExpr<QuatMultExpr<QuatExpr<A>, QuatFromVecExpr<VecExpr<B> > > >
operator%(const QuatExpr<A>& a, const VecExpr<B>& b)
{
	typedef QuatMultExpr<QuatExpr<A>, QuatFromVecExpr<VecExpr<B> > > T;
	return QuatExpr<T>(T(a, QuatFromVecExpr<VecExpr<B> >(b)));
}
inline
QuatExpr<QuatMultExpr<QuatFromVecExpr<Vec3::iterator>, Quaternion::iterator> >
operator%(const Vec3& a, const Quaternion& b)
{
	typedef QuatMultExpr<QuatFromVecExpr<Vec3::iterator>, Quaternion::iterator> T;
	return QuatExpr<T>(T(QuatFromVecExpr<Vec3::iterator>(a.begin()), b.begin()));
}
template<class A>
inline
QuatExpr<QuatMultExpr<QuatFromVecExpr<VecExpr<A> >, Quaternion::iterator> >
operator%(const VecExpr<A>& a, const Quaternion& b)
{
	typedef QuatMultExpr<QuatFromVecExpr<VecExpr<A> >, Quaternion::iterator> T;
	return QuatExpr<T>(T(QuatFromVecExpr<VecExpr<A> >(a), b.begin()));
}
template<class B>
inline
QuatExpr<QuatMultExpr<QuatFromVecExpr<Vec3::iterator>, QuatExpr<B> > >
operator%(const Vec3& a, const QuatExpr<B>& b)
{
	typedef QuatMultExpr<QuatFromVecExpr<Vec3::iterator>, QuatExpr<B> > T;
	return QuatExpr<T>(T(QuatFromVecExpr<Vec3::iterator>(a.begin()), b));
}
template<class A, class B>
inline
QuatExpr<QuatMultExpr<QuatFromVecExpr<VecExpr<A> >, QuatExpr<B> > >
operator%(const VecExpr<A>& a, const QuatExpr<B>& b)
{
	typedef QuatMultExpr<QuatFromVecExpr<VecExpr<A> >, QuatExpr<B> > T;
	return QuatExpr<T>(T(QuatFromVecExpr<VecExpr<A> >(a), b));
}

// quaternion/constant product

// constant product
inline
QuatExpr<QuatExprBOp<VecExprConstant, Quaternion::iterator, VecBExprMultiply> >
operator*(Real c, const Quaternion& a)
{
	typedef QuatExprBOp<VecExprConstant, Quaternion::iterator, VecBExprMultiply> T;
	return QuatExpr<T>(T(VecExprConstant(c), a.begin(), VecBExprMultiply()));
}
template<class A>
inline
QuatExpr<QuatExprBOp<VecExprConstant, QuatExpr<A>, VecBExprMultiply> >
operator*(Real c, const QuatExpr<A>& a)
{
	typedef QuatExprBOp<VecExprConstant, QuatExpr<A>, VecBExprMultiply> T;
	return QuatExpr<T>(T(VecExprConstant(c), a, VecBExprMultiply()));
}
inline
QuatExpr<QuatExprBOp<Quaternion::iterator, VecExprConstant, VecBExprMultiply> >
operator*(const Quaternion& a, Real c)
{
	typedef QuatExprBOp<Quaternion::iterator, VecExprConstant, VecBExprMultiply> T;
	return QuatExpr<T>(T(a.begin(), VecExprConstant(c), VecBExprMultiply()));
}
template<class A>
inline
QuatExpr<QuatExprBOp<QuatExpr<A>, VecExprConstant, VecBExprMultiply> >
operator*(const QuatExpr<A>& a, Real c)
{
	typedef QuatExprBOp<QuatExpr<A>, VecExprConstant, VecBExprMultiply> T;
	return QuatExpr<T>(T(a, VecExprConstant(c), VecBExprMultiply()));
}


//
// matrix/vector operators
//

// matrix/vector product
inline
VecExpr<MatrixVecMultExpr<Matrix::iterator, Vec3::iterator> >
operator*(const Matrix& a, const Vec3& b)
{
	typedef MatrixVecMultExpr<Matrix::iterator, Vec3::iterator> T;
	return VecExpr<T>(T(a.begin(), b.begin()));
}
template<class B>
inline
VecExpr<MatrixVecMultExpr<Matrix::iterator, VecExpr<B> > >
operator*(const Matrix& a, const VecExpr<B>& b)
{
	typedef MatrixVecMultExpr<Matrix::iterator, VecExpr<B> > T;
	return VecExpr<T>(T(a.begin(), b));
}
template<class A>
inline
VecExpr<MatrixVecMultExpr<MatrixExpr<A>, Vec3::iterator> >
operator*(const MatrixExpr<A>& a, const Vec3& b)
{
	typedef MatrixVecMultExpr<MatrixExpr<A>, Vec3::iterator> T;
	return VecExpr<T>(T(a, b.begin()));
}
template<class A, class B>
inline
VecExpr<MatrixVecMultExpr<MatrixExpr<A>, VecExpr<B> > >
operator*(const MatrixExpr<A>& a, const VecExpr<B>& b)
{
	typedef MatrixVecMultExpr<MatrixExpr<A>, VecExpr<B> > T;
	return VecExpr<T>(T(a, b));
}


//
// quaternion/vector operators
//

// quaternion/vector product
inline
VecExpr<QuatVecMultExpr<Quaternion::iterator, Vec3::iterator> >
operator*(const Quaternion& a, const Vec3& b)
{
	typedef QuatVecMultExpr<Quaternion::iterator, Vec3::iterator> T;
	return VecExpr<T>(T(a.begin(), b.begin()));
}
template<class B>
inline
VecExpr<QuatVecMultExpr<Quaternion::iterator, VecExpr<B> > >
operator*(const Quaternion& a, const VecExpr<B>& b)
{
	typedef QuatVecMultExpr<Quaternion::iterator, VecExpr<B> > T;
	return VecExpr<T>(T(a.begin(), b));
}
template<class A>
inline
VecExpr<QuatVecMultExpr<QuatExpr<A>, Vec3::iterator> >
operator*(const QuatExpr<A>& a, const Vec3& b)
{
	typedef QuatVecMultExpr<QuatExpr<A>, Vec3::iterator> T;
	return VecExpr<T>(T(a, b.begin()));
}
template<class A, class B>
inline
VecExpr<QuatVecMultExpr<QuatExpr<A>, VecExpr<B> > >
operator*(const QuatExpr<A>& a, const VecExpr<B>& b)
{
	typedef QuatVecMultExpr<QuatExpr<A>, VecExpr<B> > T;
	return VecExpr<T>(T(a, b));
}

#endif // defined(BZF_MTE)

//
// Matrix
//

inline
Matrix::Matrix()
{
	identity();
}

#if defined(BZF_MTE)

template<class A>
inline
Matrix::Matrix(MatrixExpr<A> expr)
{
	operator=(expr);
}

template<class A>
inline
Matrix::Matrix(QuatExpr<A> expr)
{
	operator=(expr);
}

template<class A>
inline
Matrix&					Matrix::operator=(MatrixExpr<A> expr)
{
	Real t[16];
	for (unsigned int i = 0; i < 16; ++i)
		t[i] = expr[i];
	for (unsigned int i = 0; i < 16; ++i)
		m[i] = t[i];
	return *this;
}

inline
Matrix&					Matrix::operator=(const Quaternion& q)
{
	return operator=(MatrixExpr<MatrixFromQuatExpr<Quaternion::iterator> >
							(MatrixFromQuatExpr<Quaternion::iterator>(q.begin())));
}

template<class A>
inline
Matrix&					Matrix::operator=(QuatExpr<A> expr)
{
	return operator=(MatrixExpr<MatrixFromQuatExpr<QuatExpr<A> > >
							(MatrixFromQuatExpr<QuatExpr<A> >(expr)));
}

template<class A>
inline
Matrix&					Matrix::operator*=(MatrixExpr<A> expr)
{
	Matrix t = expr;
	return operator*=(t);
}

#endif // defined(BZF_MTE)

inline
Matrix::Matrix(const Quaternion& q)
{
	operator=(q);
}

inline
Matrix&					Matrix::setScale(Real x, Real y, Real z)
{
	identity();
	m[0]  = x;
	m[5]  = y;
	m[10] = z;
	return *this;
}

inline
Matrix&					Matrix::setTranslate(Real x, Real y, Real z)
{
	identity();
	m[12] = x;
	m[13] = y;
	m[14] = z;
	return *this;
}

inline
Real&					Matrix::operator[](unsigned int index)
{
	assert(index < 16);
	return m[index];
}

inline
Real					Matrix::operator[](unsigned int index) const
{
	assert(index < 16);
	return m[index];
}


//
// Quaternion
//

inline
Quaternion::Quaternion()
{
    v[0] = R_(1.0);
    v[1] = v[2] = v[3] = R_(0.0);
}

inline
Quaternion::Quaternion(const Quaternion& q)
{
    v[0] = q.v[0];
    v[1] = q.v[1];
    v[2] = q.v[2];
    v[3] = q.v[3];
}

inline
Quaternion::Quaternion(Real r, Real i, Real j, Real k)
{
    v[0] = r;
    v[1] = i;
    v[2] = j;
    v[3] = k;
}

inline
Quaternion::Quaternion(const Vec3& v_)
{
    v[0] = R_(0.0);
    v[1] = v_[0];
    v[2] = v_[1];
    v[3] = v_[2];
}

inline
Quaternion::~Quaternion()
{
    // do nothing
}

inline
Quaternion&				Quaternion::operator=(const Quaternion& q)
{
    v[0] = q.v[0];
    v[1] = q.v[1];
    v[2] = q.v[2];
    v[3] = q.v[3];
    return *this;
}

inline
Quaternion&				Quaternion::operator=(const Vec3& v_)
{
    v[0] = 0.0f;
    v[1] = v_[0];
    v[2] = v_[1];
    v[3] = v_[2];
    return *this;
}

inline
Quaternion&				Quaternion::operator*=(Real a)
{
    v[0] *= a;
    v[1] *= a;
    v[2] *= a;
    v[3] *= a;
    return *this;
}

#if defined(BZF_MTE)

template<class A>
inline
Quaternion::Quaternion(QuatExpr<A> expr)
{
	operator=(expr);
}

template<class A>
inline
Quaternion&				Quaternion::operator=(QuatExpr<A> expr)
{
	Real t[3];
	t[0] = expr[0];
	t[1] = expr[1];
	t[2] = expr[2];
	v[3] = expr[3];
	v[0] = t[0];
	v[1] = t[1];
	v[2] = t[2];
	return *this;
}

#endif // defined(BZF_MTE)

inline
Quaternion&				Quaternion::invert()
{
    v[1] = -v[1];
    v[2] = -v[2];
    v[3] = -v[3];
    return *this;
}

inline
Real					Quaternion::length() const
{
    return sqrtr(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
}

inline
Quaternion&				Quaternion::normalize()
{
    return (*this) *= R_(1.0) / length();
}

inline
Quaternion&				Quaternion::negate()
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
    v[3] = -v[3];
    return *this;
}

inline
Quaternion&				Quaternion::set(Real r, Real i, Real j, Real k)
{
	v[0] = r;
	v[1] = i;
	v[2] = j;
	v[3] = k;
	return *this;
}

inline
Real&					Quaternion::operator[](unsigned int i)
{
    assert(i <= 3);
    return v[i];
}

inline
Real					Quaternion::operator[](unsigned int i) const
{
    assert(i <= 3);
    return v[i];
}

#if !defined(BZF_MTE)

//
// non-member template 3D math implementation.  this is significantly
// slower than the member-template implementation due to the overhead
// from temporary objects and redundant loops.
//

inline
Vec3
operator+(const Vec3& a)
{
	return a;
}

inline
Vec3
operator-(const Vec3& a)
{
	return Vec3(-a[0], -a[1], -a[2]);
}

inline
Vec3
operator+(const Vec3& a, const Vec3& b)
{
	return Vec3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

inline
Vec3
operator-(const Vec3& a, const Vec3& b)
{
	return Vec3(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

inline
Vec3
operator*(Real a, const Vec3& b)
{
	return Vec3(a * b[0], a * b[1], a * b[2]);
}

inline
Vec3
operator*(const Vec3& b, Real a)
{
	return Vec3(a * b[0], a * b[1], a * b[2]);
}

inline
Real
operator*(const Vec3& a, const Vec3& b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline
Vec3
operator%(const Vec3& a, const Vec3& b)
{
	return Vec3(a[1] * b[2] - a[2] * b[1],
				a[2] * b[0] - a[0] * b[2],
				a[0] * b[1] - a[1] * b[0]);
}

typedef const float* FloatMatrix;
typedef const double* DoubleMatrix;
typedef const Real* RealMatrix;

inline
Matrix::Matrix(const float* a)
{
	m[0]  = static_cast<float>(a[0]);
	m[1]  = static_cast<float>(a[1]);
	m[2]  = static_cast<float>(a[2]);
	m[3]  = static_cast<float>(a[3]);
	m[4]  = static_cast<float>(a[4]);
	m[5]  = static_cast<float>(a[5]);
	m[6]  = static_cast<float>(a[6]);
	m[7]  = static_cast<float>(a[7]);
	m[8]  = static_cast<float>(a[8]);
	m[9]  = static_cast<float>(a[9]);
	m[10] = static_cast<float>(a[10]);
	m[11] = static_cast<float>(a[11]);
	m[12] = static_cast<float>(a[12]);
	m[13] = static_cast<float>(a[13]);
	m[14] = static_cast<float>(a[14]);
	m[15] = static_cast<float>(a[15]);
}

inline
Matrix::Matrix(const double* a)
{
	m[0]  = static_cast<double>(a[0]);
	m[1]  = static_cast<double>(a[1]);
	m[2]  = static_cast<double>(a[2]);
	m[3]  = static_cast<double>(a[3]);
	m[4]  = static_cast<double>(a[4]);
	m[5]  = static_cast<double>(a[5]);
	m[6]  = static_cast<double>(a[6]);
	m[7]  = static_cast<double>(a[7]);
	m[8]  = static_cast<double>(a[8]);
	m[9]  = static_cast<double>(a[9]);
	m[10] = static_cast<double>(a[10]);
	m[11] = static_cast<double>(a[11]);
	m[12] = static_cast<double>(a[12]);
	m[13] = static_cast<double>(a[13]);
	m[14] = static_cast<double>(a[14]);
	m[15] = static_cast<double>(a[15]);
}

inline
Matrix&					Matrix::operator=(const Quaternion& q)
{
	m[0]  = R_(1.0) - R_(2.0) * (q[2] * q[2] + q[3] * q[3]);
	m[1]  = R_(2.0) * (q[1] * q[2] + q[3] * q[0]);
	m[2]  = R_(2.0) * (q[3] * q[1] - q[2] * q[0]);
	m[3]  = R_(0.0);
	m[4]  = R_(2.0) * (q[1] * q[2] - q[3] * q[0]);
	m[5]  = R_(1.0) - R_(2.0) * (q[3] * q[3] + q[1] * q[1]);
	m[6]  = R_(2.0) * (q[2] * q[3] + q[1] * q[0]);
	m[7]  = R_(0.0);
	m[8]  = R_(2.0) * (q[3] * q[1] + q[2] * q[0]);
	m[9]  = R_(2.0) * (q[2] * q[3] - q[1] * q[0]);
	m[10] = R_(1.0) - R_(2.0) * (q[1] * q[1] + q[2] * q[2]);
	m[11] = R_(0.0);
	m[12] = R_(0.0);
	m[13] = R_(0.0);
	m[14] = R_(0.0);
	m[15] = R_(1.0);
	return *this;
}

inline
Matrix
operator+(const Matrix& a)
{
	return a;
}

inline
Matrix
operator-(const Matrix& a)
{
	Matrix t = a;
	t[0]  = -t[0];
	t[1]  = -t[1];
	t[2]  = -t[2];
	t[3]  = -t[3];
	t[4]  = -t[4];
	t[5]  = -t[5];
	t[6]  = -t[6];
	t[7]  = -t[7];
	t[8]  = -t[8];
	t[9]  = -t[9];
	t[10] = -t[10];
	t[11] = -t[11];
	t[12] = -t[12];
	t[13] = -t[13];
	t[14] = -t[14];
	t[15] = -t[15];
	return t;
}

inline
Matrix
operator+(const Matrix& a, const Matrix& b)
{
	Matrix t = a;
	t[0]  += b[0];
	t[1]  += b[1];
	t[2]  += b[2];
	t[3]  += b[3];
	t[4]  += b[4];
	t[5]  += b[5];
	t[6]  += b[6];
	t[7]  += b[7];
	t[8]  += b[8];
	t[9]  += b[9];
	t[10] += b[10];
	t[11] += b[11];
	t[12] += b[12];
	t[13] += b[13];
	t[14] += b[14];
	t[15] += b[15];
	return t;
}

inline
Matrix
operator-(const Matrix& a, const Matrix& b)
{
	Matrix t = a;
	t[0]  -= b[0];
	t[1]  -= b[1];
	t[2]  -= b[2];
	t[3]  -= b[3];
	t[4]  -= b[4];
	t[5]  -= b[5];
	t[6]  -= b[6];
	t[7]  -= b[7];
	t[8]  -= b[8];
	t[9]  -= b[9];
	t[10] -= b[10];
	t[11] -= b[11];
	t[12] -= b[12];
	t[13] -= b[13];
	t[14] -= b[14];
	t[15] -= b[15];
	return t;
}

inline
Matrix
operator*(const Matrix& a, const Matrix& b)
{
	Matrix t = a;
	return t *= b;
}

inline
Matrix
operator*(Real b, const Matrix& a)
{
	Matrix t = a;
	return t *= b;
}

inline
Matrix
operator*(const Matrix& a, Real b)
{
	Matrix t = a;
	return t *= b;
}

inline
Quaternion
operator+(const Quaternion& a)
{
	return a;
}

inline
Quaternion
operator-(const Quaternion& a)
{
	return Quaternion(-a[0], -a[1], -a[2], -a[3]);
}

inline
Quaternion
operator*(const Quaternion& a, const Quaternion& b)
{
	Quaternion t = a;
	return t *= b;
}

inline
Quaternion
operator%(const Quaternion& a, const Vec3& b)
{
	Quaternion t = a;
	return t *= Quaternion(b);
}

inline
Quaternion
operator%(const Vec3& a, const Quaternion& b)
{
	Quaternion t = a;
	return t *= b;
}

inline
Quaternion
operator*(Real a, const Quaternion& b)
{
	return Quaternion(a * b[0], a * b[1], a * b[2], a * b[3]);
}

inline
Quaternion
operator*(const Quaternion& b, Real a)
{
	return Quaternion(a * b[0], a * b[1], a * b[2], a * b[3]);
}

inline
Vec3
operator*(const Matrix& a, const Vec3& b)
{
	return Vec3(a[0] * b[0] + a[4] * b[1] + a[8]  * b[2],
				a[1] * b[0] + a[5] * b[1] + a[9]  * b[2],
				a[2] * b[0] + a[6] * b[1] + a[10] * b[2]);
}

inline
Vec3
operator*(const Quaternion& a, const Vec3& b)
{
	Real q[4];
	q[0] = b[0] * a[1] + b[1] * a[2] + b[2] * a[3];
	q[1] = b[0] * a[0] + b[2] * a[2] - b[1] * a[3];
	q[2] = b[1] * a[0] + b[0] * a[3] - b[2] * a[1];
	q[3] = b[2] * a[0] + b[1] * a[1] - b[0] * a[2];
	return Vec3(a[0] * q[1] + q[0] * a[1] + a[2] * q[3] - a[3] * q[2],
				a[0] * q[2] + q[0] * a[2] + a[3] * q[1] - a[1] * q[3],
				a[0] * q[3] + q[0] * a[3] + a[1] * q[2] - a[2] * q[1]);
}

#endif // !defined(BZF_MTE)


//
// Plane
//

inline
Plane::Plane() : normal(R_(0.0), R_(0.0), R_(1.0)), d(R_(0.0))
{
	// do nothing
}

inline
Plane::Plane(const Vec3& normal_, Real d_) : normal(normal_), d(d_)
{
	// do nothing
}

inline
Plane::Plane(const Vec3& normal_, const Vec3& pointOnPlane) :
								normal(normal_),
								d(-(normal_ * pointOnPlane))
{
	// do nothing
}

inline
Plane::Plane(const Vec3& v0, const Vec3& v1, const Vec3& v2) :
								normal((v1 - v0) % (v2 - v1))
{
	normal.normalize();
	d = -(normal * v0);
}

inline
Plane&					Plane::negate()
{
	normal.negate();
	d = -d;
	return *this;
}

inline
Plane&					Plane::set(const Vec3& normal_, Real d_)
{
	normal = normal_;
	d      = d_;
	return *this;
}

inline
Plane&					Plane::set(const Vec3& normal_,
								const Vec3& pointOnPlane)
{
	normal = normal_;
	d      = -(normal * pointOnPlane);
	return *this;
}

inline
Plane&					Plane::set(const Vec3& v0,
								const Vec3& v1, const Vec3& v2)
{
	normal = (v1 - v0) % (v2 - v1);
	normal.normalize();
	d      = -(normal * v0);
	return *this;
}

inline
const Vec3&				Plane::getNormal() const
{
	return normal;
}

inline
Real					Plane::getOffset() const
{
	return d;
}

inline
Real					Plane::distance(const Vec3& p) const
{
	return d + normal * p;
}

inline
void					Plane::projectPoint(Vec3& pOut, const Vec3& p) const
{
	pOut = p - distance(p) * normal;
}

inline
void					Plane::projectVector(Vec3& vOut, const Vec3& v) const
{
	vOut = v - (normal * v) * normal;
}


//
// Ray
//

inline
Ray::Ray()
{
	// do nothing
}

inline
Ray::Ray(const Vec3& o_, const Vec3& d_) : o(o_), d(d_)
{
	// do nothing
}

inline
Ray::Ray(const float* o_, const float* d_) : o(o_), d(d_)
{
	// do nothing
}

inline
Ray::Ray(const double* o_, const double* d_) : o(o_), d(d_)
{
	// do nothing
}

inline
const Vec3&				Ray::getOrigin() const
{
	return o;
}

inline
const Vec3&				Ray::getDirection() const
{
	return d;
}

inline
void					Ray::getPoint(Real t, float p[3]) const
{
	p[0] = static_cast<float>(o[0] + t * d[0]);
	p[1] = static_cast<float>(o[1] + t * d[1]);
	p[2] = static_cast<float>(o[2] + t * d[2]);
}

inline
void					Ray::getPoint(Real t, double p[3]) const
{
	p[0] = static_cast<double>(o[0] + t * d[0]);
	p[1] = static_cast<double>(o[1] + t * d[1]);
	p[2] = static_cast<double>(o[2] + t * d[2]);
}

#endif
