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

#include "common.h"
#include <stack>
#include "RegionReader.h"
#include "RegionShape.h"
#include "TransformedShape.h"
#include "ShapeBox.h"
#include "ShapePyramid.h"

//
// RegionReader
//

RegionShape*			RegionReader::parseShape(XMLTree::iterator xml)
{
	typedef std::stack<RegionShape*> ShapeStack;

	ShapeStack stack;
	try {
		// parse post-fix region shape expression
		XMLTree::sibling_iterator scan = xml.begin();
		XMLTree::sibling_iterator end  = xml.end();
		for (; scan != end; ++scan) {
			if (scan->type == XMLNode::Tag) {
				RegionShapePrimitive* shape = parseShapeShape(scan);
				if (shape != NULL) {
					stack.push(shape);
				}
				// note -- other shapes go here
				else if (scan->value == "union") {
					if (stack.size() < 2)
						throw XMLIOException(scan->position,
							string_util::format(
								"missing argument for `%s'",
								scan->value.c_str()));
					RegionShape* b = stack.top();
					stack.pop();
					RegionShape* a = stack.top();
					stack.pop();
					stack.push(new RegionShapeUnion(a, b));
				}
				else if (scan->value == "difference") {
					if (stack.size() < 2)
						throw XMLIOException(scan->position,
							string_util::format(
								"missing argument for `%s'",
								scan->value.c_str()));
					RegionShape* b = stack.top();
					stack.pop();
					RegionShape* a = stack.top();
					stack.pop();
					stack.push(new RegionShapeDifference(a, b));
				}
				else if (scan->value == "intersection") {
					if (stack.size() < 2)
						throw XMLIOException(scan->position,
							string_util::format(
								"missing argument for `%s'",
								scan->value.c_str()));
					RegionShape* b = stack.top();
					stack.pop();
					RegionShape* a = stack.top();
					stack.pop();
					stack.push(new RegionShapeIntersection(a, b));
				}
				else {
					throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
				}
			}
		}

		// there should be exactly one region shape on the stack now
		if (stack.size() != 1)
			throw XMLIOException(xml->position,
								"invalid shape expression");
	}
	catch (...) {
		// clean up stack
		while (!stack.empty()) {
			delete stack.top();
			stack.pop();
		}

		// forward exception
		throw;
	}

	return stack.top();
}

RegionShapePrimitive*	RegionReader::parseShapePrimitive(XMLTree::iterator xml)
{
	RegionShapePrimitive* leaf = NULL;

	// find shape
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type == XMLNode::Tag) {
			RegionShapePrimitive* shape = parseShapeShape(scan);
			if (shape != NULL) {
				if (leaf == NULL)
					leaf = shape;
				else
					throw XMLIOException(xml->position,
							string_util::format(
								"only one shape allowed in `%s'",
								xml->value.c_str()));
			}
			else {
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
			}
		}
	}

	// there should be exactly one region shape
	if (leaf == NULL)
		throw XMLIOException(xml->position, "missing shape");

	return leaf;
}

RegionShapePrimitive*	RegionReader::parseShapeShape(XMLTree::iterator xml)
{
	assert(xml->type == XMLNode::Tag);

	if (xml->value == "box")
		return parseBox(xml);
	if (xml->value == "pyramid")
		return parsePyramid(xml);
	// note -- other shapes go here

	return NULL;
}

bool					RegionReader::parseTransformTag(
								Matrix& xform, XMLTree::iterator xml)
{
	assert(xml->type == XMLNode::Tag);
	if (xml->value == "translate") {
		float x = 0.0f, y = 0.0f, z = 0.0f;
		xml->getAttribute("x", xmlStrToFloat(xmlSetVar(x)));
		xml->getAttribute("y", xmlStrToFloat(xmlSetVar(y)));
		xml->getAttribute("z", xmlStrToFloat(xmlSetVar(z)));
		Matrix m;
		m.setTranslate(x, y, z);
		xform *= m;
	}
	else if (xml->value == "rotate") {
		float x = 0.0f, y = 0.0f, z = 0.0f, a = 0.0f;
		xml->getAttribute("x", xmlStrToFloat(xmlSetVar(x)));
		xml->getAttribute("y", xmlStrToFloat(xmlSetVar(y)));
		xml->getAttribute("z", xmlStrToFloat(xmlSetVar(z)));
		xml->getAttribute("a", xmlStrToFloat(xmlSetVar(a)));
		Matrix m;
		m.setRotate(x, y, z, a);
		xform *= m;
	}
	else {
		return false;
	}
	return true;
}

RegionShapePrimitive*	RegionReader::parseBox(XMLTree::iterator xml)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	Matrix xform;

	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type == XMLNode::Tag) {
			if (scan->value == "size") {
				scan->getAttribute("x", xmlStrToFloat(xmlSetVar(x)));
				scan->getAttribute("y", xmlStrToFloat(xmlSetVar(y)));
				scan->getAttribute("z", xmlStrToFloat(xmlSetVar(z)));
			}
			else if (!parseTransformTag(xform, scan)) {
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
			}
		}
	}

	if (x < 0.0f || y < 0.0f || z < 0.0f)
		throw XMLIOException(xml->position, "box is too small");

	return new RegionShapePrimitive(new TransformedShape(
								new ShapeBox(x, y, z), xform));
}

RegionShapePrimitive*	RegionReader::parsePyramid(XMLTree::iterator xml)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	Matrix xform;

	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type == XMLNode::Tag) {
			if (scan->value == "size") {
				scan->getAttribute("x", xmlStrToFloat(xmlSetVar(x)));
				scan->getAttribute("y", xmlStrToFloat(xmlSetVar(y)));
				scan->getAttribute("z", xmlStrToFloat(xmlSetVar(z)));
			}
			else if (!parseTransformTag(xform, scan)) {
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
			}
		}
	}

	if (x < 0.0f || y < 0.0f || z < 0.0f)
		throw XMLIOException(xml->position, "pyramid is too small");

	return new RegionShapePrimitive(new TransformedShape(
								new ShapePyramid(x, y, z), xform));
}
// ex: shiftwidth=4 tabstop=4
