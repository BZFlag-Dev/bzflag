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

#include <string.h>
#include <assert.h>
#include "common.h"
#include "OpenGLGState.h"
#include "TextureManager.h"
#include "TextureMatrix.h"
#include "OpenGLMaterial.h"
#include "RenderNode.h"

static GLenum		MY_GL_REPLACE = GL_MODULATE;


// for tracking glBegin/End pairs; see include/bzfgl.h
#ifdef DEBUG
int __beginendCount;
#endif

//
// OpenGLGStateState
//

class OpenGLGStateState {
  public:
			OpenGLGStateState();
			~OpenGLGStateState();

    void		reset();
    void		enableTexture(bool);
    void		enableTextureReplace(bool);
    void		enableTextureMatrix(bool);
    void		enableMaterial(bool);
    void		setTexture(const int tex);
    void		setTextureMatrix(const int texmatrix);
    void		setMaterial(const OpenGLMaterial&);
    void		setBlending(GLenum sFactor, GLenum dFactor);
    void		setStipple(float alpha);
    void		setSmoothing(bool smooth);
    void		setCulling(GLenum culling);
    void		setShading(GLenum);
    void		setAlphaFunc(GLenum func, GLclampf ref);

    bool		isBlended() const
				{ return unsorted.hasBlending; }
    bool		isTextured() const
				{ return sorted.texture >= 0; }
    bool		isTextureReplace() const
				{ return sorted.hasTextureReplace; }
    bool		isTextureMatrix() const
				{ return sorted.hasTextureMatrix; }
    bool		isLighted() const
				{ return sorted.material.isValid(); }

    void		resetOpenGLState() const;
    void		setOpenGLState(const OpenGLGStateState* prev) const;

  public:
    class Sorted {
      public:
			Sorted();
			~Sorted();

	void		reset();
	bool		operator==(const Sorted&) const;
	bool		operator<(const Sorted&) const;

      public:
	bool		hasTexture;
	bool		hasTextureReplace;
	bool     	hasTextureMatrix;
	bool		hasMaterial;
	int 		texture;
	int      	textureMatrix;
	OpenGLMaterial	material;
    };

    class Unsorted {
      public:
			Unsorted();
			~Unsorted();

	void		reset();

      public:
	bool		hasBlending;
	bool		hasStipple;
	bool		hasSmoothing;
	bool		hasCulling;
	bool		hasShading;
	bool		hasAlphaFunc;
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

    bool		isBlended() { return state.isBlended(); }
    bool		isTextured() { return state.isTextured(); }
    bool		isTextureReplace() { return state.isTextureReplace(); }
    bool		isTextureMatrix() { return state.isTextureMatrix(); }
    bool		isLighted() { return state.isLighted(); }
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
				hasTexture(false),
				hasTextureReplace(false),
				hasTextureMatrix(false),
				hasMaterial(false),
				texture(-1),
				textureMatrix(-1),
				material(OpenGLMaterial())
{
  // do nothing
}

OpenGLGStateState::Sorted::~Sorted()
{
  // do nothing
}

void			OpenGLGStateState::Sorted::reset()
{
  hasTexture = false;
  hasTextureReplace = false;
  hasTextureMatrix = false;
  hasMaterial = false;
  texture = -1;
  textureMatrix = -1;
  material = OpenGLMaterial();
}

bool			OpenGLGStateState::Sorted::operator==(
				const Sorted& s) const
{
  if (hasTexture != s.hasTexture || texture != s.texture)
    return false;
  if (hasTextureReplace != s.hasTextureReplace)
    return false;
  if ((hasTextureMatrix != s.hasTextureMatrix) ||
      (textureMatrix != s.textureMatrix))
    return false;
  if (hasMaterial != s.hasMaterial || material != s.material)
    return false;
  return true;
}

bool			OpenGLGStateState::Sorted::operator<(
				const Sorted& s) const
{
  // do arbitrary sorting:
  // this < s if this has no texture and s does or texture < s.texture
  if (hasTexture != s.hasTexture) return s.hasTexture;
  if (hasTexture) {
    if (hasTextureReplace != s.hasTextureReplace) return s.hasTextureReplace;
    return texture < s.texture;
  }
  if (hasTextureMatrix != s.hasTextureMatrix) {
    return textureMatrix < s.textureMatrix;
  }

  // this < s if this has no material and s does or material < s.material
  if (hasMaterial != s.hasMaterial) return s.hasMaterial;
  if (hasMaterial) return material < s.material;

  // states are the same
  return false;
}

OpenGLGStateState::Unsorted::Unsorted() :
				hasBlending(false),
				hasStipple(false),
				hasSmoothing(false),
				hasCulling(true),
				hasShading(false),
				hasAlphaFunc(false),
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
  hasBlending = false;
  hasStipple = false;
  hasSmoothing = false;
  hasCulling = true;
  hasShading = false;
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

void			OpenGLGStateState::enableTexture(bool on)
{
  if (on)
    sorted.hasTexture = sorted.texture >= 0;
  else
    sorted.hasTexture = false;
}

void			OpenGLGStateState::enableTextureReplace(bool)
{
  // FIXME -- disabled for now
  //   most gstates haven't been adjusted to use/not-use texture
  //   replacement, so if this is turned on then those must be
  //   fixed appropriately.
  sorted.hasTextureReplace = false;
}

void			OpenGLGStateState::enableTextureMatrix(bool on)
{
  if (on)
    sorted.hasTextureMatrix = sorted.textureMatrix >= 0;
  else
    sorted.hasTexture = false;
}

void			OpenGLGStateState::enableMaterial(bool on)
{
  if (on) sorted.hasMaterial = sorted.material.isValid();
  else sorted.hasMaterial = false;
}

void			OpenGLGStateState::setTexture(
					const int _texture)
{
  sorted.hasTexture = _texture>=0;
  sorted.texture = _texture;
}

void			OpenGLGStateState::setTextureMatrix(
					const int _textureMatrix)
{
  sorted.hasTexture = _textureMatrix>=0;
  sorted.textureMatrix = _textureMatrix;
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

void			OpenGLGStateState::setSmoothing(bool smooth)
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
  if (sorted.hasTextureMatrix) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
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
  TextureManager &tm = TextureManager::instance();
  if (oldState == this) return;
  if (oldState) {
    // texture mapping
    if (sorted.hasTexture) {
      if (oldState->sorted.hasTexture) {
	if (sorted.texture != oldState->sorted.texture)
          tm.bind(sorted.texture);
      }
      else {
	tm.bind(sorted.texture);
	glEnable(GL_TEXTURE_2D);
      }
      if (!oldState->sorted.hasTextureReplace && sorted.hasTextureReplace) {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, MY_GL_REPLACE);
      }
    }
    else {
      if (oldState->sorted.hasTexture) {
	glDisable(GL_TEXTURE_2D);
	if (oldState->sorted.hasTextureReplace) {
	  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
      }
    }

    // texture transformation matrix
    if (sorted.hasTextureMatrix) {
      if (sorted.textureMatrix != oldState->sorted.textureMatrix) {
        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(TEXMATRIXMGR.getMatrix(sorted.textureMatrix)->getMatrix());
        glMatrixMode(GL_MODELVIEW);
      }
    }
    else {
      if (oldState->sorted.hasTextureMatrix) {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
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
      tm.bind(sorted.texture);
      glEnable(GL_TEXTURE_2D);
      if (sorted.hasTextureReplace)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, MY_GL_REPLACE);
    }
    else {
      glDisable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    // texture transformation matrix
    if (sorted.hasTextureMatrix) { //TREPAN
      if (sorted.textureMatrix != oldState->sorted.textureMatrix) {
        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(TEXMATRIXMGR.getMatrix(sorted.textureMatrix)->getMatrix());
        glMatrixMode(GL_MODELVIEW);
      }
    }
    else {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
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
// OpenGLGState::ContextInitializer
//

OpenGLGState::ContextInitializer*
			OpenGLGState::ContextInitializer::head = NULL;
OpenGLGState::ContextInitializer*
			OpenGLGState::ContextInitializer::tail = NULL;

OpenGLGState::ContextInitializer::ContextInitializer(
				OpenGLContextInitializer _callback,
				void* _userData) :
				callback(_callback),
				userData(_userData)
{
  prev = NULL;
  next = head;
  head = this;
  if (next) next->prev = this;
  else tail = this;
}

OpenGLGState::ContextInitializer::~ContextInitializer()
{
  // remove me from list
  if (next != NULL) next->prev = prev;
  else tail = prev;
  if (prev != NULL) prev->next = next;
  else head = next;
}

void			OpenGLGState::ContextInitializer::execute()
{
  ContextInitializer* scan = tail;
  while (scan) {
    (scan->callback)(scan->userData);
    scan = scan->prev;
  }
}

OpenGLGState::ContextInitializer*
			OpenGLGState::ContextInitializer::find(
				OpenGLContextInitializer _callback,
				void* _userData)
{
  ContextInitializer* scan = head;
  while (scan) {
    if (scan->callback == _callback && scan->userData == _userData)
      return scan;
    scan = scan->next;
  }
  return NULL;
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

bool			OpenGLGState::isBlended() const
{
  return rep->isBlended();
}

bool			OpenGLGState::isTextured() const
{
  return rep->isTextured();
}

bool			OpenGLGState::isTextureReplace() const
{
  return rep->isTextureReplace();
}

bool			OpenGLGState::isLighted() const
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

void			OpenGLGState::initStipple(void*)
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
  initGLState();

  // other initialization
  initStipple();

  // redo stipple init if context is recreated
  registerContextInitializer(initStipple);
}

void			OpenGLGState::registerContextInitializer(
				OpenGLContextInitializer callback,
				void* userData)
{
  if (callback == NULL)
    return;
  new ContextInitializer(callback, userData);
}

void			OpenGLGState::unregisterContextInitializer(
				OpenGLContextInitializer callback,
				void* userData)
{
  delete ContextInitializer::find(callback, userData);
}

void			OpenGLGState::initContext()
{
  // initialize GL state
  initGLState();

  // reset our idea of the state
  resetState();

  // call all initializers
  ContextInitializer::execute();

  // initialize the GL state again in case one of the initializers
  // messed it up.
  initGLState();

  // and some more state
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0);
  glClearStencil(0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_SCISSOR_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void			OpenGLGState::initGLState()
{
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

void			OpenGLGStateBuilder::enableTexture(bool on)
{
  state->enableTexture(on);
}

void			OpenGLGStateBuilder::enableTextureReplace(bool on)
{
  state->enableTextureReplace(on);
}

void			OpenGLGStateBuilder::enableTextureMatrix(bool on)
{
  state->enableTextureMatrix(on);
}

void			OpenGLGStateBuilder::enableMaterial(bool on)
{
  state->enableMaterial(on);
}

void			OpenGLGStateBuilder::resetBlending()
{
  state->setBlending(GL_ONE, GL_ZERO);
}

void			OpenGLGStateBuilder::resetSmoothing()
{
  state->setSmoothing(false);
}

void			OpenGLGStateBuilder::resetAlphaFunc()
{
  state->setAlphaFunc(GL_ALWAYS, 0.0f);
}

void			OpenGLGStateBuilder::setTexture(
					const int texture)
{
  state->setTexture(texture);
}

void			OpenGLGStateBuilder::setTextureMatrix(
					const int textureMatrix)
{
  state->setTextureMatrix(textureMatrix);
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

void			OpenGLGStateBuilder::setSmoothing(bool smooth)
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

