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

#ifndef BZF_VIEWITEMMESSAGES_H
#define BZF_VIEWITEMMESSAGES_H

#include "View.h"

class MessageBuffer;

class ViewItemMessages : public View {
public:
	ViewItemMessages(MessageBuffer*);

	void				setTimeout(float);
	void				showInput(bool);
	void				setShadow(bool);

protected:
	virtual ~ViewItemMessages();

	// View overrides
	virtual bool		onPreRender(float x, float y, float w, float h);
	virtual void		onPostRender(float x, float y, float w, float h);

private:
	struct CallbackInfo {
	public:
		ViewItemMessages* self;
		const ViewState* state;
		unsigned int*	numLines;
		float			x, y, w;
	};
	bool				drawLine(const BzfString& msg, float w,
								const float* color, CallbackInfo* cbState);
	static bool			drawLineCB(const BzfString& msg,
								const float* color, void* _cbState);
	static bool			countLine(const BzfString& msg,
								const float* color, void* _cbState);

	BzfString			wordBreak(const OpenGLTexFont&, float w,
								const BzfString& msg, unsigned int* numLines);

private:
	MessageBuffer*		msgBuffer;
	float				timeout;
	bool				input;
	bool				shadow;
};

class ViewItemMessagesReader : public ViewTagReader {
public:
	ViewItemMessagesReader();
	virtual ~ViewItemMessagesReader();

	// ViewItemReader overrides
	virtual ViewTagReader* clone() const;
	virtual View*		open(const ConfigReader::Values&);
	virtual void		close();

private:
	ViewItemMessages*	item;
};

#endif
