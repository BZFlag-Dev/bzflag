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

#if defined(WIN32)
#pragma warning(4:4503)
#endif

#include "StateDatabase.h"
#include "ErrorHandler.h"
#include <assert.h>
#include <ctype.h>
#include <stack>
#include <stdexcept>

//
// StateDatabase::Item
//

StateDatabase::Item::Item() : value(),
								defValue(),
								isSet(false),
								isTrue(false),
								save(true), // FIXME -- false by default?
								permission(ReadWrite)
{
	// do nothing
}


//
// StateDatabase
//

StateDatabase*			StateDatabase::s_instance = NULL;

StateDatabase::StateDatabase()
{
	// do nothing
}

StateDatabase::~StateDatabase()
{
	s_instance = NULL;
}

void					StateDatabase::set(const std::string& name,
								const std::string& value,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission) {
		index->second.value  = value;
		index->second.isSet  = true;
		index->second.isTrue = (index->second.value != "0" &&
						    index->second.value != "false" &&
						    index->second.value != "false" &&
						    index->second.value != "FALSE" &&
						    index->second.value != "no" &&
						    index->second.value != "No" &&
						    index->second.value != "NO");
		notify(index);
	}
}

void					StateDatabase::unset(const std::string& name,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission) {
		index->second.value  = "";
		index->second.isSet  = false;
		index->second.isTrue = false;
		notify(index);
	}
}

void					StateDatabase::touch(const std::string& name,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission)
		notify(index);
}

void					StateDatabase::setPersistent(
								const std::string& name, bool save)
{
	Map::iterator index = lookup(name);
	index->second.save = save;
}

void					StateDatabase::setDefault(
								const std::string& name, const std::string& value)
{
	Map::iterator index = lookup(name);
	index->second.defValue = value;
}

void					StateDatabase::setPermission(
								const std::string& name,
								Permission permission)
{
	Map::iterator index = lookup(name);
	index->second.permission = permission;
}

void					StateDatabase::addCallback(
								const std::string& name,
								Callback callback,
								void* userData)
{
	Map::iterator index = lookup(name);
	index->second.callbacks.add(callback, userData);
}

void					StateDatabase::removeCallback(
								const std::string& name,
								Callback callback,
								void* userData)
{
	Map::iterator index = lookup(name);
	index->second.callbacks.remove(callback, userData);
}

bool					StateDatabase::isSet(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return !(index == items.end() || !index->second.isSet);
}

std::string				StateDatabase::get(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	if (index == items.end() || !index->second.isSet)
		return std::string();
	else
		return index->second.value;
}

float					StateDatabase::eval(std::string name) const
{
	static std::vector<std::string> variables;

	for (std::vector<std::string>::iterator i = variables.begin(); i != variables.end(); i++)
		if (*i == name)
			throw std::runtime_error("circular reference in eval(" + name + ")\n");
	
	variables.push_back(name);

	Map::const_iterator index = items.find(name);
	if (index == items.end() || !index->second.isSet)
		throw std::runtime_error("BZDB:" + name + " not found\n");
//		return NaN;
	Expression pre, inf;
	std::string value = index->second.value;
	value >> inf;
	pre = infixToPrefix(inf);
	float retn = evaluate(pre);
	variables.pop_back();
	return retn;
}

bool					StateDatabase::isTrue(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return !(index == items.end() || !index->second.isTrue);
}

bool					StateDatabase::isEmpty(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return (index == items.end() || !index->second.isSet ||
								index->second.value.empty());
}

bool					StateDatabase::isPersistent(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return (index != items.end() && index->second.save);
}

std::string				StateDatabase::getDefault(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	if (index != items.end())
		return index->second.defValue;
	else
		return "";
}

StateDatabase::Permission
						StateDatabase::getPermission(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	if (index != items.end())
		return index->second.permission;
	else
		return ReadWrite;
}

StateDatabase::Map::iterator
						StateDatabase::lookup(const std::string& name)
{
	Map::iterator index = items.find(name);
	if (index == items.end()) {
		Item tmp;
		return items.insert(std::make_pair(name, tmp)).first;
	}
	else {
		return index;
	}
}

void					StateDatabase::notify(Map::iterator index)
{
	index->second.callbacks.iterate(&onCallback,
								const_cast<void*>(
								reinterpret_cast<const void*>(&index->first)));
}

bool					StateDatabase::onCallback(
								Callback callback,
								void* userData,
								void* iterateData)
{
	callback(*reinterpret_cast<std::string*>(iterateData), userData);
	return true;
}

void					StateDatabase::iterate(
								Callback callback, void* userData) const
{
	assert(callback != NULL);

	for (Map::const_iterator index = items.begin();
								index != items.end(); ++index) {
		if (index->second.isSet) {
			(*callback)(index->first, userData);
		}
	}
}

void					StateDatabase::write(
								Callback callback, void* userData) const
{
	assert(callback != NULL);

	for (Map::const_iterator index = items.begin();
								index != items.end(); ++index) {
		if (index->second.isSet && index->second.save &&
		index->second.value != index->second.defValue) {
			(*callback)(index->first, userData);
		}
	}
}

StateDatabase*			StateDatabase::getInstance()
{
	if (s_instance == NULL)
		s_instance = new StateDatabase;
	return s_instance;
}

StateDatabase::ExpressionToken::ExpressionToken()
{
	tokenType = number;
	tokenContents.number = 0;
}

StateDatabase::ExpressionToken::ExpressionToken(Type _tokenType)
{
	tokenType = _tokenType;
	switch(tokenType) {
	case number:
		tokenContents.number = 0;
		break;
	case variable:
		break;
	case oper:
		tokenContents.oper = none;
		break;
	}
}

StateDatabase::ExpressionToken::ExpressionToken(Type _tokenType, Contents _tokenContents)
{
	tokenType = _tokenType;
	tokenContents = _tokenContents;
}

void StateDatabase::ExpressionToken::setType(Type _tokenType)
{
	tokenType = _tokenType;
	switch(tokenType) {
	case number:
		tokenContents.number = 0;
		break;
	case variable:
		break;
	case oper:
		tokenContents.oper = none;
	}
}

void StateDatabase::ExpressionToken::setContents(Contents _tokenContents)
{
	tokenContents = _tokenContents;
}

void StateDatabase::ExpressionToken::setNumber(double num)
{
	tokenType = number;
	tokenContents.number = num;
}

void StateDatabase::ExpressionToken::setVariable(std::string var)
{
	tokenType = variable;
	tokenContents.variable = var;
}

void StateDatabase::ExpressionToken::setOper(Operator op)
{
	tokenType = oper;
	tokenContents.oper = op;
}

StateDatabase::ExpressionToken::Type StateDatabase::ExpressionToken::getTokenType()
{
	return tokenType;
}

StateDatabase::ExpressionToken::Contents StateDatabase::ExpressionToken::getTokenContents()
{
	return tokenContents;
}

double StateDatabase::ExpressionToken::getNumber()
{
	// note that the necessary type check must be done first
	return tokenContents.number;
}

std::string	StateDatabase::ExpressionToken::getVariable()
{
	// note that the necessary type check must be done first
	return tokenContents.variable;
}

StateDatabase::ExpressionToken::Operator StateDatabase::ExpressionToken::getOperator()
{
	// note that the necessary type check must be done first
	return tokenContents.oper;
}

int StateDatabase::ExpressionToken::getPrecedence()
{
	switch (tokenContents.oper) {
		case add:
		case subtract:
			return 1;
		case multiply:
		case divide:
			return 2;
		case power:
			return 3;
		case lparen:
			return 4;
		case rparen:
		default:
			return 0;
	}
}

std::istream&operator >> (std::istream& src, StateDatabase::ExpressionToken& dst)
{
	char temp;
	std::string tempname;

	src >> temp;
	if ((temp >= '0' && temp <= '9') || temp == '.') {
		// number
		tempname += temp;
		temp = src.peek();
		while ((temp >= '0' && temp <= '9') || temp == '.') {
			src >> temp;
			tempname += temp;
			temp = src.peek();
		}
		dst.setNumber(atof(tempname.c_str()));
	}
	else if (temp == '+' || temp == '-' || temp == '*' || temp == '/' || temp == '^' || temp == '(' || temp == ')') {
		// operator
		switch(temp) {
		case '+':
			dst.setOper(StateDatabase::ExpressionToken::add);
			break;
		case '-':
			dst.setOper(StateDatabase::ExpressionToken::subtract);
			break;
		case '*':
			dst.setOper(StateDatabase::ExpressionToken::multiply);
			break;
		case '/':
			dst.setOper(StateDatabase::ExpressionToken::divide);
			break;
		case '^':
			dst.setOper(StateDatabase::ExpressionToken::power);
			break;
		case '(':
			dst.setOper(StateDatabase::ExpressionToken::lparen);
			break;
		case ')':
			dst.setOper(StateDatabase::ExpressionToken::rparen);
			break;
		}
	}
	else if((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') {
		// variable (perhaps prefix with $?)
		tempname += temp;
		temp = src.peek();
		while ((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') {
			src >> temp;
			tempname += temp;
			temp = src.peek();
		}
		dst.setVariable(tempname);
	}
	else {
		// throw an error?
	}
	return src;
}

std::string& operator >> (std::string& src, StateDatabase::ExpressionToken& dst)
{
	char temp;
	std::string tempname;

	temp = src[0]; src = src.substr(1);
	if ((temp >= '0' && temp <= '9') || temp == '.') {
		// number
		tempname += temp;
		temp = src[0];
		while (((temp >= '0' && temp <= '9') || temp == '.') && (src.length() != 0)) {
			src = src.substr(1);
			tempname += temp;
			temp = src[0];
		}
		dst.setNumber(atof(tempname.c_str()));
	}
	else if (temp == '+' || temp == '-' || temp == '*' || temp == '/' || temp == '^' || temp == '(' || temp == ')') {
		// operator
		switch (temp) {
		case '+':
			dst.setOper(StateDatabase::ExpressionToken::add);
			break;
		case '-':
			dst.setOper(StateDatabase::ExpressionToken::subtract);
			break;
		case '*':
			dst.setOper(StateDatabase::ExpressionToken::multiply);
			break;
		case '/':
			dst.setOper(StateDatabase::ExpressionToken::divide);
			break;
		case '^':
			dst.setOper(StateDatabase::ExpressionToken::power);
			break;
		case '(':
			dst.setOper(StateDatabase::ExpressionToken::lparen);
			break;
		case ')':
			dst.setOper(StateDatabase::ExpressionToken::rparen);
			break;
		}
	}
	else if ((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') {
		tempname += temp;
		temp = src[0];
		while (((temp >= 'a' && temp <= 'z') || (temp >= 'A' && temp <= 'Z') || temp == '_') && (src.length() != 0)) {
			src = src.substr(1);
			tempname += temp;
			temp = src[0];
		}
		dst.setVariable(tempname);
	}
	else {
		// throw an error?
	}
	return src;
}

std::ostream& operator << (std::ostream& dst, StateDatabase::ExpressionToken& src)
{
	switch (src.tokenType) {
	case StateDatabase::ExpressionToken::number:
		dst << src.getNumber();
		break;
	case StateDatabase::ExpressionToken::oper:
		switch (src.getOperator()) {
		case StateDatabase::ExpressionToken::add:
			dst << '+';
			break;
		case StateDatabase::ExpressionToken::subtract:
			dst << '-';
			break;
		case StateDatabase::ExpressionToken::multiply:
			dst << '*';
			break;
		case StateDatabase::ExpressionToken::divide:
			dst << '/';
			break;
		case StateDatabase::ExpressionToken::power:
			dst << '^';
			break;
		case StateDatabase::ExpressionToken::lparen:
			dst << '(';
			break;
		case StateDatabase::ExpressionToken::rparen:
			dst << ')';
			break;
		case StateDatabase::ExpressionToken::none:
			break;
		}
		break;
	case StateDatabase::ExpressionToken::variable:
		dst << src.getVariable();
		break;
	}
	return dst;
}

std::istream& operator >> (std::istream& src, StateDatabase::Expression& dst)
{
	StateDatabase::ExpressionToken temp;
	char tempc;

	dst.clear();
	src.unsetf(ios::skipws);
	while (src.peek() != '\n') {
		while (src.peek() == ' ' || src.peek() == '\t')
			src >> tempc;
		src >> temp;
		dst.push_back(temp);
	}
	src >> tempc;
	src.setf(ios::skipws);
	return src;
}

std::string& operator >> (std::string& src, StateDatabase::Expression& dst)
{
	StateDatabase::ExpressionToken temp;

	dst.clear();
	while (src.length() != 0) {
		while (src[0] == ' ' || src[0] == '\t') {
			src = src.substr(1);
		}
		src >> temp;
		dst.push_back(temp);
	}
	return src;
}

std::ostream& operator << (std::ostream& dst, StateDatabase::Expression& src)
{
	if(src.size()) {
		for (unsigned int i = 0; i < src.size() - 1; i++) {
			dst << src[i] << ' ';
		}
		dst << src[src.size() - 1];
	}
	return dst;
}

StateDatabase::Expression StateDatabase::infixToPrefix(Expression infix) const
{
	Expression postfix, prefix;
	std::stack<ExpressionToken> operators;

	for (Expression::iterator i = infix.begin(); i != infix.end(); i++) {
		if (i->getTokenType() == ExpressionToken::variable || i->getTokenType() == ExpressionToken::number) {
			postfix.push_back(*i);
		}
		else if (i->getTokenType() == ExpressionToken::oper) {
			if (i->getOperator() == ExpressionToken::lparen) {
				operators.push(*i);
			}
			else if (i->getOperator() == ExpressionToken::rparen) {
				// unstack operators until a matching ( is found
				while(operators.top().getOperator() != ExpressionToken::lparen) {
					postfix.push_back(operators.top()); operators.pop();
				}
				// discard (
				operators.pop();
			}
			else {
				while((operators.size() > 0) && (operators.top().getPrecedence() < i->getPrecedence()) && (operators.top().getOperator() != ExpressionToken::lparen)) {
					postfix.push_back(operators.top()); operators.pop();
				}
				operators.push(*i);
			}
		}
	}
	while (operators.size() > 0) {
		postfix.push_back(operators.top()); operators.pop();
	}

	for (Expression::reverse_iterator ri = postfix.rbegin(); ri != postfix.rend(); ri++)
		prefix.push_back(*ri);
	return prefix;
}

float StateDatabase::evaluate(Expression e) const
{
	stack<ExpressionToken> evaluationStack;
	ExpressionToken tok, lvalue, rvalue;
	bool unary;

	for (Expression::reverse_iterator i = e.rbegin(); i != e.rend(); i++) {
		unary = false;
		switch(i->getTokenType()) {
		case ExpressionToken::number:
			evaluationStack.push(*i);
			break;
		case ExpressionToken::variable:
			// strip off '$'?
			tok.setNumber(BZDB->eval(i->getVariable()));
			evaluationStack.push(tok);
			break;
		case ExpressionToken::oper:
			if (evaluationStack.size() == 0) {
				// syntax error
			}
			lvalue = evaluationStack.top(); evaluationStack.pop();
			if (evaluationStack.size() == 0) {
				unary = true; // syntax error or unary operator
			}
			if (!unary) {
				rvalue = evaluationStack.top(); evaluationStack.pop();
			}
			switch(i->getOperator()) {
			case ExpressionToken::add:
				tok.setNumber(lvalue.getNumber() + rvalue.getNumber());
				evaluationStack.push(tok);
				break;
			case ExpressionToken::subtract:
				if (unary)
					tok.setNumber(-lvalue.getNumber());
				else
					tok.setNumber(lvalue.getNumber() - rvalue.getNumber());
				evaluationStack.push(tok);
				break;
			case ExpressionToken::multiply:
				tok.setNumber(lvalue.getNumber() * rvalue.getNumber());
				evaluationStack.push(tok);
				break;
			case ExpressionToken::divide:
				tok.setNumber(lvalue.getNumber() / rvalue.getNumber());
				evaluationStack.push(tok);
				break;
			case ExpressionToken::power:
				tok.setNumber(pow(lvalue.getNumber(), rvalue.getNumber()));
				evaluationStack.push(tok);
				break;
			default:
				// lparen and rparen should have been stripped out
				// throw something here, too
				break;
			}
			break;
		}
	}
	return evaluationStack.top().getNumber();
}

// ex: shiftwidth=4 tabstop=4
