/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _TEXTURE_MATRIX_H_
#define _TEXTURE_MATRIX_H_


#include "TimeKeeper.h"

#include <vector>
#include <iostream>


class TextureMatrix {
  public:
    TextureMatrix();
    ~TextureMatrix();
    const float* getMatrix();
    void setShiftParams (float uFreq, float vFreq);
    void setRotateParams (float freq, float uCenter, float vCenter);
    void setScaleParams (float uFreq, float vFreq, 
                         float uCenter, float vCenter,
                         float uSize, float vSize);
    void update(float time);

    void* pack(void*);
    void* unpack(void*);
    int packSize();
    
    void print(std::ostream& out, int level);
  
  private:
    float matrix[16];
    float uPos, vPos; // time invariant
    float uSize, vSize; // time invariant
    float uShiftFreq, vShiftFreq;
    float rotateFreq, uRotateCenter, vRotateCenter;
    float uScaleFreq, vScaleFreq;
    float uScale, vScale;
    float uScaleCenter, vScaleCenter;
};


class TextureMatrixManager {
  public:
    TextureMatrixManager();
    ~TextureMatrixManager();
    void update();
    void clear();
    int addMatrix(TextureMatrix* matrix);
    TextureMatrix* getMatrix(int id);

    void* pack(void*);
    void* unpack(void*);
    int packSize();
    
    void print(std::ostream& out, int level);

  private:
    std::vector<TextureMatrix*> matrices;
};

extern TextureMatrixManager TEXMATRIXMGR;

#endif //_TEXTURE_MATRIX_H_
