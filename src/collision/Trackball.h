#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "TimeKeeper.h"

class BzfEvent;

class Trackball {
public:
	Trackball();
	~Trackball();

	void				reset();
	void				resize(int w, int h);
	void				getMatrix(float*) const;
	void				getProjection(float*, float n, float f) const;
	bool				isSpinning() const;
	bool				onEvent(const BzfEvent&, bool& redraw);
	void				spin();

#if !defined(_MSC_VER) // crs -- work around VC++ 6.0 bug
private:
#endif
	class Quaternion {
	public:
		Quaternion();
		Quaternion(const Quaternion&);
		Quaternion(float, float, float, float);
		Quaternion(float a[4]);
		Quaternion(float axis[3], float angle);
		Quaternion&		operator = (const Quaternion&);

		float&			operator [] (int);
		float			operator [] (int) const;
		void			normalize();
		float			length() const;
		void			matrix(float m[4][4]) const;

		Quaternion&		operator *= (const Quaternion&);

	private:
		float			a[4];
	};

private:
	float				projectToSphere(float r, float x, float y) const;
	void				trackball(float p1x, float p1y,
							float p2x, float p2y, float delta[4]) const;
	void				doFlip(Quaternion&, float);

private:
	float				history[20][2];
	TimeKeeper			hTime[20];
	unsigned int		pHistory;
	float				x0, y0, x, y;
	long				wx, wy, wdx, wdy;
	float				aspect;
	short				normcount;
	bool				spinning, turning, panning, trucking, zooming;
	Quaternion			base, rot;
	float				delta[4];
	float				xlate[3];
	float				xlate0[3];
	float				fov, fov0;
	TimeKeeper			lastMove;
};

#endif
// ex: shiftwidth=4 tabstop=4
