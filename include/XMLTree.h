#ifndef XMLTREE_H
#define XMLTREE_H

#include "common.h"
#include "tree.h"
#include "bzfio.h"
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <string>

class XMLStreamPosition {
public:
	XMLStreamPosition();
	XMLStreamPosition(const std::string& filename,
							unsigned int line = 1, unsigned int column = 1);

	void				swap(XMLStreamPosition&);

public:
	std::string			filename;
	unsigned int		line;
	unsigned int		column;
};

class XMLIOException : public std::runtime_error {
public:
	XMLIOException(const XMLStreamPosition&, const char*);
	XMLIOException(const XMLStreamPosition&, const std::string&);

	virtual ~XMLIOException() throw();

public:
	XMLStreamPosition	position;
};

class XMLNode {
public:
	// swap data with given node
	void				swap(XMLNode&);

	// simple attribute query.  if the attribute with name exists then
	// saves the value in the second argument and returns true.  otherwise
	// returns false and the second argument isn't modified.
	bool				getAttribute(const std::string& name, std::string&) const;

	// get an attribute's position in the stream
	XMLStreamPosition	getAttributePosition(const std::string& name) const;

	// used in function object getAttribute()
	class AttributeException : public std::runtime_error {
	public:
		AttributeException(const char* msg) : std::runtime_error(msg) { }
		AttributeException(const std::string& msg):
							std::runtime_error(msg.c_str()) { }
	};

	// if the attribute with name exists then pass the value to the
	// operation op.  if op throws AttributeException then the
	// exception is converted to an XMLIOException.
	template <class Operation>
	bool				getAttribute(const std::string& name, Operation op) const
	{
		Attributes::const_iterator index = attributes.find(name);
		if (index != attributes.end()) {
			try {
				op(index->second.second);
				return true;
			}
			catch (AttributeException& e) {
				throw XMLIOException(index->second.first,
							string_util::format(
								"%s in attribute `%s': %s",
								e.what(),
								name.c_str(),
								index->second.second.c_str()).c_str());
			}
		}
		return false;
	}

public:
	typedef std::pair<XMLStreamPosition, std::string> AttributeInfo;
	typedef std::map<std::string, AttributeInfo> Attributes;

	// types of nodes
	enum Type {
		Tag,
		Data,
		PI,		// processing instruction
	};

	Type				type;
	std::string			value;
	Attributes			attributes;
	XMLStreamPosition	position;
};

class XMLTree : public tree<XMLNode> {
public:
	XMLTree();
	XMLTree(const XMLTree&);
	~XMLTree();

	// I/O.  if an iterator is passed to read() then elements in the
	// stream are added as children of that iterator.  if the tree is
	// empty then a dummy root node is created.  operator>>() removes
	// all elements from the tree before reading.
	std::istream&			read(std::istream&, const XMLStreamPosition&);
	std::istream&			read(std::istream&, const XMLStreamPosition&,
							XMLTree::iterator);
	friend std::istream&		operator>>(std::istream&, XMLTree&);
//	friend ostream&		operator<<(ostream&, const XMLTree&);

	// escape a string so it will be correctly read by read() and
	// operator>>() (assuming it's data or an attribute value).
	static std::string	escape(const std::string& string);

private:
	enum NodeType {
		End,
		Open,
		Close,
		Empty,
		Data,
		Include
	};

	class XMLStream {
	public:
		XMLStream(std::istream*);
		XMLStream(std::istream*, const XMLStreamPosition&);

		// stream-like operations
		char			get();
		char			peek();
		void			skip();
		operator void*() const;
		bool			operator!() const;

		// other stuff
		void			setFail();
		void			setEOFIsOkay(bool on) { eofIsOkay = on; }

		// push an included stream (stream is adopted)
		void			push(std::istream*);
		void			push(std::istream*, const XMLStreamPosition&);

	private:
		void			pop();

	public:
		XMLStreamPosition	position;

	private:
		typedef std::vector<std::pair<std::istream*, XMLStreamPosition> > StreamStack;

		std::istream*		stream;
		StreamStack		streamStack;
		bool			eofIsOkay;
	};

	class XMLTreeIOException : public XMLIOException {
	public:
		XMLTreeIOException(const XMLStream&, const char*);
		XMLTreeIOException(const XMLStream&, const std::string&);
	};

	void				read(XMLStream&, XMLTree::iterator);

	typedef bool (*CharClass)(char);
	static NodeType		readNode(XMLStream&, XMLNode*, bool eofIsOkay = false);
	static void			readComment(XMLStream&);
	static NodeType		readCDATA(XMLStream&, XMLNode*);
	static void			readINCLUDE(XMLStream&);
	static std::string	readToken(XMLStream&,
							CharClass firstCharClass,
							CharClass otherCharClass,
							const char* delims,
							const char* tokenType);
	static std::string	readValue(XMLStream&);
	static void			readParameters(XMLStream&,
							XMLNode*, const char* delims);
	static char			readReference(XMLStream&);
	static char			readCharReference(XMLStream&);
	static void			eat(XMLStream&, const char* skip, const char* err);
	static void			skipWhitespace(XMLStream&);
	static bool			isLetter(char);
	static bool			isDigit(char);
	static bool			isHexDigit(char);
	static bool			isSpace(char);
	static bool			isNameChar(char);
	static bool			isNameFirstChar(char);
};

//
// function objects useful for parsing
//

template <class T>
class XMLSetVar_t : public unary_function<T, T> {
public:
	XMLSetVar_t(T* dst_) : dst(dst_) { }
	T operator()(const T& arg) const
	{
		*dst = arg;
		return *dst;
	}

private:
	T*					dst;
};

template <class T>
inline
XMLSetVar_t<T>
xmlSetVar(T& arg)
{
	return XMLSetVar_t<T>(&arg);
}

/* crs -- xmlSetMethod returns a function object to call a member function
 * and XMLSetMethod_t<>::operator() expects to be able to return void.
 * since VC++ 6.0 can't do that and setter methods don't normally return
 * values, we return some arbitrary type (int) instead so that other
 * templates that return the result of this function object don't choke
 * on returning void.  ick! */
#if defined(_MSC_VER)
#define XML_SET_METHOD_RESULT		int
#define XML_SET_METHOD_RETURN(x_)	x_; return 0
#else
#define XML_SET_METHOD_RESULT		Result
#define XML_SET_METHOD_RETURN(x_)	return x_
#endif

template <class Object, class Arg, class Result>
class XMLSetMethod_t : public unary_function<Arg, XML_SET_METHOD_RESULT> {
public:
	typedef Result (Object::*Member)(Arg);
	XMLSetMethod_t(Object* object_, Member member_) :
							object(object_), member(member_) { }
	XML_SET_METHOD_RESULT
	operator()(Arg arg) const
	{
		XML_SET_METHOD_RETURN((object->*member)(arg));
	}

private:
	Object*				object;
	Member				member;
};

template <class Object, class Arg, class Result>
inline
XMLSetMethod_t<Object, Arg, Result>
xmlSetMethod(Object* object, Result (Object::*member)(Arg))
{
	return XMLSetMethod_t<Object, Arg, Result>(object, member);
}

template <class Operation, class Function>
class XMLStr2Num_t : public unary_function<std::string,
										typename Operation::result_type> {
public:
	XMLStr2Num_t(const Operation& op_, Function func_) :
							op(op_), func(func_) { }
	typename Operation::result_type
						operator()(std::string arg) const
	{
		return op(func(arg.c_str()));
	}

private:
	Operation			op;
	Function			func;
};

inline
int xmlAttrStr2long(const char* s)
{
	char* end;
	int v = (int)strtol(s, &end, 10);
	if (*end != '\0')
		throw XMLNode::AttributeException("invalid value");
	return v;
}

inline
double xmlAttrStr2double(const char* s)
{
	char* end;
	double v = strtod(s, &end);
	if (*end != '\0')
		throw XMLNode::AttributeException("invalid value");
	return v;
}

template <class Operation>
inline
XMLStr2Num_t<Operation, int (*)(const char*)>
xmlStrToInt(const Operation& op)
{
	return XMLStr2Num_t<Operation, int (*)(const char*)>(op, xmlAttrStr2long);
}

template <class Operation>
inline
XMLStr2Num_t<Operation, double (*)(const char*)>
xmlStrToFloat(const Operation& op)
{
	return XMLStr2Num_t<Operation, double (*)(const char*)>(op, xmlAttrStr2double);
}

template <class T>
class XMLMin_t : public unary_function<T, T> {
public:
	XMLMin_t(T x_) : x(x_) { }
	T					operator()(T a) const
	{
		return (a < x) ? a : x;
	}

private:
	T					x;
};

template <class T>
inline
XMLMin_t<T> xmlMin(T x)
{
	return XMLMin_t<T>(x);
}

template <class T>
class XMLMax_t : public unary_function<T, T> {
public:
	XMLMax_t(T x_) : x(x_) { }
	T					operator()(T a) const
	{
		return (a < x) ? x : a;
	}

private:
	T					x;
};

template <class T>
inline
XMLMax_t<T> xmlMax(T x)
{
	return XMLMax_t<T>(x);
}

template <class Outer, class Inner>
class XMLCompose_t : public unary_function<typename Inner::argument_type,
											typename Outer::result_type> {
public:
	XMLCompose_t(Outer outer_, Inner inner_) : outer(outer_), inner(inner_) { }
	typename Outer::result_type
	operator()(typename Inner::argument_type arg) const
	{
		return outer(inner(arg));
	}

private:
	Outer				outer;
	Inner				inner;
};

template <class Outer, class Inner>
inline
XMLCompose_t<Outer, Inner> xmlCompose(Outer outer, Inner inner)
{
	return XMLCompose_t<Outer, Inner>(outer, inner);
}

template <class T>
class XMLParseEnumList {
public:
	const char*			name;
	T					value;
};

template <class E, class Operation>
class XMLParseEnum_t : public unary_function<std::string,
										typename Operation::result_type> {
public:
	XMLParseEnum_t(const E* enumerants_, const Operation& op) :
							enumerants(enumerants_), operation(op) { }
	typename Operation::result_type
	operator()(const std::string& enumerant) const
	{
		for (const E* scan = enumerants; scan->name != NULL; ++scan)
			if (enumerant == scan->name)
				return operation(scan->value);
		throw XMLNode::AttributeException("invalid enumerant");
	}

private:
	const E*			enumerants;
	Operation			operation;
};

template <class E, class Operation>
inline
XMLParseEnum_t<E, Operation>
xmlParseEnum(const E* e, const Operation& op)
{
	return XMLParseEnum_t<E, Operation>(e, op);
}


//
// common enumerations
//

extern const XMLParseEnumList<bool> s_xmlEnumBool[];	// "false", "true"

#endif // XMLTREE_H
// ex: shiftwidth=4 tabstop=4
