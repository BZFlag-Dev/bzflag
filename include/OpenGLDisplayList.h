/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OpenGLDisplayList:
 *	Encapsulates an OpenGL display list.
 */

#ifndef	BZF_OPENGL_DISPLAYLIST_H
#define	BZF_OPENGL_DISPLAYLIST_H

#include "bzfgl.h"
#include "common.h"

class OpenGLDisplayList {
  public:
			OpenGLDisplayList();
			OpenGLDisplayList(const OpenGLDisplayList&);
			~OpenGLDisplayList();
    OpenGLDisplayList&	operator=(const OpenGLDisplayList&);

    bool		operator==(const OpenGLDisplayList&) const;
    bool		operator!=(const OpenGLDisplayList&) const;
    bool		operator<(const OpenGLDisplayList&) const;
    bool		isValid() const;
    GLuint		getList() const;

    void		begin();
    void		end();
    void		execute();

  private:
    class Rep {
      public:
			Rep();
			~Rep();
      public:
	int		refCount;
	GLuint		list;
    };

    void		ref();
    bool		unref();

  private:
    Rep*		rep;
};

#endif // BZF_OPENGL_DISPLAYLIST_H
// ex: shiftwidth=2 tabstop=8
