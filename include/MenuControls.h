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

#ifndef BZF_MENU_CONTROLS_H
#define BZF_MENU_CONTROLS_H

#include "common.h"
#include "BzfString.h"
#include "BzfEvent.h"
#include "TimeKeeper.h"
#include "OpenGLTexFont.h"
#include <vector>

class MenuControl {
public:
	MenuControl();
	virtual ~MenuControl();

	// returns true iff the control can take focus.  default returns true.
	virtual bool		isActive() const;

	// returns true iff the focus should not be displayed for this control.
	// default returns false.
	virtual bool		isFocusHidden() const;

	// show/hide control
	void				setHidden(bool);
	bool				isHidden() const;

	// get/set control's label, font, and color
	void				setLabel(const BzfString&);
	BzfString			getLabel() const;
	void				setFont(const OpenGLTexFont&);
	const OpenGLTexFont& getFont() const;
	void				setColor(float r, float g, float b);
	void				getColor(float& r, float& g, float& b) const;

	// window size independent control height
	void				setHeight(float pixels, float fraction);
	float				getHeightPixels() const;
	float				getHeightFraction() const;

	// calculates control's true size from window size
	void				calcSize(float wWindow, float hWindow);

	// get the control's true size and the true position of the
	// detail (must be called after calcSize())
	float				getWidth() const;
	float				getHeight() const;
	float				getDetailX() const;
	float				getDetailY() const;

	// get the number of lines of text in the control.  default
	// returns 1.
	virtual float		getHeightMultiplier() const;

	// initialize.  this usually means loading current state from
	// the state database.  default does nothing.
	virtual void		init();

	// draw
	virtual void		render(float x, float y, bool isFocus) const = 0;

	// handle keyboard events
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	// set the OpenGL color
	void				setColor() const;

	// called by calcSize() after it computes the true height and
	// sets the font size (both width and height to the true
	// height).  must return the true width and the detail position.
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail) = 0;

	// set the detail position
	void				setDetail(float x, float y);

private:
	bool				hidden;
	BzfString			label;
	float				hPixels;
	float				hFraction;
	OpenGLTexFont		font;
	float				w, h, xDetail, yDetail;
	float				color[3];
};

class MenuCombo : public MenuControl {
public:
	MenuCombo();
	virtual ~MenuCombo();

	void				setName(const BzfString&);
	BzfString			getName() const;
	void				setDefault(const BzfString&);
	BzfString			getDefault() const;

	void				append(const BzfString& label);
	void				append(const BzfString& label, const BzfString& value);

	virtual void		init();
	virtual void		render(float x, float y, bool isFocus) const;
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail);

	void				setActive(int);

private:
	typedef std::pair<bool, BzfString> Value;
	typedef std::vector<std::pair<BzfString, Value> > Options;

	BzfString			name;
	BzfString			defValue;
	Options				options;
	int					active;
	float				wLabel;
};

class MenuEdit : public MenuControl {
public:
	MenuEdit();
	virtual ~MenuEdit();

	void				setName(const BzfString&);
	BzfString			getName() const;

	void				setMaxLength(int);
	void				setNumeric(bool);

	virtual void		init();
	virtual void		render(float x, float y, bool isFocus) const;
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail);

private:
	void				onChange();
	static void			onChangeCB(const BzfString&, void*);

private:
	BzfString			name;
	BzfString			value;
	int					maxLength;
	bool				numeric;
};

class MenuButton : public MenuControl {
public:
	MenuButton();
	virtual ~MenuButton();

	void				setAction(const BzfString& cmd);

	virtual void		render(float x, float y, bool isFocus) const;
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail);

private:
	BzfString			cmd;
};

class MenuKeyBind : public MenuControl {
public:
	MenuKeyBind();
	virtual ~MenuKeyBind();

	void				setBindings(const BzfString& down,
							const BzfString& up);

	virtual void		render(float x, float y, bool isFocus) const;
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
								float& width, float& detail);

private:
	void				clean();
	void				onChanged(const BzfString& name, bool press,
							const BzfString& cmd);
	void				onScan(const BzfString& name, bool press,
							const BzfString& cmd);
	static void			onChangedCB(const BzfString& name, bool press,
							const BzfString& cmd, void* userData);
	static void			onScanCB(const BzfString& name, bool press,
							const BzfString& cmd, void* userData);

private:
	bool				dirty;
	BzfString			cmdDown;
	BzfString			cmdUp;
	BzfString			keys;
	BzfString			key1;
	BzfString			key2;
	bool				binding;
};

class MenuLabel : public MenuControl {
public:
	MenuLabel();
	virtual ~MenuLabel();

	void				setName(const BzfString&);
	void				setTexture(const BzfString& filename);

	virtual bool		isActive() const;
	virtual void		render(float x, float y, bool isFocus) const;

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail);

private:
	void				onChange();
	static void			onChangeCB(const BzfString&, void*);

private:
	BzfString			name;
	BzfString			textureFile;
};

class MenuText : public MenuControl {
public:
	MenuText();
	virtual ~MenuText();

	void				setLines(int numLines);
	void				setWidth(float wPixels, float wFraction);
	void				setScrollSpeed(float linesPerSecond);
	void				setText(const BzfString&);

	virtual bool		isFocusHidden() const;
	virtual float		getHeightMultiplier() const;
	virtual void		init();
	virtual void		render(float x, float y, bool isFocus) const;
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail);

	void				onReshape();
	void				onScroll();
	static BzfString	wordBreak(const OpenGLTexFont& font,
							float w,
							const BzfString& msg,
							unsigned int* numLines);

private:
	float				numLines;
	float				wPixels;
	float				wFraction;
	BzfString			text;
	float				yOffset;
	float				speed;
	TimeKeeper			lastTime;

	// cache
	float				wOld, hOld, hLines;
	BzfString			formattedText;
	float				textHeight;
};

class MenuList : public MenuControl {
public:
	MenuList();
	virtual ~MenuList();

	// get/set the source name.  the control watches this name in the
	// state database.  when it changes, it takes the value as a list
	// of label/value pairs.  each label and value is delimited by a
	// newline (\n).  the control's list is cleared and replaced with
	// the new pairs.  the active item is updated to match the value
	// in the target name's database entry if there is such an item
	// (if not then the first item is used).
	void				setSourceName(const BzfString&);
	BzfString			getSourceName() const;

	// get/set the focus name.  when the user sets focus to an item in
	// the list, the control sets the value of this name in the database
	// to the value of the item with the focus.
	void				setFocusName(const BzfString&);
	BzfString			getFocusName() const;

	// get/set the target name(s).  when the user selects an item in the
	// list, the control sets the value of this name in the database
	// to the value of the selected item.  if name contains more than
	// one name separated by commas then the value format should be used
	// to split the item's value into parts.  each part is assigned to
	// corresponding item in the target list.
	//
	// for example, given the value format "%;%" and the target name
	// "foo;bar" then foo gets assigned everything in the item's value
	// up to the first semicolon and bar gets everything after the
	// semicolon.  the semicolon is discarded.
	//
	// if parsing the item's value doesn't extract enough parts to set
	// all targets then the remaining targets are unset.
	void				setTargetName(const BzfString&);
	BzfString			getTargetName() const;

	// get/set the value format.  this describes the format of the
	// value of each item.  the format is a sequence of characters.
	// a character other than % is a literal character and the value
	// will be scanned until the character is found.  literals are
	// discarded.  a % must be followed by a non-%;  characters up
	// to (but not including) the non-% are appended onto a list of
	// strings.  if the last % isn't followed by a non-% then the
	// remaining characters are appended onto the list.
	void				setValueFormat(const BzfString&);
	BzfString			getValueFormat() const;

	// get/set the select command.  when the user selects an item in
	// the list and after the target has been set, this command is
	// run.
	void				setSelectCommand(const BzfString&);
	BzfString			getSelectCommand() const;

	// set size
	void				setLines(unsigned int numLines);
	void				setColumns(unsigned int numColumns);
	void				setWidth(float wPixels, float wFraction);

	// clear the list and append new label/value pairs
	void				clear();
	void				append(const BzfString& label, const BzfString& value);

	// MenuControl overrides
	// note:  a list control doesn't draw the label
	virtual float		getHeightMultiplier() const;
	virtual void		init();
	virtual void		render(float x, float y, bool isFocus) const;
	virtual bool		onKeyPress(const BzfKeyEvent&);
	virtual bool		onKeyRelease(const BzfKeyEvent&);

protected:
	virtual void		onCalcSize(float wWindow, float hWindow,
							float& width, float& detail);

private:
	void				setTarget();

	void				onSourceChanged(const BzfString&);
	static void			onSourceChangedCB(const BzfString&, void*);

private:
	typedef BzfString Value;
	typedef std::vector<std::pair<BzfString, Value> > Items;
	typedef std::vector<BzfString> Names;

	BzfString			sourceName;
	BzfString			focusName;
	BzfString			targetName;
	BzfString			valueFormat;
	BzfString			selectCommand;
	Names				targetNames;
	unsigned int		numLines;
	unsigned int		numColumns;
	unsigned int		cOffset;
	float				wPixels;
	float				wFraction;
	Items				items;
	unsigned int		active;
	float				dSize;
};

#endif
