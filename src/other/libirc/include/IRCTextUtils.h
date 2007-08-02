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

// made independent of bzflag commnon.h Jeff Myers 9-27-04
/*
 * common definitions
 */

#ifndef __IRC_TEXTUTILS_H__
#define	__IRC_TEXTUTILS_H__

/* system interface headers */
#include <algorithm>
#include <ctype.h>
#include <string>
#include <stdarg.h>
#include <vector>

#ifdef _WIN32	// win don'st have all the same str uitls
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define vsnprintf _vsnprintf
#endif



// because lists of strings are used ALL over the place
typedef std::vector<std::string>	string_list;

// so we can keep a map with strings as the key
inline  bool operator < (const std::string &s1,const std::string &s2) { return (s1.compare(s2)<0);}

namespace string_util {
	std::string vformat(const char* fmt, va_list args);
	std::string format(const char* fmt, ...);

	/** returns a string converted to lowercase
	*/
	inline std::string tolower(const std::string& s)
	{
		std::string trans = s;

		for (std::string::iterator i=trans.begin(), end=trans.end(); i!=end; ++i)
			*i = ::tolower(*i);
		return trans;
	}

	/** returns a string converted to uppercase
	*/
	inline std::string toupper(const std::string& s)
	{
		std::string trans = s;
		for (std::string::iterator i=trans.begin(), end=trans.end(); i!=end; ++i)
			*i = ::toupper(*i);
		return trans;
	}

	/** replace all of in in replaceMe with withMe
	*/
	std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe);

	/**
	* Get a vector of strings from a string, using all of chars of thedelims
	* string as separators. If maxTokens > 0, then the last 'token' maycontain delimiters
	* as it just returns the rest of the line
	* if you specify use quotes then tokens can be grouped in quotes and delimeters
	* inside quotes are ignored.
	* Hence /ban "Mr Blahblah" isajerk parses to "/ban", "Mr Blahlah" and "isajerk"
	* but so does "Mr Blahblah" "isajerk", so if you want 3 tokens and a delimeter
	* is in one of the tokens, by puting quotes around it you can get the correct
	* parsing. When use quotes is enabled, \'s and "'s should\can be escaped with \
	* escaping is not currently done for any other character.
	* Should not have " as a delimeter if you want to use quotes
	*/
	std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens = 0, const bool useQuotes = false);

	/** convert a string represantation of some duration into minutes
	*  example: "1d2h16m" -> 1500
	*/
	int parseDuration(const std::string &duration);

	// C h a r a c t e r  c o m p a r i s o n

	/** compare_nocase is strait from Stroustrup.  This implementation uses
	* strings instead of char arrays and includes a maxlength bounds check.
	* It compares two strings and returns 0 if equal, <0 if s1 is less than
	* s2, and >0 if s1 is greater than s2.
	*/
	inline int compare_nocase(const std::string& s1, const std::string &s2, int maxlength=4096)
	{
		std::string::const_iterator p1 = s1.begin();
		std::string::const_iterator p2 = s2.begin();
		int i=0;
		while (p1 != s1.end() && p2 != s2.end()) {
			if (i >= maxlength) {
				return 0;
			}
			if (::tolower(*p1) != ::tolower(*p2)) {
				return (::tolower(*p1) < ::tolower(*p2)) ? -1 : 1;
			}
			++p1;
			++p2;
			++i;
		}
		return (s2.size() == s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 : 1; // size is unsigned
	}



	/** utility function returns truthfully whether
	* given character is a letter.
	*/
	inline bool isAlphabetic(const char c)
	{
		if (( c > 64 && c < 91) ||
			( c > 96 && c < 123)) {
				return true;
			}
			return false;
	}


	/** utility function returns truthfully whether
	* given character is a number.
	*/
	inline bool isNumeric(const char c)
	{
		if (( c > 47 && c < 58)) {
			return true;
		}
		return false;
	}


	/** utility function returns truthfully whether
	* a given character is printable whitespace.
	* this includes newline, carriage returns, tabs
	* and spaces.
	*/
	inline bool isWhitespace(const char c)
	{
		if ((( c >= 9 ) && ( c <= 13 )) ||
			(c == 32)) {
				return true;
			}
			return false;
	}


	/** utility function returns truthfully whether
	* a given character is punctuation.
	*/
	inline bool isPunctuation(const char c)
	{
		if (( c > 32 && c < 48) ||
			( c > 57 && c < 65) ||
			( c > 90 && c < 97) ||
			( c > 122 && c < 127)) {
				return true;
			}
			return false;
	}


	/** utility function returns truthfully whether
	* given character is an alphanumeric.  this is
	* strictly letters and numbers.
	*/
	inline bool isAlphanumeric(const char c)
	{
		if ( isAlphabetic(c) || isNumeric(c) ) {
			return true;
		}
		return false;
	}


	/** utility function returns truthfully whether
	* given character is printable.  this includes
	* letters, numbers, and punctuation.
	* (but NOT whitespace)
	*/
	inline bool isVisible(const char c)
	{
		if ( isAlphanumeric(c) || isPunctuation(c) ) {
			return true;
		}
		return false;
	}


	/** utility function returns truthfully whether
	* given character is printable.  this includes
	* letters, numbers, punctuation, and whitespace
	*/
	inline bool isPrintable(const char c)
	{
		if ( isVisible(c) || isWhitespace(c) ) {
			return false;
		}
		return true;
	}



	// S t r i n g  i t e r a t i o n

	/** utility method that returns the position of the
	* first alphanumeric character from a string
	*/
	inline int firstAlphanumeric(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;

		}

		int i = 0;
		while (!isAlphanumeric(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}


	/** utility method that returns the position of the
	* first non-alphanumeric character from a string
	*/
	inline int firstNonalphanumeric(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;
		}

		int i = 0;
		while (isAlphanumeric(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}


	/** utility method that returns the position of the
	* first printable character from a string
	*/
	inline int firstPrintable(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;
		}

		int i = 0;
		while (!isPrintable(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}


	/** utility method that returns the position of the
	* first non-printable character from a string
	*/
	inline int firstNonprintable(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;
		}

		int i = 0;
		while (isPrintable(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}


	/** utility method that returns the position of the
	* first visible character from a string
	*/
	inline int firstVisible(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;
		}

		int i = 0;
		while (isVisible(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}


	/** utility method that returns the position of the
	* first non visible character from a string (control
	* codes or whitespace
	*/
	inline int firstNonvisible(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;
		}

		int i = 0;
		while (!isVisible(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}


	/** utility method that returns the position of the
	* first alphabetic character from a string
	*/
	inline int firstAlphabetic(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;

		}

		int i = 0;
		while (!isAlphabetic(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}

	/** utility method that returns the position of the
	* first printable character from a string
	*/
	inline int firstNonalphabetic(const std::string &input, unsigned short int max=4096)
	{
		if (input.size() == 0) {
			return -1;
		}

		int i = 0;
		while (isAlphabetic(input[i]) && (i < max)) {
			i++;
		}
		return i;
	}

	/** url-encodes a string
	*/
	std::string url_encode(const std::string &text);

	/** escape a string
	*/
	std::string escape(const std::string &text, char escaper);

	/** un-escape a string
	*/
	std::string unescape(const std::string &text, char escaper);

	/** lookup for an un-escaped separator
	*/
	int unescape_lookup(const std::string &text, char escaper, char sep);

	// dump out a string from a list range deliminated, helpfull for geting stuff back out of tokenized lists
	inline std::string getStringFromList ( string_list &params, std::string delim = " ", int start = 0, int end = -1 )
	{
		int realEnd = (int)params.size();
		if ( end != -1 )
			realEnd = end;

		std::vector<std::string>::iterator itr = params.begin() + start;

		std::string  temp;

		int i = start;
		while ( itr != params.end())
		{
			temp += *itr;
			itr++;
			i++;

			if ( i >= realEnd )
				itr == params.end();

			if (itr != params.end())
				temp += delim;
		}
		return temp;
	}

	inline string_list slice ( std::string text, int maxSize, bool splitNewlines = false )
	{
		string_list	list;

		int i = 0;

		std::string::iterator itr = text.begin();
		std::string temp;

		while ( itr != text.end() )
		{
			bool split = false;

			if (splitNewlines && *itr == '\n')
				split = true;
			else
				temp += *itr;	

			i++;
			itr++;

			if ( i > maxSize )
				split = true;

			if (split)
			{
				if (temp.size())
					list.push_back(temp);
				i = 0;
				temp = "";
			}
		}

		if (temp.size())
			list.push_back(temp);

		return list;
	}

	inline bool charExists ( std::string text, char f )
	{
		return strchr(text.c_str(),f) != NULL;
	}

	inline std::string::iterator findChar ( std::string text, char f )
	{
		std::string x;
		x+=f;

		return find_first_of(text.begin(),text.end(),x.begin(),x.end());
	}

	inline void eraseFirstOf ( std::string &text, char f )
	{
		std::string::iterator itr = text.begin();

		while(itr != text.end())
		{
			if (*itr == f)
			{
				text.erase(itr);
				itr == text.end();
			}
			else
				itr++;
		}
	}
};


#endif // __IRC_TEXTUTILS_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
