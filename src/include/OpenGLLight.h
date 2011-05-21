/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OpenGLLight:
 *  Encapsulates an OpenGL (point or directional) light source.
 */

#ifndef BZF_OPENGL_LIGHT_H
#define BZF_OPENGL_LIGHT_H

// common goes first
#include "common.h"

// system headers
#include <vector>

// common headers
#include "vectors.h"
#include "ViewFrustum.h"


class OpenGLLight {
  public:
    OpenGLLight();
    OpenGLLight(const OpenGLLight&);
    OpenGLLight& operator=(const OpenGLLight&);
    ~OpenGLLight();

    const fvec4&  getPosition() const;
    const fvec4&  getColor() const;
    const fvec3&  getAttenuation() const;
    float   getMaxDist() const;

    void    setDirection(const fvec3& xyz);
    void    setPosition(const fvec3& xyz);
    void    setColor(float r, float g, float b);
    void    setColor(const fvec4& rgba);
    void    setAttenuation(const fvec3& clq);
    void    setAttenuation(int index, float value);

    void    calculateImportance(const ViewFrustum& frustum);
    float   getImportance() const;

    void    setOnlyReal(bool value);
    bool    getOnlyReal() const;
    void    setOnlyGround(bool value);
    bool    getOnlyGround() const;

    void    execute(int index, bool useList) const;

    static int    getMaxLights();
    static void   enableLight(int index, bool on); // const

  protected:
    void    makeLists();
    void    freeLists();
    void    genLight(unsigned int light) const;

  private:
    static void   freeContext(void*);
    static void   initContext(void*);

  private:
    fvec4   pos;
    fvec4   color;
    fvec3   atten;
    float   maxDist;
    float   importance;
    bool    onlyReal;
    bool    onlyGround;
    unsigned int* lists;
    static int    maxLights;
};

//
// OpenGLLight
//

inline const fvec4& OpenGLLight::getPosition() const {
  return pos;
}

inline const fvec4& OpenGLLight::getColor() const {
  return color;
}

inline const fvec3& OpenGLLight::getAttenuation() const {
  return atten;
}

inline float    OpenGLLight::getMaxDist() const {
  return maxDist;
}

inline float     OpenGLLight::getImportance() const {
  return importance;
}

inline bool    OpenGLLight::getOnlyReal() const {
  return onlyReal;
}

inline bool    OpenGLLight::getOnlyGround() const {
  return onlyGround;
}


#endif // BZF_OPENGL_LIGHT_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
