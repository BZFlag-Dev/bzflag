/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "MenuControls.h"
#include "Menu.h"
#include "MenuManager.h"
#include "CommandManager.h"
#include "KeyManager.h"
#include "StateDatabase.h"
#include "bzfgl.h"
#include <ctype.h>
#include <math.h>

//
// MenuControl
//

MenuControl::MenuControl() : hidden(false),
								hPixels(0.0f), hFraction(0.0f),
								w(0.0f), h(0.0f),
								xDetail(0.0f), yDetail(0.0f)
{
	color[0] = color[1] = color[2] = 0.0f;
}

MenuControl::~MenuControl()
{
	// do nothing
}

bool					MenuControl::isActive() const
{
	return true;
}

bool					MenuControl::isFocusHidden() const
{
	return false;
}

void					MenuControl::setHidden(bool _hidden)
{
	hidden = _hidden;
}

bool					MenuControl::isHidden() const
{
	return hidden;
}

void					MenuControl::setColor(float r, float g, float b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
}

void					MenuControl::getColor(
								float& r, float& g, float& b) const
{
	r = color[0];
	g = color[1];
	b = color[2];
}

void					MenuControl::setLabel(const BzfString& _label)
{
	label = _label;
}

BzfString				MenuControl::getLabel() const
{
	return label;
}

void					MenuControl::setFont(const OpenGLTexFont& _font)
{
	font = _font;
}

const OpenGLTexFont&		MenuControl::getFont() const
{
	return font;
}

void					MenuControl::setHeight(float pixels, float fraction)
{
	hPixels   = pixels;
	hFraction = fraction;
}

float					MenuControl::getHeightPixels() const
{
	return hPixels;
}

float					MenuControl::getHeightFraction() const
{
	return hFraction;
}

void					MenuControl::calcSize(float wWindow, float hWindow)
{
	h = hPixels + hWindow * hFraction;
	if (font.isValid())
		font.setSize(h, h);
	onCalcSize(wWindow, hWindow, w, xDetail);
	yDetail = 0.0f;
	h *= getHeightMultiplier();
}

float					MenuControl::getWidth() const
{
	return w;
}

float					MenuControl::getHeight() const
{
	return h;
}

float					MenuControl::getDetailX() const
{
	return xDetail;
}

float					MenuControl::getDetailY() const
{
	return yDetail;
}

void					MenuControl::setDetail(float x, float y)
{
	xDetail = x;
	yDetail = y;
}

float					MenuControl::getHeightMultiplier() const
{
	return 1.0f;
}

void					MenuControl::init()
{
	// do nothing
}

bool					MenuControl::onKeyPress(const BzfKeyEvent&)
{
	return false;
}

bool					MenuControl::onKeyRelease(const BzfKeyEvent&)
{
	return false;
}

void					MenuControl::setColor() const
{
	glColor3fv(color);
}


//
// MenuCombo
//

MenuCombo::MenuCombo() : active(0)
{
	// do nothing
}

MenuCombo::~MenuCombo()
{
	// do nothing
}

void					MenuCombo::setName(const BzfString& _name)
{
	name = _name;
}

BzfString				MenuCombo::getName() const
{
	return name;
}

void					MenuCombo::setDefault(const BzfString& _defValue)
{
	defValue = _defValue;
}

BzfString				MenuCombo::getDefault() const
{
	return defValue;
}

void					MenuCombo::append(const BzfString& label)
{
	options.push_back(std::make_pair(label, std::make_pair(false, BzfString())));
}

void					MenuCombo::append(const BzfString& label,
								const BzfString& value)
{
	options.push_back(std::make_pair(label, std::make_pair(true, value)));
}

void					MenuCombo::init()
{
	active = -1;

	// look up the current value for name, if any, and pick active item
	if (BZDB->isSet(name)) {
		BzfString value = BZDB->get(name);
		for (Options::const_iterator index = options.begin();
								index != options.end(); ++index)
			if (index->second.first && index->second.second == value) {
				active = index - options.begin();
				break;
			}
	}

	// if no active item chosen and there's a default value then try that
	if (active == -1 && !defValue.empty()) {
		for (Options::const_iterator index = options.begin();
								index != options.end(); ++index)
			if (index->second.first && index->second.second == defValue) {
				active = index - options.begin();
				break;
			}
	}

	// use first item if no item matched
	if (active == -1)
		active = 0;
}

void					MenuCombo::render(float x, float y, bool) const
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	setColor();
	y -= font.getAscent() - 1;
	font.draw(getLabel(), x, y);
	if (!options.empty())
		font.draw(options[active].first, x + getDetailX() + getHeight(), y);
}

bool					MenuCombo::onKeyPress(const BzfKeyEvent& key)
{
	if (!MenuControl::onKeyPress(key) && !options.empty()) {
		int newActive = active;
		switch (key.button) {
			case BzfKeyEvent::Left:
				if (--newActive < 0)
					newActive = options.size() - 1;
				break;

			case BzfKeyEvent::Right:
				if (++newActive == (int)options.size())
					newActive = 0;
				break;

			case BzfKeyEvent::Home:
				newActive = 0;
				break;

			case BzfKeyEvent::End:
				newActive = options.size() - 1;
				break;

			default:
				return false;
		}

		setActive(newActive);
	}

	return true;
}

bool					MenuCombo::onKeyRelease(const BzfKeyEvent& key)
{
	return MenuControl::onKeyRelease(key);
}

void					MenuCombo::setActive(int _active)
{
	assert(_active >= 0 && _active < (int)options.size());
	active = _active;
	if (!name.empty() && options[active].second.first)
		BZDB->set(name, options[active].second.second);
}

void					MenuCombo::onCalcSize(float, float,
								float& width, float& detail)
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid()) {
		width = 0;
		detail = 0;
		return;
	}

	float w = 0.0;
	for (Options::const_iterator index = options.begin();
								index != options.end(); ++index) {
		float wTmp = font.getWidth(index->first);
		if (wTmp > w)
			w = wTmp;
	}
	detail = font.getWidth(getLabel());
	width = w + detail + getHeight();
}


//
// MenuEdit
//

MenuEdit::MenuEdit() : name(), value(), maxLength(0), numeric(false)
{
	// do nothing
}

MenuEdit::~MenuEdit()
{
	if (!name.empty())
		BZDB->removeCallback(name, &onChangeCB, this);
}

void					MenuEdit::setName(const BzfString& _name)
{
	if (!name.empty())
		BZDB->removeCallback(name, &onChangeCB, this);
	name = _name;
	if (!name.empty()) {
		BZDB->addCallback(name, &onChangeCB, this);
		onChange();
	}
}

BzfString				MenuEdit::getName() const
{
	return name;
}

void					MenuEdit::setMaxLength(int _maxLength)
{
	assert(_maxLength >= 0);
	maxLength = _maxLength;
}

void					MenuEdit::setNumeric(bool _numeric)
{
	numeric = _numeric;
}

void					MenuEdit::init()
{
	value = "";

	// get the value from the database
	if (!name.empty() && BZDB->isSet(name))
		value = BZDB->get(name);
}

void					MenuEdit::render(float x, float y, bool focus) const
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	// FIXME -- handle horizontal scrolling
	setColor();
	y -= font.getAscent() - 1;
	font.draw(getLabel(), x, y);
	font.draw(value, x + getDetailX() + getHeight(), y);
	if (focus)
		font.draw("_", x + getDetailX() + getHeight() + font.getWidth(value), y);
}

bool					MenuEdit::onKeyPress(const BzfKeyEvent& key)
{
	if (!MenuControl::onKeyPress(key)) {
		char c = key.ascii;
		if (c == 0) switch (key.button) {
			// FIXME -- handle arrow keys for interior editing

			case BzfKeyEvent::Delete:
				c = '\b';
				break;

			default:
				return false;
		}

		// ignore non-printable characters except backspace
		if (!isprint(c) && c != '\b')
			return false;

		// handle character
		if (c == '\b') {
			// do nothing if string is empty
			if (value.empty())
				goto noRoom;

			// remove last character
			value.truncate(value.size() - 1);
		}
		else {
			// convert all whitespace to space
			if (isspace(c))
				c = ' ';

			// do nothing if at maximum length
			if (maxLength > 0 && value.size() >= (BzfString::size_type)maxLength)
				goto noRoom;

			// do nothing if requiring numeric input and character isn't a digit
			if (numeric && !isdigit(c))
				goto noRoom;

			// append character
			value.append(&c, 1);
		}

		// edited
		if (!name.empty())
			BZDB->set(name, value);
	}

	return true;

noRoom:
	// ring bell?
	return true;
}

bool					MenuEdit::onKeyRelease(const BzfKeyEvent& key)
{
	// slurp up all printing characters
	if (MenuControl::onKeyRelease(key) || isprint(key.ascii))
		return true;

	// also eat backspace
	if (key.ascii == '\b' || key.button == BzfKeyEvent::Delete)
		return true;

	return false;
}

void					MenuEdit::onCalcSize(float, float,
								float& width, float& detail)
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid()) {
		width = 0;
		detail = 0;
		return;
	}

	const int n = (maxLength == 0 || maxLength > 40) ? 40 : maxLength;
	detail = font.getWidth(getLabel());
	width = detail + getHeight() + n * font.getWidth("N");
}

void					MenuEdit::onChange()
{
	if (!name.empty())
		value = BZDB->get(name);
}

void					MenuEdit::onChangeCB(const BzfString&, void* self)
{
	reinterpret_cast<MenuEdit*>(self)->onChange();
}


//
// MenuButton
//

MenuButton::MenuButton()
{
	// do nothing
}

MenuButton::~MenuButton()
{
	// do nothing
}

void					MenuButton::setAction(const BzfString& _cmd)
{
	cmd = _cmd;
}

void					MenuButton::render(float x, float y, bool) const
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	setColor();
	y -= font.getAscent() - 1;
	font.draw(getLabel(), x, y);
}

bool					MenuButton::onKeyPress(const BzfKeyEvent& key)
{
	if (MenuControl::onKeyPress(key))
		return true;

	if (key.ascii != 13)
		return false;

	// selected
	CMDMGR->run(cmd);

	return true;
}

bool					MenuButton::onKeyRelease(const BzfKeyEvent& key)
{
	return (MenuControl::onKeyRelease(key) || key.ascii == 13);
}

void					MenuButton::onCalcSize(float, float,
								float& width, float& detail)
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid()) {
		width = 0;
		detail = 0;
		return;
	}

	detail = font.getWidth(getLabel());
	width  = getHeight() + detail;
}


//
// MenuKeyBind
//

MenuKeyBind::MenuKeyBind() : dirty(true), binding(false)
{
	KEYMGR->addCallback(&onChangedCB, this);
}

MenuKeyBind::~MenuKeyBind()
{
	KEYMGR->removeCallback(&onChangedCB, this);
}

void					MenuKeyBind::setBindings(
								const BzfString& down,
								const BzfString& up)
{
	cmdDown = down;
	cmdUp   = up;
	dirty   = true;
}

void					MenuKeyBind::render(float x, float y, bool) const
{
	if (dirty)
		const_cast<MenuKeyBind*>(this)->clean();

	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	setColor();
	y -= font.getAscent() - 1;
	font.draw(getLabel(), x, y);
	font.draw(keys, x + font.getWidth(getLabel()) + getHeight(), y);
}

bool					MenuKeyBind::onKeyPress(const BzfKeyEvent& key)
{
	if (!binding) {
		if (MenuControl::onKeyPress(key))
			return true;

		if (key.ascii != 13)
			return false;

		binding = true;
		if (key2.empty()) {
			// update label
			keys  = key1;
			keys += " or ???";
		}
		else {
			// unbind keys
			BzfKeyEvent event;
			if (KEYMGR->stringToKeyEvent(key1, event)) {
				KEYMGR->unbind(event, true);
				KEYMGR->unbind(event, false);
			}
			if (KEYMGR->stringToKeyEvent(key2, event)) {
				KEYMGR->unbind(event, true);
				KEYMGR->unbind(event, false);
			}

			// update label
			dirty = false;
			keys  = "???";
		}
	}
	else {
		binding = false;
		dirty   = true;
		if (key.ascii != 27) {
			// bind new key
			KEYMGR->bind(key, true, cmdDown);
			if (!cmdUp.empty())
				KEYMGR->bind(key, false, cmdUp);
		}
	}

	return true;
}

bool					MenuKeyBind::onKeyRelease(const BzfKeyEvent& key)
{
	if (!binding)
		return (MenuControl::onKeyRelease(key) || key.ascii == 13);
	else
		return true;
}

void					MenuKeyBind::onCalcSize(float, float,
								float& width, float& detail)
{
	if (dirty)
		const_cast<MenuKeyBind*>(this)->clean();

	const OpenGLTexFont& font = getFont();
	if (!font.isValid()) {
		width = 0;
		detail = 0;
		return;
	}

	detail = font.getWidth(getLabel());
	width  = getHeight() + detail + font.getWidth(keys);
}

void					MenuKeyBind::clean()
{
	dirty = false;

	// get bound keys
	key1  = "";
	key2  = "";
	KEYMGR->iterate(&onScanCB, this);

	// assemble key binding label
	keys = key1;
	if (!key2.empty()) {
		keys += " or ";
		keys += key2;
	}
}

void					MenuKeyBind::onChanged(
								const BzfString& name, bool,
								const BzfString&)
{
	if (name == key1 || name == key2)
		dirty = true;
}

void					MenuKeyBind::onScan(
								const BzfString& name, bool press,
								const BzfString& cmd)
{
	if (press && cmd == cmdDown) {
		if (key1.empty())
			key1 = name;
		else if (key2.empty())
			key2 = name;
	}
}

void					MenuKeyBind::onChangedCB(
								const BzfString& name, bool press,
								const BzfString& cmd, void* userData)
{
	reinterpret_cast<MenuKeyBind*>(userData)->onChanged(name, press, cmd);
}

void					MenuKeyBind::onScanCB(
								const BzfString& name, bool press,
								const BzfString& cmd, void* userData)
{
	reinterpret_cast<MenuKeyBind*>(userData)->onScan(name, press, cmd);
}


//
// MenuLabel
//

MenuLabel::MenuLabel()
{
	// do nothing
}

MenuLabel::~MenuLabel()
{
	if (!name.empty())
		BZDB->removeCallback(name, &onChangeCB, this);
}

bool					MenuLabel::isActive() const
{
	return false;
}

void					MenuLabel::setName(const BzfString& _name)
{
	if (!name.empty())
		BZDB->removeCallback(name, &onChangeCB, this);
	name = _name;
	if (!name.empty()) {
		BZDB->addCallback(name, &onChangeCB, this);
		onChange();
	}
}

void					MenuLabel::setTexture(const BzfString& filename)
{
	textureFile = filename;
	// FIXME -- maybe load texture immediately
}

void					MenuLabel::render(float x, float y, bool) const
{
	// FIXME -- handle texture style
	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	setColor();
	y -= font.getAscent() - 1;
	if (!name.empty())
		font.draw(BZDB->get(name), x, y);
	else
		font.draw(getLabel(), x, y);
}

void					MenuLabel::onCalcSize(float, float,
								float& width, float& detail)
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid()) {
		width = 0;
		detail = 0;
		return;
	}

	detail = 0.0f;
	width = font.getWidth(getLabel());
}

void					MenuLabel::onChange()
{
	// do nothing.  we could cache the value here but we don't bother.
}

void					MenuLabel::onChangeCB(const BzfString&, void* self)
{
	reinterpret_cast<MenuLabel*>(self)->onChange();
}


//
// MenuText
//

MenuText::MenuText() : numLines(1.0f),
								wPixels(0.0f), wFraction(1.0f),
								yOffset(0.0f),
								speed(0.0f),
								wOld(0.0f), hOld(0.0f), hLines(0.0f),
								textHeight(0.0f)
{
	// do nothing
}

MenuText::~MenuText()
{
	// do nothing
}

void					MenuText::setLines(int _numLines)
{
	numLines = _numLines;
}

void					MenuText::setWidth(float _wPixels, float _wFraction)
{
	wPixels   = _wPixels;
	wFraction = _wFraction;
}

void					MenuText::setText(const BzfString& _text)
{
	text = _text;
}

void					MenuText::setScrollSpeed(float _speed)
{
	speed = _speed;
}

float					MenuText::getHeightMultiplier() const
{
	return numLines;
}

bool					MenuText::isFocusHidden() const
{
	return true;
}

void					MenuText::init()
{
	lastTime = TimeKeeper::getCurrent();

	if (speed > 0.0f) {
		yOffset = -hLines;
	}
	else if (speed < 0.0f) {
		yOffset = textHeight;
	}
	else {
		yOffset = 0.0f;
	}
}

void					MenuText::render(float x, float y, bool) const
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	// update cache
	const_cast<MenuText*>(this)->onReshape();

	// figure out the position of the scissor region
	int ww = static_cast<GLsizei>(wOld);
	int hw = static_cast<GLsizei>(hLines);
	int xw = static_cast<GLint>(x) + MENUMGR->top()->getX();
	int yw = static_cast<GLint>(y) + MENUMGR->top()->getY() - hw;

	// clip
	glPushAttrib(GL_SCISSOR_BIT);
	glScissor(xw, yw, ww, hw);

	// draw
	setColor();
	y -= font.getAscent() - 1;
	y += yOffset;
	font.draw(formattedText, x, y);

	// restore state
	glPopAttrib();

	// scroll
	const_cast<MenuText*>(this)->onScroll();
}

bool					MenuText::onKeyPress(const BzfKeyEvent& key)
{
	if (speed == 0.0f && key.ascii == 0) {
		float n;
		switch (key.button) {
			case BzfKeyEvent::Up:
				yOffset -= getFont().getSpacing();
				break;

			case BzfKeyEvent::Down:
				yOffset += getFont().getSpacing();
				break;

			case BzfKeyEvent::PageUp:
				n = floorf(hOld / getFont().getSpacing()) - 1;
				if (n < 1.0f)
					n = 1.0f;
				yOffset -= n * getFont().getSpacing();
				break;

			case BzfKeyEvent::PageDown:
				n = floorf(hOld / getFont().getSpacing()) - 1;
				if (n < 1.0f)
					n = 1.0f;
				yOffset += n * getFont().getSpacing();
				break;

			case BzfKeyEvent::Home:
				yOffset = 0.0f;
				break;

			case BzfKeyEvent::End:
				yOffset = hOld - textHeight;
				break;

			default:
				return MenuControl::onKeyPress(key);
		}

		// limit offset
		if (yOffset < 0.0f || textHeight <= hLines)
			yOffset = 0.0f;
		else if (textHeight - yOffset < hLines)
			yOffset = textHeight - hLines;
		return true;
	}

	return MenuControl::onKeyPress(key);
}

bool					MenuText::onKeyRelease(const BzfKeyEvent& key)
{
	if (speed == 0.0f && key.ascii == 0) {
		switch (key.button) {
			case BzfKeyEvent::Up:
			case BzfKeyEvent::Down:
			case BzfKeyEvent::PageUp:
			case BzfKeyEvent::PageDown:
			case BzfKeyEvent::Home:
			case BzfKeyEvent::End:
				return true;
		}
	}
	return MenuControl::onKeyRelease(key);
}

void					MenuText::onCalcSize(float wWindow, float,
								float& width, float& detail)
{
	width  = wPixels + wWindow * wFraction;
	detail = -getHeight();
}

void					MenuText::onReshape()
{
	float w = getWidth();
	float h = getHeight();
	if (w != wOld || h != hOld) {
		// FIXME -- should also update if font or font size changed
		wOld = w;
		hOld = h;
		unsigned int n;
		formattedText = wordBreak(getFont(), wOld, text, &n);
		textHeight = static_cast<float>(n) * getFont().getSpacing();
		hLines = numLines * getFont().getSpacing();
		if (speed == 0.0f) {
			if (textHeight <= hLines)
				yOffset = 0.0f;
			else if (textHeight - yOffset < hLines)
				yOffset = textHeight - hLines;
		}
		else {
			init();
		}
	}
}

void					MenuText::onScroll()
{
	if (speed != 0.0f) {
		TimeKeeper now(TimeKeeper::getCurrent());
		float dt = now - lastTime;
		lastTime = now;
		yOffset += dt * speed * getFont().getSpacing();
		if (yOffset < -hLines)
			yOffset = textHeight;
		else if (textHeight - yOffset < 0.0f)
			yOffset = -hLines;
	}
}

BzfString				MenuText::wordBreak(const OpenGLTexFont& font,
								float w,
								const BzfString& msg,
								unsigned int* numLines)
{
	BzfString split;
	*numLines = 0;
	const char* begin = msg.c_str();
	const char* end = begin + msg.size();
	const char* scan = strchr(begin, '\n');
	if (scan == NULL)
		scan = end;
	do {
		// check that portion fits on line
		int n = font.getLengthInWidth(w, begin, scan - begin);
		while (n < scan - begin) {
			// doesn't fit.  back up until we find a space.
			int n0 = n;
			while (n > 0 && !isspace(begin[n]))
				--n;

			// if we found a space then back up until we find a non-space
			while (n > 0 && isspace(begin[n - 1]))
				--n;

			// if we couldn't find a place to break then break inside a word
			if (n == 0)
				n = n0;

			// append the line
			split.append(begin, n);
			split += "\n";
			++(*numLines);

			// skip ahead
			begin += n;

			// skip leading whitespace
			while (begin != scan && isspace(*begin))
				++begin;

			// check next portion
			n = font.getLengthInWidth(w, begin, scan - begin);
		}

		// add remainder of line
		if (scan == end) {
			split.append(begin, scan - begin);
			++(*numLines);
		}
		else {
			split.append(begin, scan - begin + 1);
			++(*numLines);
			begin = scan + 1;
			scan  = strchr(begin, '\n');
			if (scan == NULL)
				scan = end;
		}
	} while (scan != end);

	return split;
}


//
// MenuList
//

MenuList::MenuList() : numLines(1), numColumns(1),
								cOffset(0),
								wPixels(0.0f), wFraction(0.0f),
								active(0)
{
	// do nothing
}

MenuList::~MenuList()
{
	setSourceName("");
}

void					MenuList::setSourceName(const BzfString& name)
{
	if (sourceName != name) {
		if (!sourceName.empty())
			BZDB->removeCallback(sourceName, &onSourceChangedCB, this);
		sourceName = name;
		if (!sourceName.empty())
			BZDB->addCallback(sourceName, &onSourceChangedCB, this);
		onSourceChanged(sourceName);
	}
}

BzfString				MenuList::getSourceName() const
{
	return sourceName;
}

void					MenuList::setFocusName(const BzfString& name)
{
	// discard old value, if any
	if (!focusName.empty())
		BZDB->unset(focusName);

	// save name
	focusName = name;

	// prep value
	if (!focusName.empty()) {
		// we don't want this value to be persistent or user modifiable
		BZDB->setPersistent(focusName, false);
		BZDB->setPermission(focusName, StateDatabase::ReadOnly);

		// update value
		if (active < items.size())
			BZDB->set(focusName, items[active].second);
	}
}

BzfString				MenuList::getFocusName() const
{
	return focusName;
}

void					MenuList::setTargetName(const BzfString& name)
{
	targetName = name;
	targetNames.clear();

	// parse
	const char* scan = targetName.c_str();
	const char* next = strchr(scan, ',');
	while (next != NULL) {
		// append name to list
		targetNames.push_back(BzfString(scan, next - scan));

		// prepare for next name
		scan = next + 1;
		if (*scan == '\0')
			break;

		// find next delimiter
		next = strchr(scan, ',');
	}

	// append last name
	if (*scan != '\0')
		targetNames.push_back(scan);
}

BzfString				MenuList::getTargetName() const
{
	return targetName;
}

void					MenuList::setValueFormat(const BzfString& format)
{
	valueFormat = format;
}

BzfString				MenuList::getValueFormat() const
{
	return valueFormat;
}

void					MenuList::setSelectCommand(const BzfString& name)
{
	selectCommand = name;
}

BzfString				MenuList::getSelectCommand() const
{
	return selectCommand;
}

void					MenuList::setLines(unsigned int _numLines)
{
	numLines = _numLines;
}

void					MenuList::setColumns(unsigned int _numColumns)
{
	numColumns = _numColumns;
}

void					MenuList::setWidth(float _wPixels, float _wFraction)
{
	wPixels   = _wPixels;
	wFraction = _wFraction;
}

void					MenuList::clear()
{
	items.clear();
	active = 0;
	if (!focusName.empty())
		BZDB->unset(focusName);
}

void					MenuList::append(
								const BzfString& label,
								const BzfString& value)
{
	// append item
	items.push_back(std::make_pair(label, value));

	// set focus to item if it's the first one
	if (items.size() == 1 && !focusName.empty())
		BZDB->set(focusName, items[active].second);
}

float					MenuList::getHeightMultiplier() const
{
	return numLines;
}

void					MenuList::init()
{
	active = 0;

	// construct current value
	BzfString current;
	if (targetNames.size() <= 1 || valueFormat.empty()) {
		// just one target and/or no value format.  use single target value.
		current = BZDB->get(targetName);
	}
	else {
		Names::const_iterator index = targetNames.begin();
		const char* scan = valueFormat.c_str();
		while (*scan != '\0') {
			if (*scan == '%') {
				// replace % in value format with the next target value.
				// if we've run out of targets then do nothing.
				if (index != targetNames.end()) {
					current += BZDB->get(*index);
					++index;
				}
			}
			else {
				// append literal character
				current.append(scan, 1);
			}

			// next format character
			++scan;
		}
	}

	// pick active item
	for (Items::const_iterator index = items.begin();
								index != items.end(); ++index) {
		if (index->second == current) {
			active = index - items.begin();
			if (!focusName.empty())
				BZDB->set(focusName, items[active].second);
			break;
		}
	}
}

void					MenuList::render(float x, float y, bool) const
{
	const OpenGLTexFont& font = getFont();
	if (!font.isValid())
		return;

	// compute number of (partially) filled columns
	const unsigned int filledColumns = (static_cast<int>(items.size()) +
								numLines - 1) / numLines;

	// if active item isn't visible then make it so
	unsigned cOffsetNew = cOffset;
	if (active < cOffsetNew * numLines) {
		cOffsetNew = active / numLines;
	}
	else if (active >= (cOffsetNew + numColumns) * numLines) {
		cOffsetNew = active / numLines;
		if (cOffsetNew > numColumns - 1)
			cOffsetNew -= numColumns - 1;
		else
			cOffsetNew = 0;
	}

	// if column offset is too big then fix it
	if (filledColumns > numColumns && filledColumns - cOffsetNew < numColumns)
		cOffsetNew = filledColumns - numColumns;

	// now save it new column offset
	const_cast<MenuList*>(this)->cOffset = cOffsetNew;

	// now draw the visible items
	setColor();
	y -= font.getAscent() - 1;
	for (unsigned int j = 0; j < numColumns; ++j) {
		// compute range of items in column
		unsigned int i = (j + cOffset) * numLines;
		unsigned int n = i + numLines;
		if (n > items.size())
			n = items.size();

		// compute position of top item in column
		float x0 = x + j * (getWidth() / numColumns);
		float y0 = y;
		for (; i < n; ++i) {
			if (i == active) {
				const_cast<MenuList*>(this)->setDetail(x0 - x, y0 - y);
			}
			font.draw(items[i].first, x0 + dSize, y0);
			y0 -= font.getSpacing();
		}
	}
}

bool					MenuList::onKeyPress(const BzfKeyEvent& key)
{
	if (MenuControl::onKeyPress(key))
		return true;

	if (key.ascii == 13) {
		// select item
		if (!items.empty() && !targetName.empty())
			setTarget();
		CMDMGR->run(selectCommand);

		return true;
	}
	else {
		int newActive = static_cast<int>(active);
		switch (key.button) {
			case BzfKeyEvent::Up:
				--newActive;
				break;

			case BzfKeyEvent::Down:
				++newActive;
				break;

			case BzfKeyEvent::PageUp:
				if (numColumns > 1)
					newActive -= numLines * (numColumns - 1);
				else
					newActive -= numLines;
				break;

			case BzfKeyEvent::PageDown:
				if (numColumns > 1)
					newActive += numLines * (numColumns - 1);
				else
					newActive += numLines;
				break;

			case BzfKeyEvent::Left:
				newActive -= numLines;
				break;

			case BzfKeyEvent::Right:
				newActive += numLines;
				break;

			case BzfKeyEvent::Home:
				newActive = 0;
				break;

			case BzfKeyEvent::End:
				newActive = items.size() - 1;
				break;

			default:
				return false;
		}

		// clamp
		if (newActive < 0)
			newActive = 0;
		else if (newActive >= static_cast<int>(items.size()))
			newActive = static_cast<int>(items.size()) - 1;

		// change active
		if (!items.empty()) {
			active = static_cast<unsigned int>(newActive);
			if (!focusName.empty())
				BZDB->set(focusName, items[active].second);
		}
	}

	return true;
}

bool					MenuList::onKeyRelease(const BzfKeyEvent& key)
{
	if (MenuControl::onKeyRelease(key))
		return true;
	if (key.ascii == 13)
		return true;
	switch (key.button) {
		case BzfKeyEvent::Left:
		case BzfKeyEvent::Right:
		case BzfKeyEvent::Home:
		case BzfKeyEvent::End:
			return true;
	}
	return false;
}

void					MenuList::onCalcSize(float wWindow, float,
								float& width, float& detail)
{
	dSize  = getHeight();
	width  = wPixels + wWindow * wFraction;
	detail = 0.0f;
}

void					MenuList::onSourceChanged(const BzfString& name)
{
	// get values
	BzfString values(BZDB->get(name));
	const char* scan = values.c_str();

	// clear current list
	clear();

	// parse and append
	while (*scan != '\0') {
		// parse label
		const char* label = scan;
		while (*scan != '\0' && *scan != '\n')
			++scan;
		if (*scan == '\0')
			break;
		const char* labelEnd = scan++;

		// parse value
		const char* value = scan;
		while (*scan != '\0' && *scan != '\n')
			++scan;
		if (*scan == '\0')
			break;
		const char* valueEnd = scan++;

		// add item
		append(BzfString(label, labelEnd - label),
				BzfString(value, valueEnd - value));
	}

	// update active item
	init();
}

void					MenuList::onSourceChangedCB(
								const BzfString& name, void* self)
{
	reinterpret_cast<MenuList*>(self)->onSourceChanged(name);
}

void					MenuList::setTarget()
{
	BzfString value = items[active].second;

	if (targetNames.size() <= 1 || valueFormat.empty()) {
		// just one target and/or no value format.  set single target value.
		BZDB->set(targetName, value);
	}
	else {
		// parse value format
		Names::iterator index = targetNames.begin();
		const char* formatScan = valueFormat.c_str();
		const char* valueScan  = value.c_str();
		while (*formatScan != '\0') {
			if (*formatScan == '%') {
				// find end of next value
				char end = formatScan[1];
				const char* valueEnd = valueScan;
				while (*valueEnd != '\0' && *valueEnd != end)
					++valueEnd;

				// set next target value for % in value format.
				// if we've run out of targets then do nothing.
				if (index != targetNames.end()) {
					BZDB->set(*index, BzfString(valueScan, valueEnd - valueScan));
					++index;
				}

				// skip past value
				valueScan = valueEnd;
			}
			else {
				// find literal character
				while (*valueScan != '\0')
					if (*valueScan++ == *formatScan)
						break;
			}

			// next format character
			++formatScan;
		}

		// unset remaining target names
		while (index != targetNames.end()) {
			BZDB->unset(*index);
			++index;
		}
	}
}
