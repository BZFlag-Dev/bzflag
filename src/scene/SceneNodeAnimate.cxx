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

#include "SceneNodeAnimate.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include <math.h>

//
// SceneNodeAnimate
//

static const char* typeMap[] = { "cycle", "swing" };

SceneNodeAnimate::SceneNodeAnimate() :
								type("type", Cycle, typeMap, countof(typeMap)),
								src("src", "time"),
								dst("dst", "t"),
								start("start", 0.0f),
								end("end", 1.0f),
								bias("bias", 0.0f),
								scale("scale", 1.0f),
								step("step", 0.0f),
								cycles("cycles", 0.0f)
{
	// do nothing
}

SceneNodeAnimate::~SceneNodeAnimate()
{
	// do nothing
}

float					SceneNodeAnimate::get(const SceneVisitorParams& params)
{
	// get source value.  bail with start if not found.
	float t;
	if (params.isFloat(src.get()))
		t = params.getFloat(src.get());
	else if (params.isInt(src.get()))
		t = static_cast<float>(params.getInt(src.get()));
	else
		return start.get();

	// apply bias and scale
	t = scale.get() * (bias.get() + t);

	// normalize to start/end range
	float dt = end.get() - start.get();
	if (dt <= 0.0f) {
		// empty range always maps to start
		t = start.get();
	}
	else {
		float t0 = start.get();

		// normalize (into number of cycles)
		t = (t - t0) / dt;

		// swing requires twice through start to end per cycle (once
		// start to end and once end to start)
		if (type.get() == Swing)
			t *= 0.5f;

		// apply num cycles and clamp
		const float tMax = cycles.get();
		if (tMax <= 0.0f) {
			// get fraction of cycle
			t = t - floor(t);

			// [-1,0) maps to [0,1)
			if (t < 0.0f)
				t = 1.0f + t;
		}
		else {
			// clamp
			if (t < 0.0f) {
				// before the beginning
				t = 0.0f;
			}
			else if (t >= tMax) {
				// at or past the end
				t = tMax;

				// get fraction of cycle.  if type is Cycle and we've got
				// an integer number of cycles then stop on the end of the
				// cycle instead of the start.
				float tf = floorf(t);
				if (tf == t && type.get() == Cycle)
					t = 1.0f;
				else
					t = t - tf;
			}
			else {
				// get fraction of cycle
				t = t - floor(t);
			}
		}

		// denormalize
		switch (type.get()) {
			case Cycle:
				t = t * dt;
				break;

			case Swing:
				if (t < 0.5f)
					t = 2.0f * t * dt;
				else
					t = 2.0f * (1.0f - t) * dt;
				break;
		}

		// force to step granularity
		float st = step.get();
		if (st > 0.0f)
			t = st * floorf(t / st);

		// shift to start time
		t += t0;
	}

	return t;
}

bool					SceneNodeAnimate::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
