/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 * Written By Dave Rodgers (aka: trepan)
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// there is no 'implementation header'

// common headers
/////////////////////////
#define UNSAFE_GL_CONTEXT
/////////// for /////////
#include "bzfgl.h"
/////////////////////////
#include "common/bzfio.h"
#include "ogl/OpenGLGState.h"


//============================================================================//
//============================================================================//
//
//  for hooking debuggers
//

static void contextFreeError(const char* message) {
  printf("contextFreeError(): %s\n", message);
}


static void contextInitError(const char* message) {
  printf("contextInitError(): %s\n", message);
}


//============================================================================//
//============================================================================//
//
//  Display Lists
//

void bzNewList(GLuint list, GLenum mode) {
  glNewList(list, mode);
  return;
}


GLuint bzGenLists(GLsizei count) {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzGenLists() is having issues");
  }
  GLuint base = glGenLists(count);
  return base;
}


void bzDeleteLists(GLuint base, GLsizei count) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteLists() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteLists(base, count);
  }
  else {
    debugf(4, "bzDeleteLists(), no context\n");
  }
  return;
}


//============================================================================//
//============================================================================//
//
//  Textures
//

void bzGenTextures(GLsizei count, GLuint* textures) {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzGenTextures() is having issues");
  }
  glGenTextures(count, textures);
  return;
}


void bzDeleteTextures(GLsizei count, const GLuint* textures) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteTextures() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteTextures(count, textures);
  }
  else {
    debugf(4, "bzDeleteTextures(), no context\n");
  }
  return;
}


//============================================================================//
//============================================================================//
//
//  Shaders
//

GLuint bzCreateShader(GLenum type) {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzCreateShader() is having issues");
  }
  return glCreateShader(type);
}


void bzDeleteShader(GLuint shader) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteShader() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteShader(shader);
  }
  else {
    debugf(4, "bzDeleteShader(), no context\n");
  }
}


GLuint bzCreateProgram() {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzCreateProgram() is having issues");
  }
  return glCreateProgram();
}


void bzDeleteProgram(GLuint program) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteProgram() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteProgram(program);
  }
  else {
    debugf(4, "bzDeleteProgram(), no context\n");
  }
}


//============================================================================//
//============================================================================//
//
//  Occlusion Queries
//

void bzGenQueries(GLsizei n, GLuint* queries) {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzGenQueries() is having issues");
  }
  glGenQueries(n, queries);
}


void bzDeleteQueries(GLsizei n, const GLuint* queries) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteProgram() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteQueries(n, queries);
  }
  else {
    debugf(4, "bzDeleteQueries(), no context\n");
  }
}


//============================================================================//
//============================================================================//
//
//  Frame Buffers
//

void bzGenFramebuffersEXT(GLsizei n, GLuint* fbos) {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzGenFramebuffersEXT() is having issues");
  }
  glGenFramebuffersEXT(n, fbos);
}


void bzDeleteFramebuffersEXT(GLsizei n, const GLuint* fbos) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteFramebuffersEXT() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteFramebuffersEXT(n, fbos);
  }
  else {
    debugf(4, "bzDeleteFramebuffersEXT(), no context\n");
  }
}


//============================================================================//
//============================================================================//
//
//  Render Buffers
//

void bzGenRenderbuffersEXT(GLsizei n, GLuint* rbos) {
  if (OpenGLGState::isExecutingFreeFuncs()) {
    contextFreeError("bzGenRenderbuffersEXT() is having issues");
  }
  glGenRenderbuffersEXT(n, rbos);
}


void bzDeleteRenderbuffersEXT(GLsizei n, const GLuint* rbos) {
  if (OpenGLGState::isExecutingInitFuncs()) {
    contextInitError("bzDeleteRenderbuffersEXT() is having issues");
  }
  if (OpenGLGState::haveGLContext()) {
    glDeleteRenderbuffersEXT(n, rbos);
  }
  else {
    debugf(4, "bzDeleteRenderbuffersEXT(), no context\n");
  }
}


//============================================================================//
//============================================================================//
//
//  Matrices
//
// Test for matrix underflows (overflows are not yet tested)
//

#ifdef DEBUG_GL_MATRIX_STACKS

static GLenum matrixMode = GL_MODELVIEW;
static int matrixDepth[3] = {0, 0, 0};
static const int maxMatrixDepth[3] = {32, 2, 2}; // guaranteed


static inline int getMatrixSlot(GLenum mode) {
  if (mode == GL_MODELVIEW) {
    return 0;
  }
  else if (mode == GL_PROJECTION) {
    return 1;
  }
  else if (mode == GL_TEXTURE) {
    return 2;
  }
  else {
    return -1;
  }
}


void bzPushMatrix() {
  int slot = getMatrixSlot(matrixMode);
  if (slot < 0) {
    printf("bzPushMatrix(): bad matrix mode: %i\n", matrixMode);
    return;
  }
  matrixDepth[slot]++;
  if (matrixDepth[slot] > maxMatrixDepth[slot]) {
    printf("bzPushMatrix(): overflow (mode %i, depth %i)\n",
           matrixMode, matrixDepth[slot]);
    return;
  }
  glPushMatrix();
}


void bzPopMatrix() {
  int slot = getMatrixSlot(matrixMode);
  if (slot < 0) {
    printf("bzPopMatrix(): bad matrix mode: %i\n", matrixMode);
    return;
  }
  matrixDepth[slot]--;
  if (matrixDepth[slot] < 0) {
    printf("bzPopMatrix(): underflow (mode %i, depth %i)\n",
           matrixMode, matrixDepth[slot]);
    return;
  }
  glPopMatrix();
}


void bzMatrixMode(GLenum mode) {
  int slot = getMatrixSlot(matrixMode);
  if (slot < 0) {
    printf("bzMatrixMode(): bad matrix mode: %i\n", mode);
    return;
  }
  matrixMode = mode;
  glMatrixMode(mode);
  debugf(3, "MatrixMode: %i %i %i\n", matrixDepth[0], matrixDepth[1], matrixDepth[2]);
}


#endif // DEBUG_GL_MATRIX_STACKS


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
