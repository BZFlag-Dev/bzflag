/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "OpenGLMaterial.h"
#include "RenderNode.h"
#include <string.h>

static GLenum		MY_GL_REPLACE = GL_MODULATE;

//
// OpenGLGStateState
//

class OpenGLGStateState {
  public:
			OpenGLGStateState();
			~OpenGLGStateState();

    void		reset();
    void		enableTexture(boolean);
    void		enableTextureReplace(boolean);
    void		enableMaterial(boolean);
    void		setTexture(const OpenGLTexture&);
    void		setMaterial(const OpenGLMaterial&);
    void		setBlending(GLenum sFactor, GLenum dFactor);
    void		setStipple(float alpha);
    void		setSmoothing(boolean smooth);
    void		setCulling(GLenum culling);
    void		setShading(GLenum);
    void		setAlphaFunc(GLenum func, GLclampf ref);

    boolean		isBlended() const
				{ return unsorted.hasBlending; }
    boolean		isTextured() const
				{ return sorted.texture.isValid(); }
    boolean		isTextureReplace() const
				{ return sorted.hasTextureReplace; }
    boolean		isLighted() const
				{ return sorted.material.isValid(); }

    void		resetOpenGLState() const;
    void		setOpenGLState(const OpenGLGStateState* prev) const;

  public:
    class Sorted {
      public:
			Sorted();
			~Sorted();

	void		reset();
	boolean		operator==(const Sorted&) const;
	boolean		operator<(const Sorted&) const;

      public:
	boolean		hasTexture;
	boolean		hasTextureReplace;
	boolean		hasMaterial;
	OpenGLTexture	texture;
	OpenGLMaterial	material;
    };

    class Unsorted {
      public:
			Unsorted();
			~Unsorted();

	void		reset();

      public:
	boolean		hasBlending;
	boolean		hasStipple;
	boolean		hasSmoothing;
	boolean		hasCulling;
	boolean		hasShading;
	boolean		hasAlphaFunc;
	GLenum		blendSFactor;
	GLenum		blendDFactor;
	int		stippleIndex;
	GLenum		culling;
	GLenum		alphaFunc;
	GLclampf	alphaRef;
    };

  public:
    Sorted		sorted;
    Unsorted		unsorted;
};

//
// OpenGLGStateRep
//

class OpenGLGStateRep {
  public:
			OpenGLGStateRep();
			OpenGLGStateRep(const OpenGLGStateState&);
			~OpenGLGStateRep();

    boolean		isBlended() { return state.isBlended(); }
    boolean		isTextured() { return state.isTextured(); }
    boolean		isTextureReplace() { return state.isTextureReplace(); }
    boolean		isLighted() { return state.isLighted(); }
    void		setState();
    static void		resetState();

    void		ref();
    void		unref();

    const OpenGLGStateState& getState() const { return state; }
    void		addRenderNode(RenderNode* node,
				const OpenGLGState* gstate);

  private:
    friend class SortedGState;

    int			refCount;
    OpenGLGStateState	state;

    // modified by SortedGState
    SortedGState*	bucket;
    OpenGLGStateRep*	prev;
    OpenGLGStateRep*	next;

    static OpenGLGStateState*	lastState;
};

//
// OpenGLGStateState
//

OpenGLGStateState::Sorted::Sorted() :
				hasTexture(False),
				hasTextureReplace(False),
				hasMaterial(False)
{
  // do nothing
}

OpenGLGStateState::Sorted::~Sorted()
{
  // do nothing
}

void			OpenGLGStateState::Sorted::reset()
{
  hasTexture = False;
  hasTextureReplace = False;
  hasMaterial = False;
  texture = OpenGLTexture();
  material = OpenGLMaterial();
}

boolean			OpenGLGStateState::Sorted::operator==(
				const Sorted& s) const
{
  if (hasTexture != s.hasTexture || texture != s.texture)
    return False;
  if (hasTextureReplace != s.hasTextureReplace)
    return False;
  if (hasMaterial != s.hasMaterial || material != s.material)
    return False;
  return True;
}

boolean			OpenGLGStateState::Sorted::operator<(
				const Sorted& s) const
{
  // do arbitrary sorting:
  // this < s if this has no texture and s does or texture < s.texture
  if (hasTexture != s.hasTexture) return s.hasTexture;
  if (hasTexture) {
    if (hasTextureReplace != s.hasTextureReplace) return s.hasTextureReplace;
    return texture < s.texture;
  }

  // this < s if this has no material and s does or material < s.material
  if (hasMaterial != s.hasMaterial) return s.hasMaterial;
  if (hasMaterial) return material < s.material;

  // states are the same
  return False;
}

OpenGLGStateState::Unsorted::Unsorted() :
				hasBlending(False),
				hasStipple(False),
				hasSmoothing(False),
				hasCulling(True),
				hasShading(False),
				hasAlphaFunc(False),
				blendSFactor(GL_ONE),
				blendDFactor(GL_ZERO),
				stippleIndex(0),
				culling(GL_BACK),
				alphaFunc(GL_ALWAYS),
				alphaRef(0.0f)
{
  // do nothing
}

OpenGLGStateState::Unsorted::~Unsorted()
{
  // do nothing
}

void			OpenGLGStateState::Unsorted::reset()
{
  hasBlending = False;
  hasStipple = False;
  hasSmoothing = False;
  hasCulling = True;
  hasShading = False;
  culling = GL_BACK;
}

OpenGLGStateState::OpenGLGStateState()
{
  // do nothing
}

OpenGLGStateState::~OpenGLGStateState()
{
  // do nothing
}

void			OpenGLGStateState::reset()
{
  sorted.reset();
  unsorted.reset();
}

void			OpenGLGStateState::enableTexture(boolean on)
{
  if (on) sorted.hasTexture = sorted.texture.isValid();
  else sorted.hasTexture = False;
}

void			OpenGLGStateState::enableTextureReplace(boolean)
{
  // FIXME -- disabled for now
  //   most gstates haven't been adjusted to use/not-use texture
  //   replacement, so if this is turned on then those must be
  //   fixed appropriately.
  sorted.hasTextureReplace = False;
}

void			OpenGLGStateState::enableMaterial(boolean on)
{
  if (on) sorted.hasMaterial = sorted.material.isValid();
  else sorted.hasMaterial = False;
}

void			OpenGLGStateState::setTexture(
					const OpenGLTexture& _texture)
{
  sorted.hasTexture = _texture.isValid();
  sorted.texture = _texture;
}

void			OpenGLGStateState::setMaterial(
					const OpenGLMaterial& _material)
{
  sorted.hasMaterial = _material.isValid();
  sorted.material = _material;
}

void			OpenGLGStateState::setBlending(
					GLenum sFactor, GLenum dFactor)
{
  unsorted.hasBlending = (sFactor != GL_ONE || dFactor != GL_ZERO);
  unsorted.blendSFactor = sFactor;
  unsorted.blendDFactor = dFactor;
}

void			OpenGLGStateState::setStipple(float alpha)
{
  unsorted.stippleIndex = OpenGLGState::getStippleIndex(alpha);
  unsorted.hasStipple =
	(unsorted.stippleIndex < OpenGLGState::getOpaqueStippleIndex());
}

void			OpenGLGStateState::setSmoothing(boolean smooth)
{
  unsorted.hasSmoothing = smooth;
}

void			OpenGLGStateState::setCulling(GLenum _culling)
{
  unsorted.hasCulling = (_culling != GL_NONE);
  unsorted.culling = _culling;
}

void			OpenGLGStateState::setShading(GLenum shading)
{
  unsorted.hasShading = (shading != GL_FLAT);
}

void			OpenGLGStateState::setAlphaFunc(
				GLenum func, GLclampf ref)
{
  unsorted.hasAlphaFunc = (func != GL_ALWAYS);
  unsorted.alphaFunc = func;
  unsorted.alphaRef = ref;
}

void			OpenGLGStateState::resetOpenGLState() const
{
  if (sorted.hasTexture) {
    glDisable(GL_TEXTURE_2D);
  }
  if (sorted.hasTextureReplace) {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }
  if (sorted.hasMaterial) {
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);	
  }
  if (unsorted.hasBlending) {
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  if (unsorted.hasSmoothing) {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
  }
  if (unsorted.hasStipple) {
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_POLYGON_STIPPLE);
  }
  if (!unsorted.hasCulling || unsorted.culling != GL_BACK) {
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
  }
  if (unsorted.hasShading) {
    glShadeModel(GL_FLAT);
  }
  if (unsorted.hasAlphaFunc) {
    glDisable(GL_ALPHA_TEST);
  }
}

void			OpenGLGStateState::setOpenGLState(
				const OpenGLGStateState* oldState) const
{
  if (oldState == this) return;
  if (oldState) {
    // texture mapping
    if (sorted.hasTexture) {
      if (oldState->sorted.hasTexture) {
	if (sorted.texture != oldState->sorted.texture)
	  sorted.texture.execute();
      }
      else {
	sorted.texture.execute();
	glEnable(GL_TEXTURE_2D);
      }
      if (!oldState->sorted.hasTextureReplace && sorted.hasTextureReplace)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, MY_GL_REPLACE);
    }
    else {
      if (oldState->sorted.hasTexture) {
	glDisable(GL_TEXTURE_2D);
	if (oldState->sorted.hasTextureReplace)
	  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      }
    }

    // lighting and material
    if (sorted.hasMaterial) {
      if (oldState->sorted.hasMaterial) {
	if (sorted.material != oldState->sorted.material)
	  sorted.material.execute();
      }
      else {
	sorted.material.execute();
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);	
      }
    }
    else {
      if (oldState->sorted.hasMaterial) {
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);	
      }
    }

    // blending and blend function
    if (unsorted.hasBlending) {
      if (oldState->unsorted.hasBlending) {
	if (unsorted.blendSFactor != oldState->unsorted.blendSFactor ||
	    unsorted.blendDFactor != oldState->unsorted.blendDFactor)
	  glBlendFunc(unsorted.blendSFactor, unsorted.blendDFactor);
      }
      else {
	glBlendFunc(unsorted.blendSFactor, unsorted.blendDFactor);
	glEnable(GL_BLEND);
      }
    }
    else {
      if (oldState->unsorted.hasBlending)
	glDisable(GL_BLEND);
    }

    // antialiasing
    if (unsorted.hasSmoothing) {
      if (!oldState->unsorted.hasSmoothing) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
      }
    }
    else {
      if (oldState->unsorted.hasSmoothing) {
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
      }
    }

    // stippling
    if (unsorted.hasStipple) {
      if (oldState->unsorted.hasStipple) {
	if (unsorted.stippleIndex != oldState->unsorted.stippleIndex)
	  OpenGLGState::setStippleIndex(unsorted.stippleIndex);
      }
      else {
	OpenGLGState::setStippleIndex(unsorted.stippleIndex);
	glEnable(GL_LINE_STIPPLE);
	glEnable(GL_POLYGON_STIPPLE);
      }
    }
    else {
      if (oldState->unsorted.hasStipple) {
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_POLYGON_STIPPLE);
      }
    }

    // culling
    if (unsorted.hasCulling) {
      if (oldState->unsorted.hasCulling) {
	if (unsorted.culling != oldState->unsorted.culling)
	  glCullFace(unsorted.culling);
      }
      else {
	glCullFace(unsorted.culling);
	glEnable(GL_CULL_FACE);
      }
    }
    else {
      if (oldState->unsorted.hasCulling)
	glDisable(GL_CULL_FACE);
    }

    // shading
    if (unsorted.hasShading) {
      if (!oldState->unsorted.hasShading)
	glShadeModel(GL_SMOOTH);
    }
    else {
      if (oldState->unsorted.hasShading)
	glShadeModel(GL_FLAT);
    }

    // alpha func
    if (unsorted.hasAlphaFunc) {
      if (oldState->unsorted.hasAlphaFunc) {
	if (unsorted.alphaFunc != oldState->unsorted.alphaFunc ||
	    unsorted.alphaRef != oldState->unsorted.alphaRef)
	  glAlphaFunc(unsorted.alphaFunc, unsorted.alphaRef);
      }
      else {
	glAlphaFunc(unsorted.alphaFunc, unsorted.alphaRef);
	glEnable(GL_ALPHA_TEST);
      }
    }
    else {
      if (oldState->unsorted.hasAlphaFunc)
	glDisable(GL_ALPHA_TEST);
    }
  }
  else {
    // texture mapping
    if (sorted.hasTexture) {
      sorted.texture.execute();
      glEnable(GL_TEXTURE_2D);
      if (sorted.hasTextureReplace)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, MY_GL_REPLACE);
    }
    else {
      glDisable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    // lighting and material
    if (sorted.hasMaterial) {
      sorted.material.execute();
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
    }
    else {
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
    }

    // blending and blend function
    if (unsorted.hasBlending) {
      glBlendFunc(unsorted.blendSFactor, unsorted.blendDFactor);
      glEnable(GL_BLEND);
    }
    else {
      glDisable(GL_BLEND);
    }

    // antialiasing
    if (unsorted.hasSmoothing) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_POINT_SMOOTH);
    }
    else {
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_POINT_SMOOTH);
    }

    // stippling
    if (unsorted.hasStipple) {
      OpenGLGState::setStippleIndex(unsorted.stippleIndex);
      glEnable(GL_LINE_STIPPLE);
      glEnable(GL_POLYGON_STIPPLE);
    }
    else {
      glDisable(GL_LINE_STIPPLE);
      glDisable(GL_POLYGON_STIPPLE);
    }

    // texture mapping
    if (unsorted.hasCulling) {
      glCullFace(unsorted.culling);
      glEnable(GL_CULL_FACE);
    }
    else {
      glDisable(GL_CULL_FACE);
    }

    // shading
    if (unsorted.hasShading) {
      glShadeModel(GL_SMOOTH);
    }
    else {
      glShadeModel(GL_FLAT);
    }

    // alpha function
    if (unsorted.hasAlphaFunc) {
      glAlphaFunc(unsorted.alphaFunc, unsorted.alphaRef);
      glEnable(GL_ALPHA_TEST);
    }
    else {
      glDisable(GL_ALPHA_TEST);
    }
  }
}

//
// SortedGState
//

class SortedGState {
  public:
    void		addRenderNode(RenderNode* node,
				const OpenGLGState* gstate)
				{ nodes.append(node, gstate); }

    static void		add(OpenGLGStateRep*);
    static void		remove(OpenGLGStateRep*);
    static void		clearRenderNodes();
    static void		render();

  protected: // only protected to shutup compiler;  would be private
			SortedGState(const OpenGLGStateState::Sorted& state);
			~SortedGState();

  private:
    void		doAdd(OpenGLGStateRep*);
    void		doRemove(OpenGLGStateRep*);

  private:
    SortedGState*		prev;
    SortedGState*		next;
    OpenGLGStateState::Sorted	state;
    OpenGLGStateRep*		firstGState;
    RenderNodeGStateList	nodes;
    static SortedGState*	list;
};

SortedGState*		SortedGState::list = NULL;

SortedGState::SortedGState(const OpenGLGStateState::Sorted& _state) :
				prev(NULL),
				next(NULL),
				state(_state),
				firstGState(NULL)
{
  // insert me into the proper spot in the list
  if (!list || state < list->state) {
    prev = NULL;
    next = list;
    list = this;
  }
  else {
    prev = list;
    for (SortedGState* scan = prev->next; scan; prev = scan, scan = scan->next)
      if (state < scan->state)
	break;
    next = prev->next;
  }
  if (next) next->prev = this;
  if (prev) prev->next = this;
}

SortedGState::~SortedGState()
{
  if (prev) prev->next = next;
  else list = next;
  if (next) next->prev = prev;
}

void			SortedGState::add(OpenGLGStateRep* rep)
{
  assert(rep != NULL);

  // check for the state in the list
  const OpenGLGStateState::Sorted& repSorted = rep->getState().sorted;
  for (SortedGState* scan = list; scan; scan = scan->next)
    if (scan->state == repSorted) {
      scan->doAdd(rep);
      return;
    }

  // not on list -- make a new one and add it to the list
  SortedGState* newState = new SortedGState(repSorted);
  newState->doAdd(rep);
}

void			SortedGState::remove(OpenGLGStateRep* rep)
{
  assert(rep != NULL && rep->bucket != NULL);
  SortedGState* bucket = rep->bucket;
  bucket->doRemove(rep);

  // if no gstates under sorted gstate then remove it
  if (bucket->firstGState == NULL) delete bucket;
}

void			SortedGState::doAdd(OpenGLGStateRep* rep)
{
  rep->bucket = this;
  rep->prev = NULL;
  rep->next = firstGState;
  if (firstGState) firstGState->prev = rep;
  firstGState = rep;
}

void			SortedGState::doRemove(OpenGLGStateRep* rep)
{
  if (rep->prev == NULL) firstGState = rep->next;
  else rep->prev->next = rep->next;
  if (rep->next) rep->next->prev = rep->prev;
  rep->bucket = NULL;
  rep->prev = NULL;
  rep->next = NULL;
}

void			SortedGState::clearRenderNodes()
{
  for (SortedGState* scan = list; scan; scan = scan->next)
    scan->nodes.clear();
}

void			SortedGState::render()
{
  for (SortedGState* scan = list; scan; scan = scan->next)
    scan->nodes.render();
}

//
// OpenGLGStateRep
//

OpenGLGStateState*	OpenGLGStateRep::lastState = NULL;

OpenGLGStateRep::OpenGLGStateRep() : refCount(1)
{
  SortedGState::add(this);
}

OpenGLGStateRep::OpenGLGStateRep(const OpenGLGStateState& _state) :
				refCount(1),
				state(_state)
{
  SortedGState::add(this);
}

OpenGLGStateRep::~OpenGLGStateRep()
{
  SortedGState::remove(this);

  // forget last state if it's me
  if (lastState == &state) resetState();
}

void			OpenGLGStateRep::ref()
{
  refCount++;
}

void			OpenGLGStateRep::unref()
{
  if (refCount == 1) delete this;
  else refCount--;
}

void			OpenGLGStateRep::addRenderNode(RenderNode* node,
				const OpenGLGState* gstate)
{
  assert(bucket != NULL);
  bucket->addRenderNode(node, gstate);
}

void			OpenGLGStateRep::resetState()
{
  if (lastState) {
    lastState->resetOpenGLState();
    lastState = NULL;
  }
}

void			OpenGLGStateRep::setState()
{
  state.setOpenGLState(lastState);
  lastState = &state;
}

//
// OpenGLGState
//

const int NumStipples = 9;
static const GLubyte	stipplePattern[NumStipples][4] = {
				{ 0x00, 0x00, 0x00, 0x00 },
				{ 0x88, 0x00, 0x22, 0x00 },
				{ 0xaa, 0x00, 0xaa, 0x00 },
				{ 0xaa, 0x44, 0xaa, 0x44 },
				{ 0xaa, 0x55, 0xaa, 0x55 },
				{ 0xea, 0x55, 0xbb, 0x55 },
				{ 0xff, 0x55, 0xff, 0x55 },
				{ 0xff, 0xdd, 0xff, 0x77 },
				{ 0xff, 0xff, 0xff, 0xff },
			};
GLuint			OpenGLGState::stipples = 0u;

OpenGLGState::OpenGLGState()
{
  rep = new OpenGLGStateRep();
}

OpenGLGState::OpenGLGState(const OpenGLGState& state)
{
  rep = state.rep;
  rep->ref();
}

OpenGLGState::OpenGLGState(const OpenGLGStateState& state)
{
  rep = new OpenGLGStateRep(state);
}

OpenGLGState::~OpenGLGState()
{
  rep->unref();
}

OpenGLGState&		OpenGLGState::operator=(const OpenGLGState& state)
{
  state.rep->ref();
  rep->unref();
  rep = state.rep;
  return *this;
}

void			OpenGLGState::setState() const
{
  rep->setState();
}

boolean			OpenGLGState::isBlended() const
{
  return rep->isBlended();
}

boolean			OpenGLGState::isTextured() const
{
  return rep->isTextured();
}

boolean			OpenGLGState::isTextureReplace() const
{
  return rep->isTextureReplace();
}

boolean			OpenGLGState::isLighted() const
{
  return rep->isLighted();
}

void			OpenGLGState::resetState()
{
  OpenGLGStateRep::resetState();
}

void			OpenGLGState::addRenderNode(RenderNode* node) const
{
  rep->addRenderNode(node, this);
}

void			OpenGLGState::clearLists()
{
  SortedGState::clearRenderNodes();
}

void			OpenGLGState::renderLists()
{
  SortedGState::render();
}

void			OpenGLGState::setStipple(GLfloat alpha)
{
  setStippleIndex(getStippleIndex(alpha));
}

void			OpenGLGState::setStippleIndex(int index)
{
  glCallList(stipples + index);
}

int			OpenGLGState::getStippleIndex(float alpha)
{
  return (int)((float)(NumStipples - 1) * alpha + 0.5f);
}

int			OpenGLGState::getOpaqueStippleIndex()
{
  return NumStipples - 1;
}

void			OpenGLGState::initStipple()
{
  stipples = glGenLists(NumStipples);
  for (int i = 0; i < NumStipples; i++) {
    GLubyte stipple[132];
    GLubyte* sPtr = (GLubyte*)(((unsigned long)stipple & ~3) + 4);
    GLushort lineStipple;
    for (int j = 0; j < 128; j += 16) {
      sPtr[j+0] = stipplePattern[i][0];
      sPtr[j+1] = stipplePattern[i][0];
      sPtr[j+2] = stipplePattern[i][0];
      sPtr[j+3] = stipplePattern[i][0];
      sPtr[j+4] = stipplePattern[i][1];
      sPtr[j+5] = stipplePattern[i][1];
      sPtr[j+6] = stipplePattern[i][1];
      sPtr[j+7] = stipplePattern[i][1];
      sPtr[j+8] = stipplePattern[i][2];
      sPtr[j+9] = stipplePattern[i][2];
      sPtr[j+10] = stipplePattern[i][2];
      sPtr[j+11] = stipplePattern[i][2];
      sPtr[j+12] = stipplePattern[i][3];
      sPtr[j+13] = stipplePattern[i][3];
      sPtr[j+14] = stipplePattern[i][3];
      sPtr[j+15] = stipplePattern[i][3];
    }
    lineStipple = (GLushort)stipplePattern[i][0] +
			(((GLushort)stipplePattern[i][0]) << 8);
    glNewList(stipples + i, GL_COMPILE);
      glPolygonStipple(sPtr);
      glLineStipple(1, lineStipple);
    glEndList();
  }
}

void			OpenGLGState::init()
{
  // choose texture replace enum
#if defined(GL_EXT_texture)
  // IMPACT, RE, RE2, and VTX known not to support GL_REPLACE_EXT.
  // doesn't matter... they wouldn't go any faster.
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  if (strncmp(renderer, "IMPACT", 6) != 0 &&
      strncmp(renderer, "REC/", 4) != 0 &&
      strncmp(renderer, "RE/", 3) != 0 &&
      strncmp(renderer, "REV/", 2) != 0 &&
      strstr((const char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture"))
    MY_GL_REPLACE = GL_REPLACE_EXT;
#elif defined(GL_VERSION_1_1)
  MY_GL_REPLACE = GL_REPLACE;
#endif

  // initialize GL state to what we expect
  glDisable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);	
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_POINT_SMOOTH);
  glDisable(GL_LINE_STIPPLE);
  glDisable(GL_POLYGON_STIPPLE);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_FLAT);
  glDisable(GL_ALPHA_TEST);

  // other initialization
  initStipple();
}

//
// OpenGLGStateBuilder
//

OpenGLGStateBuilder::OpenGLGStateBuilder()
{
  state = new OpenGLGStateState;
}

OpenGLGStateBuilder::OpenGLGStateBuilder(const OpenGLGState& gstate)
{
  state = new OpenGLGStateState;
  init(gstate);
}

OpenGLGStateBuilder::~OpenGLGStateBuilder()
{
  delete state;
}

OpenGLGStateBuilder&	OpenGLGStateBuilder::operator=(
				const OpenGLGState& gstate)
{
  init(gstate);
  return *this;
}

void			OpenGLGStateBuilder::init(const OpenGLGState& gstate)
{
  *state = gstate.rep->getState();
}

void			OpenGLGStateBuilder::reset()
{
  state->reset();
}

void			OpenGLGStateBuilder::enableTexture(boolean on)
{
  state->enableTexture(on);
}

void			OpenGLGStateBuilder::enableTextureReplace(boolean on)
{
  state->enableTextureReplace(on);
}

void			OpenGLGStateBuilder::enableMaterial(boolean on)
{
  state->enableMaterial(on);
}

void			OpenGLGStateBuilder::resetBlending()
{
  state->setBlending(GL_ONE, GL_ZERO);
}

void			OpenGLGStateBuilder::resetSmoothing()
{
  state->setSmoothing(False);
}

void			OpenGLGStateBuilder::resetAlphaFunc()
{
  state->setAlphaFunc(GL_ALWAYS, 0.0f);
}

void			OpenGLGStateBuilder::setTexture(
					const OpenGLTexture& texture)
{
  state->setTexture(texture);
}

void			OpenGLGStateBuilder::setMaterial(
					const OpenGLMaterial& material)
{
  state->setMaterial(material);
}

void			OpenGLGStateBuilder::setBlending(
					GLenum sFactor, GLenum dFactor)
{
  state->setBlending(sFactor, dFactor);
}

void			OpenGLGStateBuilder::setStipple(float alpha)
{
  state->setStipple(alpha);
}

void			OpenGLGStateBuilder::setSmoothing(boolean smooth)
{
  state->setSmoothing(smooth);
}

void			OpenGLGStateBuilder::setCulling(GLenum culling)
{
  state->setCulling(culling);
}

void			OpenGLGStateBuilder::setShading(GLenum shading)
{
  state->setShading(shading);
}

void			OpenGLGStateBuilder::setAlphaFunc(
					GLenum func, GLclampf ref)
{
  state->setAlphaFunc(func, ref);
}

OpenGLGState		OpenGLGStateBuilder::getState() const
{
  return OpenGLGState(*state);
}
